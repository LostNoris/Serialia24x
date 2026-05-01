#include "plugin.hpp"


struct Serialia24x : Module {

	int step = 0;
	int step2 = 0;
	int step3 = 0;

	int rowLength = 24;
	int prevRowLength = 24;

	bool invertState = false;
	bool pendInvert = false;
	bool retrogradeState = false;

	bool invertState2 = false;
	bool pendInvert2 = false;
	bool retrogradeState2 = false;

	bool invertState3 = false;
	bool pendInvert3 = false;
	bool retrogradeState3 = false;

	int toneRow[24];
	int inversion[24];
	int lastParams[24];

	dsp::SchmittTrigger clockTrigger;
	dsp::SchmittTrigger clockTrigger2;
	dsp::SchmittTrigger clockTrigger3;

	dsp::SchmittTrigger InvertTrigger;
	dsp::SchmittTrigger RetrogradeTrigger;

	dsp::SchmittTrigger InvertButton;
	dsp::SchmittTrigger RetrogradeButton;

	dsp::SchmittTrigger InvertTrigger2;
	dsp::SchmittTrigger RetrogradeTrigger2;

	dsp::SchmittTrigger InvertButton2;
	dsp::SchmittTrigger RetrogradeButton2;

	dsp::SchmittTrigger InvertTrigger3;
	dsp::SchmittTrigger RetrogradeTrigger3;

	dsp::SchmittTrigger InvertButton3;
	dsp::SchmittTrigger RetrogradeButton3;

	void computeInversion() {
		int root = toneRow[0];

		for (int i = 0; i < rowLength; i++) {
			int interval = (toneRow[i] - root + rowLength) % rowLength;
			inversion[i] = (root - interval + rowLength) % rowLength;
		}
	}


	enum ParamId {
		VALUE_PARAM,
		VALUE_PARAM_LAST = VALUE_PARAM + 23,

		LENGTH_PARAM,

		INVERT_BUTTON,
		RETROGRADE_BUTTON,

		INVERT2_BUTTON,
		RETROGRADE2_BUTTON,

		INVERT3_BUTTON,
		RETROGRADE3_BUTTON,

		PARAMS_LEN
	};

	enum InputId {
		CLOCK_INPUT,
		CLOCK_INPUT_LAST = CLOCK_INPUT + 2,

		INVERT_INPUT,
		RETROGRADE_INPUT,

		INVERT2_INPUT,
		RETROGRADE2_INPUT,

		INVERT3_INPUT,
		RETROGRADE3_INPUT,

		VALUE_INPUT,
		VALUE_INPUT_LAST = VALUE_INPUT + 23,

		INPUTS_LEN
	};

	enum OutputId {
		SEQ_OUTPUT,
		SEQ_OUTPUT_LAST = SEQ_OUTPUT + 2,

		OUTPUTS_LEN
	};

	enum LightId {
		SEQ_LIGHT,
		SEQ_LIGHT_LAST = SEQ_LIGHT + 11,

		SEQ2_LIGHT,
		SEQ2_LIGHT_LAST = SEQ2_LIGHT + 11,

		SEQ3_LIGHT,
		SEQ3_LIGHT_LAST = SEQ3_LIGHT + 11,

		INVERT2_LIGHT,
		RETROGRADE2_LIGHT,

		INVERT3_LIGHT,
		RETROGRADE3_LIGHT,

		INVERT_LIGHT,
		RETROGRADE_LIGHT,

		LIGHTS_LEN
	};

	Serialia24x() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

		for (int i = 0; i < 24; i++) {
			configParam(VALUE_PARAM + i, 0.f, 23.f, 0.f, "value");
			paramQuantities[VALUE_PARAM + i]->snapEnabled = true;
		}

		configParam(LENGTH_PARAM, 1, 24, 12, "length");
		paramQuantities[LENGTH_PARAM]->snapEnabled = true;

		for (int i = 0; i < rowLength; i++) {
			toneRow[i] = i;
			params[VALUE_PARAM + i].setValue(i);
			lastParams[i] = i;
		}

		configParam(INVERT_BUTTON, 0.f, 1.f, 0.f, "invert");
		configParam(RETROGRADE_BUTTON, 0.f, 1.f, 0.f, "retrograde");

