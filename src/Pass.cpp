/**
 * @file Pass.cpp
 * @author Casey Stijlaart (casey.stijlaart@hotmail.com)
 * @brief Model definition of the Pass for VCV Rack 2.
 * @version 1.0
 * @date 2024-06-06
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "plugin.hpp"

struct Pass : Module {
  enum ParamId { POWER_PARAM, SUM_PARAM, AVG_PARAM, PARAMS_LEN };
  enum InputId { IN_1_INPUT, IN_2_INPUT, IN_3_INPUT, INPUTS_LEN };
  enum OutputId { OUT_1_OUTPUT, OUTPUTS_LEN };
  enum LightId {
    POWER_LIGHT_LIGHT,
    SUM_LIGHT_LIGHT,
    AVG_LIGHT_LIGHT,
    LIGHTS_LEN
  };

  int num_channels = 0;
  std::vector<float> voltages;
  bool state_on = false;
  bool last_state = false;

  bool state_on_avg = false;
  bool last_state_avg = false;

  bool state_on_sum = false;
  bool last_state_sum = false;

  Pass() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configButton(Pass::POWER_PARAM, "Power Trigger");
    configButton(Pass::SUM_PARAM, "Sum Trigger");
    configButton(Pass::AVG_PARAM, "AVG Trigger");

    configInput(Pass::IN_1_INPUT, "Track 1");
    configInput(Pass::IN_2_INPUT, "Track 2");
    configInput(Pass::IN_3_INPUT, "Track 3");

    configOutput(Pass::OUT_1_OUTPUT, "Audio Output");

    configLight(Pass::POWER_LIGHT_LIGHT, "Power Status");
    configLight(Pass::SUM_LIGHT_LIGHT, "Sum Status");
    configLight(Pass::AVG_LIGHT_LIGHT, "Avg Status");
  }

  void process(const ProcessArgs& args) override {
    updatePowerState();

    if (!state_on) {
      disableOutput();
      return;
    } else {
      updateModeStates();

      if (state_on_sum || state_on_avg) {
        processInputs();
      }

      if (num_channels > 0) {
        if (state_on_avg) {
          applyAverage();
        }
        sendOutput();
      }
    }
  }

  void updatePowerState() {
    bool current_state = params[POWER_PARAM].getValue() == 1;
    if (current_state && !last_state) {
      state_on = !state_on;
    }
    last_state = current_state;

    lights[POWER_LIGHT_LIGHT].setBrightness(state_on ? 1.0f : 0.0f);
  }

  void updateModeStates() {
    bool current_state_sum = params[SUM_PARAM].getValue() == 1;
    if (current_state_sum && !last_state_sum) {
      state_on_sum = true;
      state_on_avg = false;
    }
    last_state_sum = current_state_sum;

    bool current_state_avg = params[AVG_PARAM].getValue() == 1;
    if (current_state_avg && !last_state_avg) {
      state_on_avg = true;
      state_on_sum = false;
    }
    last_state_avg = current_state_avg;

    lights[SUM_LIGHT_LIGHT].setBrightness(state_on_sum ? 1.0f : 0.0f);
    lights[AVG_LIGHT_LIGHT].setBrightness(state_on_avg ? 1.0f : 0.0f);
  }

  void processInputs() {
    voltages.clear();
    num_channels = 0;
    processInput(inputs[IN_1_INPUT]);
    processInput(inputs[IN_2_INPUT]);
    processInput(inputs[IN_3_INPUT]);
  }

  void processInput(Input& input) {
    if (!input.isConnected()) return;

    int channels = input.getChannels();
    if (channels > voltages.size()) {
      voltages.resize(channels, 0.0f);
    }

    std::vector<float> inputVoltages(channels);
    input.readVoltages(inputVoltages.data());

    for (int i = 0; i < channels; ++i) {
      voltages[i] += inputVoltages[i];
    }

    num_channels += input.getChannels();
  }

  void applyAverage() {
    for (float& voltage : voltages) {
      voltage /= num_channels;
    }
  }

  void sendOutput() {
    outputs[OUT_1_OUTPUT].setChannels(1);
    outputs[OUT_1_OUTPUT].writeVoltages(voltages.data());
  }

  void disableOutput() {
    outputs[OUT_1_OUTPUT].setChannels(0);
    state_on_sum = false;
    state_on_avg = false;
    lights[SUM_LIGHT_LIGHT].setBrightness(0.0f);
    lights[AVG_LIGHT_LIGHT].setBrightness(0.0f);
  }
};

struct PassWidget : ModuleWidget {
  PassWidget(Pass* module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/Pass.svg")));

    addChild(createWidget<ScrewSilver>(Vec(15, 0)));
    addChild(createWidget<ScrewSilver>(Vec(15, 375)));

    addParam(createParamCentered<VCVButton>(Vec(17, 52.5), module,
                                            Pass::POWER_PARAM));

    addParam(
        createParamCentered<VCVButton>(Vec(17, 91.5), module, Pass::SUM_PARAM));
    addParam(createParamCentered<VCVButton>(Vec(17, 130.5), module,
                                            Pass::AVG_PARAM));

    addInput(createInputCentered<PJ301MPort>(Vec(23, 188.5), module,
                                             Pass::IN_1_INPUT));
    addInput(createInputCentered<PJ301MPort>(Vec(23, 242.5), module,
                                             Pass::IN_2_INPUT));
    addInput(createInputCentered<PJ301MPort>(Vec(23, 296.5), module,
                                             Pass::IN_3_INPUT));

    addOutput(createOutputCentered<PJ301MPort>(Vec(23, 350), module,
                                               Pass::OUT_1_OUTPUT));

    addChild(createLightCentered<MediumLight<RedLight>>(
        Vec(37.5, 52.5), module, Pass::POWER_LIGHT_LIGHT));
    addChild(createLightCentered<MediumLight<RedLight>>(Vec(37.5, 91.5), module,
                                                        Pass::SUM_LIGHT_LIGHT));
    addChild(createLightCentered<MediumLight<RedLight>>(
        Vec(37.5, 130.5), module, Pass::AVG_LIGHT_LIGHT));
  }
};

Model* modelPass = createModel<Pass, PassWidget>("Pass");
