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
STATIC_GSTREAMER_PATH=""

# Extract arguments and values
for arg in "$@"
do
  case $arg in
    --godot-cpp-path=*)
    GODOT_CPP_PATH="${arg#*=}"
    shift
    ;;
    --thirdparty-path=*)
    STATIC_GSTREAMER_PATH="${arg#*=}"
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
if [ -z "$STATIC_GSTREAMER_PATH" ]; then
  # --- BUILDING gd_mqtt_com with shared gstreamer
  cmake ./ -B ./build \
      -DGODOT_CPP_PATH=$GODOT_CPP_PATH \
      -DCMAKE_BUILD_TYPE=Release || exit 1
  echo -e "\e[1;34mCompliling gd_mqtt_com with shared gstreamer ..\e[0m"
  cmake --build ./build
  if [ $? -ne 0 ]; then
      echo "\e[1;31mFailed compiling gd_mqtt_com.\e[0m" 
      exit 1
  fi
  echo -e "\e[1;34mBuild finished.\e[0m"
else

  cd ./gd_remote_com/thirdparty/paho.mqtt.cpp
  ln -s ./src/externals ./externals
  cmake . -B ./build -DPAHO_BUILD_SHARED=FALSE -DPAHO_BUILD_STATIC=TRUE -DPAHO_WITH_MQTT_C=TRUE -DPAHO_HIGH_PERFORMANCE=TRUE -DPAHO_WITH_SSL=TRUE -DPAHO_BUILD_SAMPLES=FALSE -DPAHO_ENABLE_TESTING=FALSE -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=./install
  cmake --build build --target install
  rm ./externals
  cd ../../..
  cmake ./gd_remote_com -B ./gd_remote_com/build \
      -DCPP_BINDINGS_PATH=../../godot-cpp \
      -DGODOT_GDEXTENSION_DIR=../../godot-cpp/gdextension/ \
      -DCMAKE_BUILD_TYPE=Release || exit 1
  echo "Make build instructions generated."
  cmake --build ./gd_remote_com/build
  if [ $? -ne 0 ]; then
      echo -e "\e[1;31mFailed compiling gd_remote_com.\e[0m" 
      exit 1
  fi
  echo "Build finished."

  # --- BUILDING gd_mqtt_com with static gstreamer
  cmake ./ -B ./build \
      -DGODOT_CPP_PATH=$GODOT_CPP_PATH \
      -DSTATIC_GSTREAMER=ON \
      -DLINUX_UTILS_PATH=${STATIC_GSTREAMER_PATH}/util-linux-2.39.1 \
      -DELFUTILS_PATH=${STATIC_GSTREAMER_PATH}/elfutils-0.191 \
      -DGLIB_PATH=${STATIC_GSTREAMER_PATH}/glib-2.81.2 \
      -DSRT_PATH=${STATIC_GSTREAMER_PATH}/srt-v1.5.3 \
      -DGSTREAMER_PATH=${STATIC_GSTREAMER_PATH}/gstreamer-1.24.7 \
      -DCMAKE_BUILD_TYPE=Release || exit 1
  echo -e "\e[1;34mCompliling gd_mqtt_com with static gstreamer ..\e[0m"
  cmake --build ./build
  if [ $? -ne 0 ]; then
      echo "\e[1;31mFailed compiling gd_mqtt_com.\e[0m" 
      exit 1
  fi
  echo -e "\e[1;34mBuild finished.\e[0m"
fi
## ========================================================================== #
##| END   | GD Gst Player