		configParam(INVERT2_BUTTON, 0.f, 1.f, 0.f, "invert 2");
		configParam(RETROGRADE2_BUTTON, 0.f, 1.f, 0.f, "retrograde 2");

		configParam(INVERT3_BUTTON, 0.f, 1.f, 0.f, "invert 3");
		configParam(RETROGRADE3_BUTTON, 0.f, 1.f, 0.f, "retrograd3 3");

		for (int i = 0; i < 3; i++) {
			configInput(CLOCK_INPUT + i, "clock");
			configOutput(SEQ_OUTPUT + i, "Sequence");
		}

		configInput(INVERT_INPUT, "invert");
		configInput(RETROGRADE_INPUT, "retrograde");

		configInput(INVERT2_INPUT, "invert 2");
		configInput(RETROGRADE2_INPUT, "retrograde 2");

		configInput(INVERT3_INPUT, "invert 3");
		configInput(RETROGRADE3_INPUT, "retrograde 3");

		for (int i = 0; i < rowLength; i++) {

			configInput(VALUE_INPUT + i, "value");
			params[VALUE_PARAM + i].setValue(toneRow[i]);

		}

		

	}

	void process(const ProcessArgs& args) override {

		rowLength = params[LENGTH_PARAM].getValue();

		if (clockTrigger.process(inputs[CLOCK_INPUT].getVoltage())) {

			if (pendInvert == true) {
			    invertState = !invertState;

			    pendInvert = false;
		    }

			if (retrogradeState) {
				step = (step - 1 + rowLength) % rowLength;
			}
			else {
				step = (step + 1) % rowLength;
			}
		}

		if (clockTrigger2.process(inputs[CLOCK_INPUT + 1].getVoltage())) {
			if (pendInvert2 == true) {
				invertState2 = !invertState2;

				pendInvert2 = false;
			}

			if (retrogradeState2) {
				step2 = (step2 - 1 + rowLength) % rowLength;
			}
			else {
				step2 = (step2 + 1) % rowLength;
			}
		}
		if (clockTrigger3.process(inputs[CLOCK_INPUT + 2].getVoltage())) {
			if (pendInvert3 == true) {
				invertState3 = !invertState3;

				pendInvert3 = false;
			}

			if (retrogradeState3) {
				step3 = (step3 - 1 + rowLength) % rowLength;
			}
			else {
				step3 = (step3 + 1) % rowLength;
			}
		}
		for (int i = 0; i < rowLength; i++) {

			float value = params[VALUE_PARAM + i].getValue();

			float cv = inputs[VALUE_INPUT + i].isConnected() ? inputs[VALUE_INPUT + i].getVoltage() : 0.f;

			float semitones = cv * rowLength;

			int note = (int)std::round(semitones);

			int pitch = (note % rowLength + rowLength) % rowLength;

			if (inputs[VALUE_INPUT + i].isConnected()) {
				value = pitch;
				params[VALUE_PARAM + i].setValue(pitch);
			}

			if (value != lastParams[i]) {
				
				int newValue = value;
				int oldValue = toneRow[i];

				int clash = -1;
				for (int j = 0; j < rowLength; j++) {
					if (toneRow[j] == newValue) {
						clash = j;
						break;
					}
				}

				toneRow[i] = newValue;

				if (clash != -1 && clash != i) {
					toneRow[clash] = oldValue;
					params[VALUE_PARAM + clash].setValue(oldValue);
				}

				lastParams[i] = value;
			}
		}

		computeInversion();

		if (RetrogradeButton.process(params[RETROGRADE_BUTTON].getValue()))
			retrogradeState = !retrogradeState;
		if (InvertButton.process(params[INVERT_BUTTON].getValue()))
			pendInvert = true;
		if (RetrogradeTrigger.process(inputs[RETROGRADE_INPUT].getVoltage()))
			retrogradeState = !retrogradeState;
		if (InvertTrigger.process(inputs[INVERT_INPUT].getVoltage()))
			pendInvert = true;

		float currentStep = invertState
			? inversion[step]
			: toneRow[step];

		if (RetrogradeButton2.process(params[RETROGRADE2_BUTTON].getValue()))
			retrogradeState2 = !retrogradeState2;
		if (InvertButton2.process(params[INVERT2_BUTTON].getValue()))
			pendInvert2 = true;
		if (RetrogradeTrigger2.process(inputs[RETROGRADE2_INPUT].getVoltage()))
			retrogradeState2 = !retrogradeState2;
		if (InvertTrigger2.process(inputs[INVERT2_INPUT].getVoltage()))
			pendInvert2 = true;

		float currentStep2 = invertState2
			? inversion[step2]
			: toneRow[step2];

		if (RetrogradeButton3.process(params[RETROGRADE3_BUTTON].getValue()))
			retrogradeState3 = !retrogradeState3;
		if (InvertButton3.process(params[INVERT3_BUTTON].getValue()))
			pendInvert3 = true;
		if (RetrogradeTrigger3.process(inputs[RETROGRADE3_INPUT].getVoltage()))
			retrogradeState3 = !retrogradeState3;
		if (InvertTrigger3.process(inputs[INVERT3_INPUT].getVoltage()))
			pendInvert3 = true;

		float currentStep3 = invertState3
			? inversion[step3]
			: toneRow[step3];



		float voltage = currentStep / rowLength;
		float voltage2 = currentStep2 / rowLength;
		float voltage3 = currentStep3 /rowLength;

		outputs[SEQ_OUTPUT].setVoltage(voltage);
		outputs[SEQ_OUTPUT + 1].setVoltage(voltage2);
		outputs[SEQ_OUTPUT + 2].setVoltage(voltage3);

		for (int i = 0; i < 12; i++) {
			lights[SEQ_LIGHT + i].setBrightness(i == (step % 12) ? 1.f : 0.f);
			lights[SEQ2_LIGHT + i].setBrightness(i == (step2 % 12) ? 1.f : 0.f);
			lights[SEQ3_LIGHT + i].setBrightness(i == (step3 % 12) ? 1.f : 0.f);
		}

		lights[INVERT_LIGHT].setBrightness(invertState);
		lights[RETROGRADE_LIGHT].setBrightness(retrogradeState);

		lights[INVERT2_LIGHT].setBrightness(invertState2);
		lights[RETROGRADE2_LIGHT].setBrightness(retrogradeState2);

		lights[INVERT3_LIGHT].setBrightness(invertState3);
		lights[RETROGRADE3_LIGHT].setBrightness(retrogradeState3);
	   
	}
};


