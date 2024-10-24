#!/bin/bash

set -e

# Display usage message
usage() {
  echo "Usage: $0 --godot-cpp-path=/path/to/godot-cpp (required) --static-gstreamer-path=./thirdparty/<distro> (optional if a static gstreamer build should be used)."
  exit 1
}

if [ $# -lt 1 ]; then
  echo "No arguments provided."
  usage
fi

# Initialize variables
GODOT_CPP_PATH=""
STATIC_PAHO_MQTT_PATH=""

# Extract arguments and values
for arg in "$@"
do
  case $arg in
    --godot-cpp-path=*)
    GODOT_CPP_PATH="${arg#*=}"
    shift
    ;;
    --static-mqtt-path=*)
    STATIC_PAHO_MQTT_PATH="${arg#*=}"
    shift
    ;;
    *)
    echo "Error: Unknown argument."
    usage
    ;;
  esac
done

if [ -z "$GODOT_CPP_PATH" ]; then
  echo "No godot cpp path provided."
  usage
fi

#| BEGIN | GD Gst Player
# ========================================================================== #
if [ -z "$STATIC_PAHO_MQTT_PATH" ]; then
  # --- BUILDING gd_mqtt_com with shared paho mqtt
  cmake ./ -B ./build \
      -DGODOT_CPP_PATH=$GODOT_CPP_PATH \
      -DCMAKE_BUILD_TYPE=Release || exit 1
  echo -e "\e[1;34mCompliling gd_mqtt_com with shared paho mqtt ..\e[0m"
  cmake --build ./build
  if [ $? -ne 0 ]; then
      echo "\e[1;31mFailed compiling gd_mqtt_com.\e[0m" 
      exit 1
  fi
  echo -e "\e[1;34mBuild finished.\e[0m"
else
  # --- BUILDING gd_mqtt_com with static paho mqtt
  cmake ./ -B ./build \
      -DGODOT_CPP_PATH=$GODOT_CPP_PATH \
      -DSTATIC_PAHO_MQTT=ON \
      -DSTATIC_PAHO_MQTT_PATH=$STATIC_PAHO_MQTT_PATH \
      -DCMAKE_BUILD_TYPE=Release || exit 1
  echo -e "\e[1;34mCompliling gd_mqtt_com with static paho mqtt ..\e[0m"
  cmake --build ./build
  if [ $? -ne 0 ]; then
      echo "\e[1;31mFailed compiling gd_mqtt_com.\e[0m" 
      exit 1
  fi
  echo -e "\e[1;34mBuild finished.\e[0m"
fi
## ========================================================================== #
##| END   | GD Gst Player

