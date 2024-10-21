#ifndef GD_EXTENSION_MQTT_COM
#define GD_EXTENSION_MQTT_COM

#include <string>
#include <atomic>
#include <memory>
#include <unordered_map>
#include <vector>
#include <functional>
#include <iostream>

#include <mqtt/async_client.h>
#include <mqtt/connect_options.h>
#include <mqtt/string_collection.h>

namespace mqtt_com {

struct MqttComOptions {
    std::string host_address{""};
    std::string client_id{""};
    std::string username{""};
    std::string password{""};
    std::string server_cert_path{""};
    std::string client_cert_path{""};
    std::string client_key_path{""};
};

//enum ResultCode {
//    SUCCESS,
//    ERROR,
//    ERROR_CONNECTION,
//    ERROR_AUTHENTIFICATION,
//    ERROR_EXCEPTION
//};

typedef std::function<void(const std::string&)> SubscriptionCallback;

class MqttCom {
public:
    MqttCom(); //= default;
    ~MqttCom(); //= default;

    void setup_mqtt_com(const MqttComOptions&);

    /**
     * Connect to an MQTT broker
     * Instructs the broker to clean all messages and subscriptions on disconnect.
     * Keep alive is set to 60 seconds.
     * @return the result code of the remote conenction.
     */
    void connect();
    void reconnect();
    void disconnect();

    bool is_connected();

    void iterate();

    void subscribe_to_topic(const std::string& topic_name, const SubscriptionCallback& cb);
    void unsubscribe_to_topic(const std::string& topic_name);

    void publish_to_topic(const std::string& topic_name, const std::string& json_str);

private:
    /**
     * Connect to the broker
     * @param p_clean_session set to true to instruct the broker to clean all messages and 
     * subscriptions on disconnect, false to instruct it to keep them
     * @param p_keep_alive keep alive time
     * @param p_automatic_reconnect if true then automatic reconnect
     * @return the reason code, if something wrong happen. 0 = OK 
     * (see https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901031)
     */
    int connect_to_broker(
        const bool p_clean_session = true, 
        const int  p_keep_alive = 60, 
        const bool p_automatic_reconnect = true);

    /**
     * Connect to the broker
     * @return the reason code, if something wrong happen. 0 = OK (see https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901031)
     */
    int disconnect_to_broker();

    /**
     * Connect to the broker
     * @param p_topic the name of the topic
     * @param p_qos the QoS used
     * @return the reason code, if something wrong happen. 0 = OK (see https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901031)
     */
    int subscribe_to(const std::string& p_topic, const int p_qos = 1);

    /**
     * Unscubscribe to specific topic
     * @param p_topic the name of the topic
     * @return the reason code, if something wrong happen. 0 = OK (see https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901031)
     */
    int unsubscribe_to(const std::string& p_topic);

    /**
     * Connect to the broker
     * @param p_topic the name of the topic
     * @param p_data data to send
     * @param p_qos the QoS used
     * @param p_retain if true the data is retained
     * @return the reason code, if something wrong happen. 0 = OK (see https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901031)
     */
    int publish_to(
        const std::string& p_topic, 
        const std::string& p_data, 
        const int p_qos = 1, 
        const bool p_retain = false);

    MqttComOptions _mqtt_com_options;

    /**
     * Create an MQTT client using a smart pointer to be shared among threads.
     */
    std::shared_ptr<mqtt::async_client> _client;

    // TODO use mutex to protect client
    // mutex for the case the client is used offthread
    std::mutex _mutex_client;

    std::atomic<bool> _is_connected{false};

    /**
     * Connect options for a persistent session and automatic reconnects.
     */
    mqtt::connect_options _m_connection_options;
  
    std::unordered_map<std::string, std::vector<SubscriptionCallback>> _subscriptions_cb_map;

};

}

#endif