struct Serialia24xWidget : ModuleWidget {
	Serialia24xWidget(Serialia24x* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Serialia.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		for (int i = 0; i < 12; i++) {
			float x = 15.5f + i * 12.f;

			addParam(createParamCentered<RoundBlackKnob>(
				mm2px(Vec(x, 25)),
				module,
				Serialia24x::VALUE_PARAM + i
			));
		}

		for (int i = 0; i < 12; i++) {
			float x = 15.5f + i * 12.f;

			addInput(createInputCentered<PJ301MPort>(
				mm2px(Vec(x, 35)),
				module,
				Serialia24x::VALUE_INPUT + i
			));
		}

		for (int i = 0; i < 12; i++) {
			float x = 15.5f + i * 12.f;

			addParam(createParamCentered<RoundBlackKnob>(
				mm2px(Vec(x, 45)),
				module,
				Serialia24x::VALUE_PARAM + ( 12 + i )
			));
		}

		for (int i = 0; i < 12; i++) {
			float x = 15.5f + i * 12.f;

			addInput(createInputCentered<PJ301MPort>(
				mm2px(Vec(x, 55)),
				module,
				Serialia24x::VALUE_INPUT + ( 12 + i )
			));
		}

		for (int i = 0; i < 3; i++) {
			float x = 15.5f + i * 10.f;
			float z = 120.f + i * 10.f;

			addInput(createInputCentered<PJ301MPort>(
				mm2px(Vec(x, 110)),
				module,
				Serialia24x::CLOCK_INPUT + i
			));

			addOutput(createOutputCentered<PJ301MPort>(
				mm2px(Vec(z, 110)),
				module,
				Serialia24x::SEQ_OUTPUT + i
			));
		}

		for (int i = 0; i < 12; i++) {
			float x = 51.f + i * 6.f;

			addChild(createLightCentered<MediumLight<GreenLight>>(
				mm2px(Vec(x, 115)), module,
				Serialia24x::SEQ_LIGHT + i
			));
			addChild(createLightCentered<MediumLight<BlueLight>>(
				mm2px(Vec(x, 120)), module,
				Serialia24x::SEQ2_LIGHT + i
			));
			addChild(createLightCentered<MediumLight<WhiteLight>>(
			    mm2px(Vec(x, 125)), module,
				Serialia24x::SEQ3_LIGHT + i
			));
		}

		// Invert and Retrograde controls 1

		addParam(createParamCentered<LEDButton>(
			mm2px(Vec(40.5f, 90)),
			module,
			Serialia24x::INVERT_BUTTON
		));

        addInput(createInputCentered<PJ301MPort>(
			mm2px(Vec(40.5f, 98)),
			module,
			Serialia24x::INVERT_INPUT
		));

		addChild(createLightCentered<MediumLight<BlueLight>>(
			mm2px(Vec(40.5f, 106)), 
			module,
			Serialia24x::INVERT_LIGHT
		));

		addParam(createParamCentered<LEDButton>(
			mm2px(Vec(50.5f, 90)),
			module,
			Serialia24x::RETROGRADE_BUTTON
		));

        addInput(createInputCentered<PJ301MPort>(
			mm2px(Vec(50.5f, 98)),
			module,
			Serialia24x::RETROGRADE_INPUT
		));

		addChild(createLightCentered<MediumLight<RedLight>>(
			mm2px(Vec(50.5f, 106)), module,
			Serialia24x::RETROGRADE_LIGHT
		));

        // Invert and Retrograde controls 2

		addParam(createParamCentered<LEDButton>(
			mm2px(Vec(76, 90)),
			module,
			Serialia24x::INVERT2_BUTTON
		));

		addInput(createInputCentered<PJ301MPort>(
			mm2px(Vec(76, 98)),
			module,
			Serialia24x::INVERT2_INPUT
		));

		addChild(createLightCentered<MediumLight<BlueLight>>(
			mm2px(Vec(76, 106)),
			module,
			Serialia24x::INVERT2_LIGHT
		));

		addParam(createParamCentered<LEDButton>(
			mm2px(Vec(86, 90)),
			module,
			Serialia24x::RETROGRADE2_BUTTON
		));

		addInput(createInputCentered<PJ301MPort>(
			mm2px(Vec(86, 98)),
			module,
			Serialia24x::RETROGRADE2_INPUT
		));

		addChild(createLightCentered<MediumLight<RedLight>>(
			mm2px(Vec(86, 106)), module,
			Serialia24x::RETROGRADE2_LIGHT
		));
		
		//Invert and Retrograde controls 3

		addParam(createParamCentered<LEDButton>(
			mm2px(Vec(121.5f, 90)),
			module,
			Serialia24x::INVERT3_BUTTON
		));

		addInput(createInputCentered<PJ301MPort>(
			mm2px(Vec(121.5f, 98)),
			module,
			Serialia24x::INVERT3_INPUT
		));

		addChild(createLightCentered<MediumLight<BlueLight>>(
			mm2px(Vec(121.5f, 106)),
			module,
			Serialia24x::INVERT3_LIGHT
		));

		addParam(createParamCentered<LEDButton>(
			mm2px(Vec(131.5f, 90)),
			module,
			Serialia24x::RETROGRADE3_BUTTON
		));

		addInput(createInputCentered<PJ301MPort>(
			mm2px(Vec(131.5f, 98)),
			module,
			Serialia24x::RETROGRADE3_INPUT
		));

		addChild(createLightCentered<MediumLight<RedLight>>(
			mm2px(Vec(131.5f, 106)), module,
			Serialia24x::RETROGRADE3_LIGHT
		));


		addParam(createParamCentered<RoundBlackKnob>(
			mm2px(Vec(15.5f, 65)),
			module,
			Serialia24x::LENGTH_PARAM
		));

	}
};


Model* modelSerialia24x = createModel<Serialia24x, Serialia24xWidget>("Serialia24x");