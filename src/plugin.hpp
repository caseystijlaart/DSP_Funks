#pragma once
#include <rack.hpp>

using namespace rack;

// Declare the Plugin, defined in plugin.cpp
extern Plugin* pluginInstance;

static const float HALF_KNOB_MED = 38 * 0.5;
static const float HALF_KNOB_SMALL = 28 * 0.5;
static const float HALF_KNOB_TINY = 18 * 0.5;
static const float HALF_PORT = 31.58 * 0.5;
static const float HALF_BUTTON = 30 * 0.5;
static const float HALF_BUTTON_SMALL = 15 * 0.5;
static const float HALF_SLIDER = 15 * 0.5;
static const float HALF_LIGHT_TINY = mm2px(1.0) * 0.5f;
static const float HALF_LIGHT_SMALL =
    mm2px(2.0) * 0.5f;  // Was 6.4252f in Rack 1
static const float HALF_LIGHT_MEDIUM = mm2px(3.0) * 0.5f;
static const float HALF_LIGHT_LARGE = mm2px(5.0) * 0.5f;

// Declare each Model, defined in each module source file
extern Model* modelPass;
