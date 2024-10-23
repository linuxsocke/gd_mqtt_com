#!/bin/bash

sudo apt install cmake git \
    libpaho-mqtt-dev libpaho-mqttpp-dev

if [ $? -ne 0 ]; then
    exit 1
fi

if [ ! -d "godot-cpp" ]; then
    git clone https://github.com/godotengine/godot-cpp.git godot-cpp --branch godot-4.3-stable
fi

cmake -DCMAKE_BUILD_TYPE=Release ./godot-cpp -B ./godot-cpp/build
cmake --build ./godot-cpp/build

./build_gd_mqtt.sh  --godot-cpp-path=./godot-cpp

if [ ! -d "demo/addons/bin" ]; then
    mkdir -p demo/addons/bin
fi
cp gdmqttcom.gdextension ./demo/addons/bin/
cp ./bin/x11/*.so ./demo/addons/bin/
echo -e "\e[1;34mExample installed.\e[0m"