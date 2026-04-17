#include "plugin.hpp"


struct Serialia : Module {

	int step = 0;

	bool invertState = false;
	bool pendInvert = false;
	bool retrogradeState = false;

	int toneRow[12];
	int inversion[12];
	int lastParams[12];

	dsp::SchmittTrigger clockTrigger;

	dsp::SchmittTrigger InvertTrigger;
	dsp::SchmittTrigger RetrogradeTrigger;

	dsp::SchmittTrigger InvertButton;
	dsp::SchmittTrigger RetrogradeButton;

	void computeInversion() {
		int root = toneRow[0];

		for (int i = 0; i < 12; i++) {
			int interval = (toneRow[i] - root + 12) % 12;
			inversion[i] = (root - interval + 12) % 12;
		}
	}


	enum ParamId {
		VALUE_PARAM,
		VALUE_PARAM_LAST = VALUE_PARAM + 11,
		INVERT_BUTTON,
		RETROGRADE_BUTTON,

		PARAMS_LEN
	};

	enum InputId {
		CLOCK_INPUT,

		INVERT_INPUT,
		RETROGRADE_INPUT,

		VALUE_INPUT,
		VALUE_INPUT_LAST = VALUE_INPUT + 11,

		INPUTS_LEN
	};

	enum OutputId {
		SEQ_OUTPUT,

		OUTPUTS_LEN
	};

	enum LightId {
		SEQ_LIGHT,
		SEQ_LIGHT_LAST = SEQ_LIGHT + 11,
		INVERT_LIGHT,
		RETROGRADE_LIGHT,
		LIGHTS_LEN
	};

	Serialia() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

		for (int i = 0; i < 12; i++) {
			configParam(VALUE_PARAM + i, 0.f, 11.f, 0.f, "value");
			paramQuantities[VALUE_PARAM + i]->snapEnabled = true;
		}

		for (int i = 0; i < 12; i++)
			toneRow[i] = i;

		configParam(INVERT_BUTTON, 0.f, 1.f, 0.f, "invert");
		configParam(RETROGRADE_BUTTON, 0.f, 1.f, 0.f, "retrograde");

		configInput(CLOCK_INPUT, "clock");

		configInput(INVERT_INPUT, "invert");
		configInput(RETROGRADE_INPUT, "retrograde");

		for (int i = 0; i < 12; i++) {

			configInput(VALUE_INPUT + i, "value");

		}

		configOutput(SEQ_OUTPUT, "Sequence");

	}

	void process(const ProcessArgs& args) override {

		if (clockTrigger.process(inputs[CLOCK_INPUT].getVoltage())) {

			if (pendInvert == true) {
			    invertState = !invertState;

			    pendInvert = false;
		    }

			if (retrogradeState) {
				step = (step - 1 + 12) % 12;
			}
			else {
				step = (step + 1) % 12;
			}
		}
		for (int i = 0; i < 12; i++) {

			float value = params[VALUE_PARAM + i].getValue();

			float cv = inputs[VALUE_INPUT + i].isConnected() ? inputs[VALUE_INPUT + i].getVoltage() : 0.f;

			float semitones = cv * 12.f;

			int note = (int)std::round(semitones);

			int pitch = (note % 12 + 12) % 12;

			if (cv > 0.f) {
				value = pitch;
				params[VALUE_PARAM + i].setValue(pitch);
			}

			if (value != lastParams[i]) {
				int newValue = value;
				int oldValue = toneRow[i];

				int clash = -1;
				for (int j = 0; j < 12; j++) {
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

		if (RetrogradeButton.process(params[RETROGRADE_BUTTON].getValue())) {
			retrogradeState = !retrogradeState;
		}

		if (InvertButton.process(params[INVERT_BUTTON].getValue()))
			pendInvert = true;

		if (RetrogradeTrigger.process(inputs[RETROGRADE_INPUT].getVoltage())) {
			retrogradeState = !retrogradeState;
		}

		if (InvertTrigger.process(inputs[INVERT_INPUT].getVoltage()))
			pendInvert = true;

		float currentStep = invertState
			? inversion[step]
			: toneRow[step];

		float voltage = currentStep / 12.f;

		outputs[SEQ_OUTPUT].setVoltage(voltage);

		for (int i = 0; i < 12; i++) {
			lights[SEQ_LIGHT + i].setBrightness(i == step ? 1.f : 0.f);
		}

		lights[INVERT_LIGHT].setBrightness(invertState);
		lights[RETROGRADE_LIGHT].setBrightness(retrogradeState);
	   
	}
};


struct SerialiaWidget : ModuleWidget {
	SerialiaWidget(Serialia* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Serialia.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		for (int i = 0; i < 6; i++) {
			float x = 10.5f + i * 12.f;

			addParam(createParamCentered<RoundBlackKnob>(
				mm2px(Vec(x, 25)),
				module,
				Serialia::VALUE_PARAM + i
			));
		}

		for (int i = 0; i < 6; i++) {
			float x = 10.5f + i * 12.f;

			addInput(createInputCentered<PJ301MPort>(
				mm2px(Vec(x, 35)),
				module,
				Serialia::VALUE_INPUT + i
			));
		}

		for (int i = 0; i < 6; i++) {
			float x = 10.5f + i * 12.f;

			addParam(createParamCentered<RoundBlackKnob>(
				mm2px(Vec(x, 45)),
				module,
				Serialia::VALUE_PARAM + ( 6 + i )
			));
		}

		for (int i = 0; i < 6; i++) {
			float x = 10.5f + i * 12.f;

			addInput(createInputCentered<PJ301MPort>(
				mm2px(Vec(x, 55)),
				module,
				Serialia::VALUE_INPUT + ( 6 + i )
			));
		}

		addInput(createInputCentered<PJ301MPort>(
			mm2px(Vec(20, 110)),
			module,
			Serialia::CLOCK_INPUT
		));

		addOutput(createOutputCentered<PJ301MPort>(
			mm2px(Vec(60, 110)),
			module,
			Serialia::SEQ_OUTPUT
		));

		for (int i = 0; i < 12; i++) {
			float x = 6.6f + i * 6.f;

			addChild(createLightCentered<MediumLight<GreenLight>>(
				mm2px(Vec(x, 120)), module,
				Serialia::SEQ_LIGHT + i
			));
		}

		addParam(createParamCentered<LEDButton>(
			mm2px(Vec(35, 67)),
			module,
			Serialia::INVERT_BUTTON
		));

		addChild(createLightCentered<MediumLight<BlueLight>>(
			mm2px(Vec(35, 83)), module,
			Serialia::INVERT_LIGHT
		));

		addInput(createInputCentered<PJ301MPort>(
			mm2px(Vec(35, 75)),
			module,
			Serialia::INVERT_INPUT
		));

		addParam(createParamCentered<LEDButton>(
			mm2px(Vec(45, 67)),
			module,
			Serialia::RETROGRADE_BUTTON
		));

		addChild(createLightCentered<MediumLight<RedLight>>(
			mm2px(Vec(45, 83)), module,
			Serialia::RETROGRADE_LIGHT
		));

		addInput(createInputCentered<PJ301MPort>(
			mm2px(Vec(45, 75)),
			module,
			Serialia::RETROGRADE_INPUT
		));

	}
};


Model* modelSerialia = createModel<Serialia, SerialiaWidget>("Serialia");