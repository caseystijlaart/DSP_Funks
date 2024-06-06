RACK_DIR ?= ../Rack-SDK
#RACK_DIR=~/Rack-SDK-2.0.0

FLAGS +=
#FLAGS += -w
CFLAGS +=
CXXFLAGS +=

LDFLAGS +=

SOURCES += src/plugin.cpp
SOURCES += $(wildcard src/*.cpp)

DISTRIBUTABLES += $(wildcard LICENSE*) res

include $(RACK_DIR)/plugin.mk