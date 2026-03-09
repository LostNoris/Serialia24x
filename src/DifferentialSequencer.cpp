#include "plugin.hpp"


struct DifferentialSequencer : Module {

	int Lerpstep = 0;
	int Sumstep = 0;
	int Diffstep = 0;
	int Prodstep = 0;
	int Valuestep = 0;
	int Targetstep = 0;
	int Morphstep = 0;
	dsp::SchmittTrigger lerpTrigger;
	dsp::SchmittTrigger sumTrigger;
	dsp::SchmittTrigger diffTrigger;
	dsp::SchmittTrigger prodTrigger;
	dsp::SchmittTrigger valueTrigger;
	dsp::SchmittTrigger targetTrigger;
	dsp::SchmittTrigger morphTrigger;

	float values[8];
	float differences[8];
	float sums[8];
	float products[8];
	float sequenceOut[8];

	enum ParamId {
		VALUE_PARAM,
		VALUE_PARAM_LAST = VALUE_PARAM + 7,

		TARGET_PARAM,
		TARGET_PARAM_LAST = TARGET_PARAM + 7,

		MORPH_PARAM,
		MORPH_PARAM_LAST = MORPH_PARAM + 7,

		PARAMS_LEN
	};

	enum InputId {
		LERPCLOCK_INPUT,
		SUMCLOCK_INPUT,
		DIFFCLOCK_INPUT,
		PRODCLOCK_INPUT,
		VALUECLOCK_INPUT,
		TARGETCLOCK_INPUT,
		MORPHCLOCK_INPUT,
		INPUTS_LEN
	};

	enum OutputId {
		LERP_OUTPUT,
		SUM_OUTPUT,
		DIFF_OUTPUT,
		PROD_OUTPUT,
		VALUE_ROW_OUTPUT,
		TARGET_ROW_OUTPUT,
		MORPH_ROW_OUTPUT,
		OUTPUTS_LEN
	};

	enum LightId {
		STEP1_LIGHT,
		STEP2_LIGHT,
		STEP3_LIGHT,
		STEP4_LIGHT,
		STEP5_LIGHT,
		STEP6_LIGHT,
		STEP7_LIGHT,
		STEP8_LIGHT,
		LIGHTS_LEN
	};

	DifferentialSequencer() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

		for (int i = 0; i < 8; i++) {

			configParam(VALUE_PARAM + i, -5.f, 5.f, 0.f, "Step value");

			configParam(TARGET_PARAM + i, 0.f, 7.f, 0.f, "Target step");
			paramQuantities[TARGET_PARAM + i]->snapEnabled = true;

			configParam(MORPH_PARAM + i, 0.f, 1.f, 0.f, "Operator morph");
		}

		configInput(LERPCLOCK_INPUT, "Lerped Clock");
		configInput(SUMCLOCK_INPUT, "Summed Clock");
		configInput(DIFFCLOCK_INPUT, "Difference Clock");
		configInput(PRODCLOCK_INPUT, "Product Clock");

		configInput(VALUECLOCK_INPUT, "Value Clock");
		configInput(TARGETCLOCK_INPUT, "Target Clock");
		configInput(MORPHCLOCK_INPUT, "MorphClock");


		configOutput(LERP_OUTPUT, "Lerped sequence");
		configOutput(SUM_OUTPUT, "Sum sequence");
		configOutput(DIFF_OUTPUT, "Difference sequence");
		configOutput(PROD_OUTPUT, "Product sequence");

