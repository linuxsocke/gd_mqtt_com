#include "gd_mqtt_com/mqtt_com/mqtt_com.hpp"

#include <chrono>
#include <stdexcept>

namespace mqtt_com {

MqttCom::MqttCom(){
}

MqttCom::~MqttCom(){
    if (_client && _client->is_connected()) {
        disconnect();
    }
}

void MqttCom::setup_mqtt_com(const MqttComOptions& mqtt_com_options) {
    _mqtt_com_options = mqtt_com_options;
	_m_connection_options = mqtt::connect_options_builder()
        .clean_session(true)
        .finalize();

    mqtt::ssl_options ssl_options = mqtt::ssl_options();
    ssl_options.set_error_handler([](const std::string& msg) {
            std::cerr << "[ERROR] SSL Error: " << msg << std::endl;
        });
    if (_mqtt_com_options.server_cert_path!=""){
        ssl_options.set_trust_store(_mqtt_com_options.server_cert_path);
    }
    if (_mqtt_com_options.client_cert_path!="" &&
        _mqtt_com_options.client_key_path!=""){
        ssl_options.set_key_store  (_mqtt_com_options.client_cert_path);
        ssl_options.set_private_key(_mqtt_com_options.client_key_path);
    } else if (mqtt_com_options.username!="" && mqtt_com_options.password!="") {
        _m_connection_options.set_user_name(mqtt_com_options.username);
        _m_connection_options.set_password (mqtt_com_options.password);
    } else {
        throw std::runtime_error("Certificate path or username + password not set.");
    }
    _m_connection_options.set_ssl(ssl_options);

    _client = std::make_shared<mqtt::async_client>(
        _mqtt_com_options.host_address,
        _mqtt_com_options.client_id, 
        mqtt::create_options(MQTTVERSION_3_1_1)
    );
}


void MqttCom::connect() {
    switch (auto reason_code = connect_to_broker(true, 0, false)){
    case mqtt::ReasonCode::SUCCESS:
        _is_connected.store(true);
        break;
    
    default:
        throw std::runtime_error("Unhandled mqtt reason code: "+std::to_string(reason_code));
    }
}

void MqttCom::reconnect() {
    try {
        const mqtt::token_ptr& l_token = _client->reconnect();
		l_token->wait_for(std::chrono::milliseconds{3000});
        if (!_client->is_connected()){
            throw std::runtime_error("Client not connnected. Mqtt reason code: "+std::to_string(l_token->get_reason_code()));
        }
        _client->start_consuming();
        if (l_token->get_reason_code()!=mqtt::ReasonCode::SUCCESS) {
            throw std::runtime_error("Unhandled mqtt reason code: "+std::to_string(l_token->get_reason_code()));
        }
        _is_connected.store(true);
    } catch (const mqtt::exception& p_exception) {
        throw std::runtime_error(p_exception.get_error_str());
    }

    for (auto& topic_cb_pair: _subscriptions_cb_map){
        switch (auto reason_code = subscribe_to(topic_cb_pair.first)){
        case mqtt::ReasonCode::SUCCESS:
            break;

        default:
            throw std::runtime_error("Unhandled mqtt reason code: "+std::to_string(reason_code));
        }
    }
}

void MqttCom::disconnect() {
    switch (auto reason_code = disconnect_to_broker()){
    case mqtt::ReasonCode::SUCCESS:
        break;
    
    default:
        throw std::runtime_error("Unhandled mqtt reason code: "+std::to_string(reason_code));
    }
}

bool MqttCom::is_connected(){
    // - this mechanism is in order to catch an error where mqtt crashes 
    //   if client is illdefined
    if (_is_connected.load())
        _is_connected.store(_client->is_connected());
    return _is_connected.load();
}

void MqttCom::iterate() {
    if(!_client) throw std::runtime_error("Unitialized client sub.");
	if(!is_connected()) throw std::runtime_error("Client not connected.");
    mqtt::const_message_ptr msg;
    if (_client->try_consume_message_for(&msg, std::chrono::milliseconds{0})){
        const std::string& topic_name = msg->get_topic();
        if (_subscriptions_cb_map.find(topic_name)!=_subscriptions_cb_map.end()){
            for (auto&cb:_subscriptions_cb_map[topic_name]){
                if (cb)
                    cb(msg->to_string());
            }
        }
    }
}

void MqttCom::subscribe_to_topic(const std::string& topic_name, const SubscriptionCallback& cb) {
    switch (auto reason_code = subscribe_to(topic_name)){
    case mqtt::ReasonCode::SUCCESS:
        _subscriptions_cb_map[topic_name].push_back(cb);
        break;

    default:
        throw std::runtime_error("Unhandled mqtt reason code: "+std::to_string(reason_code));
    }
}

void MqttCom::unsubscribe_to_topic(const std::string& topic_name) {
    switch (auto reason_code = unsubscribe_to(topic_name)){
    case mqtt::ReasonCode::SUCCESS:
        if (_subscriptions_cb_map.find(topic_name)!=_subscriptions_cb_map.end()){
            _subscriptions_cb_map.erase(_subscriptions_cb_map.find(topic_name));
        }
        break;

    default:
        throw std::runtime_error("Unhandled mqtt reason code: "+std::to_string(reason_code));
    }
}

void MqttCom::publish_to_topic(
    const std::string& topic_name, const std::string& json_str) {
    switch (auto reason_code = publish_to(topic_name, json_str)){
    case mqtt::ReasonCode::SUCCESS:
        break;

    default:
        throw std::runtime_error("Unhandled mqtt reason code: "+std::to_string(reason_code));
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE MQTT FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

int MqttCom::connect_to_broker(const bool p_clean_session, const int p_keep_alive, const bool p_automatic_reconnect) {
    if(!_client) throw std::runtime_error("Unitialized client sub.");
    _m_connection_options.set_clean_session(p_clean_session);
    if (p_keep_alive>0) {
        _m_connection_options.set_keep_alive_interval(p_keep_alive);
    }
    if (p_automatic_reconnect) {
        _m_connection_options.set_automatic_reconnect(p_automatic_reconnect);
        _m_connection_options.set_automatic_reconnect(std::chrono::seconds(2), std::chrono::seconds(30));
    }

    
    try {
        const mqtt::token_ptr& l_token = _client->connect(_m_connection_options);
		l_token->wait_for(std::chrono::milliseconds{3000});
        _client->start_consuming();
        return l_token->get_reason_code();
    } catch (const mqtt::exception& p_exception) {
        throw std::runtime_error(p_exception.get_error_str());
    }
}

int MqttCom::disconnect_to_broker() {
    if(!_client) throw std::runtime_error("Unitialized client sub.");
    try {
        const auto& l_token = _client->disconnect(); l_token->wait();
        _client->stop_consuming();
        return l_token->get_reason_code();
    } catch (const mqtt::exception& p_exception) {
        throw std::runtime_error(p_exception.get_error_str());
    }
}

int MqttCom::subscribe_to(const std::string& p_topic, const int p_qos) {
    if(!_client) throw std::runtime_error("Unitialized client sub.");
	if(!is_connected()) throw std::runtime_error("Client not connected. Not subscribting topic "+p_topic);
    if(p_topic.empty()) throw std::runtime_error("Empty topic. Not subscribing.");
    try {
        const auto& l_token = _client->subscribe(p_topic, p_qos);
        return l_token->get_reason_code();
    } catch (const mqtt::exception& p_exception) {
        throw std::runtime_error(p_exception.get_error_str());
    }
}

int MqttCom::unsubscribe_to(const std::string& p_topic) {
    if(!_client) throw std::runtime_error("Unitialized client sub.");
	if(!is_connected()) throw std::runtime_error("Client not connected. Not unsubscribing topic "+p_topic);
    try {
        const auto& l_token = _client->unsubscribe(p_topic);
        return l_token->get_reason_code();
    } catch (const mqtt::exception& p_exception) {
        throw std::runtime_error(p_exception.get_error_str());
    }
}

int MqttCom::publish_to(const std::string& p_topic, const std::string& p_data, const int p_qos, const bool p_retain) {
    if(!_client) throw std::runtime_error("Unitialized client sub.");
	if(!is_connected()) throw std::runtime_error("Client not connected. Not unsubscribing.");
    //if(p_topic.empty()) throw std::runtime_error("Empty topic. Not publishing.");
    try{
        const auto& l_token = _client->publish(p_topic, p_data, p_qos, p_retain);
        return l_token->get_reason_code();
    } catch (const mqtt::exception& p_exception) {
        throw std::runtime_error(p_exception.get_error_str());
    }
}

}