#include <cmath>
#include <vector>

#include "plugin.hpp"

struct Filter : Module {
  enum ParamId { POWER_PARAM, FRQ_PARAM, PARAMS_LEN };
  enum InputId { IN_1_INPUT, INPUTS_LEN };
  enum OutputId { OUT_1_OUTPUT, OUT_2_OUTPUT, OUT_3_OUTPUT, OUTPUTS_LEN };
  enum LightId { POWER_LIGHT_LIGHT, LIGHTS_LEN };

  std::vector<float> voltages;
  bool state_on = false;
  bool last_state = false;
  float prevLowPassSample = 0.0f;
  float prevHighPassSample = 0.0f;
  float prevInputSample = 0.0f;
  float sampleRate;

  Filter() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configButton(Filter::POWER_PARAM, "Power Trigger");
    configParam(Filter::FRQ_PARAM, -250.f, 250.f, 0.f, "Cutoff frequency",
                " Hz");
    configInput(Filter::IN_1_INPUT, "Signal Input");
    configOutput(Filter::OUT_1_OUTPUT, "Low Pass Output");
    configOutput(Filter::OUT_2_OUTPUT, "Band Pass Output");
    configOutput(Filter::OUT_3_OUTPUT, "High Pass Output");
    configLight(Filter::POWER_LIGHT_LIGHT, "Power State");
    sampleRate = APP->engine->getSampleRate();
  }

  float calculateAlpha(float cutoff, float sampleRate) {
    cutoff = std::fabs(cutoff);
    if (cutoff > sampleRate / 2.0f) {
      cutoff = sampleRate / 2.0f;
    }
    float omega = 2.0f * M_PI * cutoff / sampleRate;
    return omega / (omega + 1.0f);
  }

  void updatePowerState() {
    bool current_state = params[POWER_PARAM].getValue() == 1;
    if (current_state && !last_state) {
      state_on = !state_on;
    }
    last_state = current_state;

    lights[POWER_LIGHT_LIGHT].setBrightness(state_on ? 1.0f : 0.0f);
  }

  void disableOutput() {
    outputs[OUT_1_OUTPUT].setChannels(0);
    outputs[OUT_2_OUTPUT].setChannels(0);
    outputs[OUT_3_OUTPUT].setChannels(0);
  }

  void ApplyLowPass() {
    if (!outputs[OUT_1_OUTPUT].isConnected()) return;
    outputs[OUT_1_OUTPUT].setChannels(voltages.size());

    float cutoff = params[FRQ_PARAM].getValue();
    // if (cutoff < 0) return;

    float alpha = calculateAlpha(cutoff, sampleRate);

    for (size_t i = 0; i < voltages.size(); ++i) {
      voltages[i] = alpha * voltages[i] + (1.0f - alpha) * prevLowPassSample;
      prevLowPassSample = voltages[i];
    }

    outputs[OUT_1_OUTPUT].writeVoltages(voltages.data());
  }

  void ApplyBandPass() {
    if (!outputs[OUT_2_OUTPUT].isConnected()) return;
    outputs[OUT_2_OUTPUT].setChannels(voltages.size());

    float cutoff = params[FRQ_PARAM].getValue();
    // if (cutoff == 0) return;  // Skip if cutoff is zero for band-pass filter

    float lowCutoff = std::fabs(cutoff - 5);
    float highCutoff = std::fabs(cutoff + 5);

    float alphaLow = calculateAlpha(lowCutoff, sampleRate);
    float alphaHigh = calculateAlpha(highCutoff, sampleRate);

    for (size_t i = 0; i < voltages.size(); ++i) {
      float lowPass =
          alphaLow * voltages[i] + (1.0f - alphaLow) * prevLowPassSample;
      prevLowPassSample = lowPass;
      float highPass =
          alphaHigh * (prevHighPassSample + voltages[i] - prevInputSample);
      prevHighPassSample = highPass;
      prevInputSample = voltages[i];
      voltages[i] = lowPass - highPass;
    }

    outputs[OUT_2_OUTPUT].writeVoltages(voltages.data());
  }
  void ApplyHighPass() {
    if (!outputs[OUT_3_OUTPUT].isConnected()) return;
    outputs[OUT_3_OUTPUT].setChannels(voltages.size());

    float cutoff = params[FRQ_PARAM].getValue();
    // if (cutoff > 0) return;
    float alpha = calculateAlpha(cutoff, sampleRate);

    for (size_t i = 0; i < voltages.size(); ++i) {
      voltages[i] =
          alpha * (prevHighPassSample + voltages[i] - prevInputSample);
      prevHighPassSample = voltages[i];
      prevInputSample = voltages[i];
    }

    outputs[OUT_3_OUTPUT].writeVoltages(voltages.data());
  }

  void GetInput() {
    voltages.clear();
    if (!inputs[IN_1_INPUT].isConnected()) return;

    int channels = inputs[IN_1_INPUT].getChannels();
    voltages.resize(channels, 0.0f);

    inputs[IN_1_INPUT].readVoltages(voltages.data());
  }

  void process(const ProcessArgs& args) override {
    updatePowerState();

    if (!state_on) {
      disableOutput();
      return;
    } else {
      GetInput();
      ApplyLowPass();
      ApplyBandPass();
      ApplyHighPass();
    }
  }
};

struct FilterWidget : ModuleWidget {
  FilterWidget(Filter* module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/Filter.svg")));

    addChild(createWidget<ScrewSilver>(Vec(15, 0)));
    addChild(createWidget<ScrewSilver>(Vec(15, 363)));

    addParam(createParamCentered<VCVButton>(Vec(17, 62.5), module,
                                            Filter::POWER_PARAM));
    addParam(createParamCentered<RoundBlackKnob>(Vec(23, 127.5), module,
                                                 Filter::FRQ_PARAM));

    addInput(createInputCentered<PJ301MPort>(Vec(23, 187.5), module,
                                             Filter::IN_1_INPUT));

    addOutput(createOutputCentered<PJ301MPort>(Vec(23, 240.5), module,
                                               Filter::OUT_1_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(Vec(23, 294), module,
                                               Filter::OUT_2_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(Vec(23, 347.5), module,
                                               Filter::OUT_3_OUTPUT));

    addChild(createLightCentered<MediumLight<RedLight>>(
        Vec(37.5, 62.5), module, Filter::POWER_LIGHT_LIGHT));
  }
};

Model* modelFilter = createModel<Filter, FilterWidget>("Filter");
