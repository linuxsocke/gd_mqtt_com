#ifndef GD_EXTENSION_GD_MQTT_COM
#define GD_EXTENSION_GD_MQTT_COM

#include <memory>
#include <functional>
#include <atomic>
#include <future>
#include <mutex>
#include <stdexcept>

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/callable.hpp>

#include "gd_mqtt_com/mqtt_com/mqtt_com.hpp"

enum : int {
	GD_SUCCESS,
	GD_ERROR
};

namespace mqtt_com {

using namespace godot;

// TODO add error signal

class GDMqttCom : public RefCounted {
	GDCLASS(GDMqttCom, RefCounted)
public:

private:
	std::shared_ptr<MqttCom> _mqtt_com;

    std::future<void> _connection_future;
    std::atomic<bool> _mqtt_connecting{false};

protected:
	static void _bind_methods();

public:
	GDMqttCom();
	~GDMqttCom();

	int setup_mqtt_com(
	const String& host_address, 
	const String& client_id,
	const String& username, 
	const String& password ,
	const String& server_cert_path);

	int connect();
	int reconnect();
	int end_connection();

	bool is_connected();

	void iterate();

	int subscribe_to_topic(const String topic_name, const Callable& cb);
	int unsubscribe_to_topic(const String topic_name);

	int publish_to_topic(const String topic_name, const String json_str);
};

}

#endif