#include <thread>
#include <chrono>

#include <godot_cpp/core/class_db.hpp>

#include "gd_mqtt_com/gd_mqtt_com.hpp"
#include "gd_mqtt_com/mqtt_com/mqtt_com.hpp"

namespace mqtt_com {

using namespace godot;

void GDMqttCom::_bind_methods() {
	ClassDB::bind_method(D_METHOD("setup_mqtt_com"), &GDMqttCom::setup_mqtt_com);
	ClassDB::bind_method(D_METHOD("connect"), &GDMqttCom::connect);
	ClassDB::bind_method(D_METHOD("end_connection"), &GDMqttCom::end_connection);
	ClassDB::bind_method(D_METHOD("reconnect"), &GDMqttCom::reconnect);
	ClassDB::bind_method(D_METHOD("is_connected"), &GDMqttCom::is_connected);
	ClassDB::bind_method(D_METHOD("iterate"), &GDMqttCom::iterate);
	ClassDB::bind_method(D_METHOD("subscribe_to_topic"), &GDMqttCom::subscribe_to_topic);
	ClassDB::bind_method(D_METHOD("unsubscribe_to_topic"), &GDMqttCom::unsubscribe_to_topic);
	ClassDB::bind_method(D_METHOD("publish_to_topic"), &GDMqttCom::publish_to_topic);

	// TODO connected signal for await coroutines https://docs.godotengine.org/en/latest/tutorials/scripting/gdscript/gdscript_basics.html#awaiting-signals-or-coroutines
	//ADD_SIGNAL(MethodInfo(
	//	StringName(SIGNAL_NAME.c_str()),
	//	PropertyInfo(Variant::STRING, "msg_json_str")
	//));

	BIND_CONSTANT(GD_SUCCESS);
	BIND_CONSTANT(GD_ERROR);
}

GDMqttCom::GDMqttCom() {
	_mqtt_com = std::make_shared<MqttCom>();
	UtilityFunctions::print("Construction of GDMqttCom done.");
}

GDMqttCom::~GDMqttCom() {
	// Add your cleanup here.
	if (_mqtt_com->is_connected()) {
		this->end_connection();
	}
}

int GDMqttCom::setup_mqtt_com(
	const String& host_address, 
	const String& client_id,
	const String& username, 
	const String& password ,
	const String& server_cert_path) {
	mqtt_com::MqttComOptions options{
		.host_address=host_address.utf8().get_data(),
		.client_id=client_id.utf8().get_data(),
		.username=username.utf8().get_data(),
		.password=password.utf8().get_data(),
		.server_cert_path=server_cert_path.utf8().get_data()
	};
	try {
		_mqtt_com->setup_mqtt_com(options);
	} catch (std::exception& e) {
		UtilityFunctions::printerr(("Setup remote com faield with excetption: "+std::string(e.what())).c_str());
		return GD_ERROR;
	}
	return GD_SUCCESS;
}

int GDMqttCom::connect() {
	if (_connection_future.valid() && 
		_connection_future.wait_for(std::chrono::milliseconds(0))!=std::future_status::ready) {
		UtilityFunctions::print("[GDMqttCom] Process busy.");
		return GD_ERROR;
	}
	if (_mqtt_connecting.load()){
		return GD_SUCCESS;
	}
	_mqtt_connecting.store(true);
	_connection_future = std::async(std::launch::async, [this](){
			try {
				_mqtt_com->connect();
			} catch (std::exception& e) {
				UtilityFunctions::printerr(("[GDMqttCom] Failed connecting with exception: "+std::string(e.what())).c_str());
			}
			_mqtt_connecting.store(false);
		});

	return GD_SUCCESS;
}

int GDMqttCom::reconnect() {
	if (_mqtt_connecting.load()){
		return GD_SUCCESS;
	}
	_mqtt_connecting.store(true);
	_connection_future = std::async(std::launch::async, [this](){
			try {
				_mqtt_com->reconnect();
			} catch (std::exception& e) {
				UtilityFunctions::printerr(("[GDMqttCom] Failed reconnecting with exception: "+std::string(e.what())).c_str());
			}
			_mqtt_connecting.store(false);
		});
	return GD_SUCCESS;
}

int GDMqttCom::end_connection() {
	try {
		_mqtt_com->disconnect();
	} catch (std::exception& e) {
		UtilityFunctions::printerr(("[GDMqttCom] Disconnecting failed with exception: "+std::string(e.what())).c_str());
		return GD_ERROR;
	}
	return GD_SUCCESS;
}

bool GDMqttCom::is_connected(){
	if (_mqtt_connecting.load()){
		return false;
	}
	return _mqtt_com->is_connected();
}

void GDMqttCom::iterate(){
	if (_mqtt_connecting.load()){
		UtilityFunctions::print("[GDMqttCom] Still connection. Skipping iterating.");
		return;
	}
	try {
		_mqtt_com->iterate();
	} catch (std::exception& e) {
		UtilityFunctions::printerr(("[GDMqttCom] Exception during iteration: "+std::string(e.what())).c_str());
	}
}

int GDMqttCom::subscribe_to_topic(const String topic_name, const Callable& cb) {
	if (_mqtt_connecting.load()){
		UtilityFunctions::print("[GDMqttCom] Still connection. Skipping subscribing topic "+topic_name);
		return GD_ERROR;
	}
	UtilityFunctions::print("[GDMqttCom] Called subscribe topic: "+topic_name);
	try {
		_mqtt_com->subscribe_to_topic(topic_name.utf8().get_data(), [cb](const std::string& json_str){
				cb.call(String(json_str.c_str()));
			});
	} catch (std::exception& e) {
		UtilityFunctions::printerr(("[GDMqttCom] Exception during remote subscription: "+std::string(e.what())+". Topic "+topic_name.utf8().get_data()+" not subscribed.").c_str());
		return GD_ERROR;
	}
	return GD_SUCCESS;
}

int GDMqttCom::unsubscribe_to_topic(const String topic_name) {
	if (_mqtt_connecting.load()){
		UtilityFunctions::print("[GDMqttCom] Still connection. Failed subscribing topic "+topic_name);
		return GD_ERROR;
	}
	UtilityFunctions::print("[GDMqttCom] Called unsubscribe topic: "+topic_name);
	try {
		_mqtt_com->unsubscribe_to_topic(topic_name.utf8().get_data());
	} catch (std::exception& e) {
		UtilityFunctions::printerr(("[GDMqttCom] Exception during remote unsubscription: "+std::string(e.what())).c_str());
		return GD_ERROR;
	}
	return GD_SUCCESS;
}

int GDMqttCom::publish_to_topic(const String topic_name, const String json_str) {
	if (_mqtt_connecting.load()){
		UtilityFunctions::print("[GDMqttCom] Still connection. Failed publishing to topic "+topic_name);
		return GD_ERROR;
	}
	UtilityFunctions::print("[GDMqttCom] Called publish topic: "+topic_name+" Message: "+json_str);
	try {
		_mqtt_com->publish_to_topic(topic_name.utf8().get_data(), json_str.utf8().get_data());
	} catch (std::exception& e) {
		UtilityFunctions::printerr(("[GDMqttCom] Exception during remote publish: "+std::string(e.what())+". Not published to topic "+topic_name.utf8().get_data()+".").c_str());
		return GD_ERROR;
	}
	return GD_SUCCESS;
}

}
