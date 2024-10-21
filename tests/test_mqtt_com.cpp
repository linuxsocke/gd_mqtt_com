#include <string>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <memory>

#include <signal.h>
#include <stdlib.h>

#include "gd_mqtt_com/mqtt_com/mqtt_com.hpp"

using std::literals::operator""ms; //NOLINT

bool running = false;

static void stopHandler(int sign) {
    (void)sign;
    std::cerr << "received ctrl-c\n";
    running = false;
}

int main(int argc, char* argv[]) {
	signal(SIGINT, stopHandler);
    signal(SIGTERM, stopHandler);


	std::string path_cert  = (argc > 1) ? std::string(argv[1]) : "";

    std::cout << "Testing MqttCom with cert path: " << path_cert << std::endl;

	mqtt_com::MqttComOptions options{
		.host_address=std::getenv("MQTT_BROKER_URL"),
		.client_id=std::getenv("MQTT_BROKER_CLIENT"),
		.username=std::getenv("MQTT_BROKER_USER"),
		.password=std::getenv("MQTT_BROKER_PASSWORD")
	};
	options.server_cert_path = path_cert;

	auto mqtt_com = std::make_shared<mqtt_com::MqttCom>();

	try {
		mqtt_com->setup_mqtt_com(options);
	} catch (std::exception& e) {
        std::cout << "Exception during remote initialisation: "+std::string(e.what()) << std::endl;
		return -1;
	}

    std::cout << "Connecting to mqtt broker .." << std::endl;
	try {
		mqtt_com->connect();
	} catch (std::exception& e) {
        std::cout << "Connection failed with error: "+std::string(e.what()) << std::endl;
		return -1;
	}

	try {
		std::string test_topic = std::getenv("MQTT_TEST_TOPIC");
    	std::cout << "Subscribing to " << test_topic << ": " << test_topic << std::endl;
		mqtt_com->subscribe_to_topic(test_topic, [&](const std::string& heartbeat){
			std::cout << "Received heartbeat from " << heartbeat << std::endl;
		});
	} catch (std::exception& e) {
		std::cout << "Subscribing to heartbeat failed with error: "+std::string(e.what()) << std::endl;
		return -1;
	}

	running = true;
    std::cout << "Starting iterations." << std::endl;
    while(running) {
        try {
            mqtt_com->iterate();
        } catch (std::exception& e) {
            std::cout << "Exception during connection iteration: "+std::string(e.what()) << std::endl;
            std::this_thread::sleep_for(10ms);
            continue;
        }
        std::this_thread::sleep_for(10ms);
    }

    std::cout << "Mqtt com disconnecting .." << std::endl;
	try{
		mqtt_com->disconnect();
	} catch (std::exception& e) {
        std::cout << "Disconnect failed with error: "+std::string(e.what()) << std::endl;
		return -1;
	}
    std::cout << "Mqtt com disconnected." << std::endl;

    return 0;
}