		configOutput(VALUE_ROW_OUTPUT, "Value row");
		configOutput(TARGET_ROW_OUTPUT, "Target row");
		configOutput(MORPH_ROW_OUTPUT, "Morph row");
	}

	void process(const ProcessArgs& args) override {

		if (lerpTrigger.process(inputs[LERPCLOCK_INPUT].getVoltage())) {
			Lerpstep = (Lerpstep + 1) % 8;
		}
		if (sumTrigger.process(inputs[SUMCLOCK_INPUT].getVoltage())) {
			Sumstep = (Sumstep + 1) % 8;
		}
		if (diffTrigger.process(inputs[DIFFCLOCK_INPUT].getVoltage())) {
			Diffstep = (Diffstep + 1) % 8;
		}
		if (prodTrigger.process(inputs[PRODCLOCK_INPUT].getVoltage())) {
			Prodstep = (Prodstep + 1) % 8;
		}
		if (valueTrigger.process(inputs[VALUECLOCK_INPUT].getVoltage())) {
			Valuestep = (Valuestep + 1) % 8;
		}
		if (targetTrigger.process(inputs[TARGETCLOCK_INPUT].getVoltage())) {
			Targetstep = (Targetstep + 1) % 8;
		}
		if (morphTrigger.process(inputs[MORPHCLOCK_INPUT].getVoltage())) {
			Morphstep = (Morphstep + 1) % 8;
		}

		float values[8];
		float differences[8];
		float sums[8];
		float products[8];
		float sequenceOut[8];

		for (int i = 0; i < 8; i++)
			values[i] = params[VALUE_PARAM + i].getValue();

		for (int i = 0; i < 8; i++) {

			int target = (int)std::round(params[TARGET_PARAM + i].getValue());
			target = clamp(target, 0, 7);

			float a = values[i];
			float b = values[target];

			differences[i] = fabs(a - b);
			sums[i] = (a + b) * 0.5f;
			products[i] = (a * b) / 25.f * 5.f;

			float morph = params[MORPH_PARAM + i].getValue();

			if (morph < 0.5f)
				sequenceOut[i] = crossfade(differences[i], sums[i], morph * 2.f);
			else
				sequenceOut[i] = crossfade(sums[i], products[i], (morph - 0.5f) * 2.f);
		}

		outputs[LERP_OUTPUT].setVoltage(sequenceOut[Lerpstep]);
		outputs[SUM_OUTPUT].setVoltage(sums[Sumstep]);
		outputs[DIFF_OUTPUT].setVoltage(differences[Diffstep]);
		outputs[PROD_OUTPUT].setVoltage(products[Prodstep]);

		outputs[VALUE_ROW_OUTPUT].setVoltage(values[Valuestep]);
		outputs[TARGET_ROW_OUTPUT].setVoltage(params[TARGET_PARAM + Targetstep].getValue());
		outputs[MORPH_ROW_OUTPUT].setVoltage(params[MORPH_PARAM + Morphstep].getValue());

		//for (int i = 0; i < 8; i++)
			//lights[STEP1_LIGHT + i].setBrightness(i == step ? 1.f : 0.f);
	}
};


struct DifferentialSequencerWidget : ModuleWidget {
	DifferentialSequencerWidget(DifferentialSequencer* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/DifferentialSequencer.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		for (int i = 0; i < 8; i++) {
			float x = 5.f + i * 10.f;

			addParam(createParamCentered<RoundBlackKnob>(
				mm2px(Vec(x, 25)),
				module,
				DifferentialSequencer::VALUE_PARAM + i
			));
		}

		for (int i = 0; i < 8; i++) {
			float x = 5.f + i * 10.f;

			addParam(createParamCentered<RoundBlackKnob>(
				mm2px(Vec(x, 45)),
				module,
				DifferentialSequencer::TARGET_PARAM + i
			));
		}

		for (int i = 0; i < 8; i++) {
			float x = 5.f + i * 10.f;

			addParam(createParamCentered<RoundBlackKnob>(
				mm2px(Vec(x, 65)),
				module,
				DifferentialSequencer::MORPH_PARAM + i
			));
		}

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(0, 110)),module,DifferentialSequencer::LERPCLOCK_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15, 110)),module,DifferentialSequencer::SUMCLOCK_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30, 110)),module,DifferentialSequencer::DIFFCLOCK_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(45, 110)),module,DifferentialSequencer::PRODCLOCK_INPUT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(60, 110)),module,DifferentialSequencer::VALUECLOCK_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(75, 110)),module, DifferentialSequencer::TARGETCLOCK_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(90, 110)),module, DifferentialSequencer::MORPHCLOCK_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(0, 120)), module, DifferentialSequencer::LERP_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(15, 120)), module, DifferentialSequencer::SUM_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(30, 120)), module, DifferentialSequencer::DIFF_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(45, 120)), module, DifferentialSequencer::PROD_OUTPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(60, 120)), module, DifferentialSequencer::VALUE_ROW_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(75, 120)), module, DifferentialSequencer::TARGET_ROW_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(90, 120)), module, DifferentialSequencer::MORPH_ROW_OUTPUT));

		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(5, 15)), module, DifferentialSequencer::STEP1_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(15, 15)), module, DifferentialSequencer::STEP2_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(25, 15)), module, DifferentialSequencer::STEP3_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(35, 15)), module, DifferentialSequencer::STEP4_LIGHT));

		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(45, 15)), module, DifferentialSequencer::STEP5_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(55, 15)), module, DifferentialSequencer::STEP6_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(65, 15)), module, DifferentialSequencer::STEP7_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(75, 15)), module, DifferentialSequencer::STEP8_LIGHT));

	}
};


Model* modelDifferentialSequencer = createModel<DifferentialSequencer, DifferentialSequencerWidget>("DifferentialSequencer");