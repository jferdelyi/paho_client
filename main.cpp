#include <iostream>
#include "mqtt.hpp"

#define PAHO_ERR_NOT_INIT -1000
#define PAHO_ERR_NOT_INITIALIZED_ONCE -1010
#define PAHO_ERR_ALREADY_CONNECTED -1011
#define PAHO_ERR_NOT_CONNECTED -1012

static MQTTClient* s_wrapper = nullptr;
static bool s_connection_initialized_once = false;

int initialise(const std::string& p_id, const std::string& p_host = "localhost", const int p_port = 1883) {
	try {
		std::string& m_connection_address = "tcp://" + p_host + ":" + std::to_string(p_port);
		s_wrapper = new MQTTClient(p_id, m_connection_address);
		return mqtt::ReasonCode::SUCCESS;
	} catch (std::exception& p_exception) {
		return atoi(p_exception.what());
	}
}

bool is_connected_to_broker() {
	if (s_wrapper) {
		return s_wrapper->is_connected();
	}
	return false;
}

int connect(const bool p_clean_session = true, const int p_keep_alive = 60) {
	if (!s_wrapper) {
		return PAHO_ERR_NOT_INIT;
	}
	if (s_wrapper->is_connected()) {
		return PAHO_ERR_ALREADY_CONNECTED;
	}
	s_connection_initialized_once = true;
	return s_wrapper->connect_to(p_clean_session, p_keep_alive);
}

int reconnect() {
	if (!s_wrapper) {
		return PAHO_ERR_NOT_INIT;
	}
	if (s_wrapper->is_connected()) {
		return PAHO_ERR_ALREADY_CONNECTED;
	}
	if (!s_connection_initialized_once) {
		return PAHO_ERR_NOT_INITIALIZED_ONCE;
	}
	return s_wrapper->reconnect_to();
}

int disconnect() {
	if (!s_wrapper) {
		return PAHO_ERR_NOT_INIT;
	}
	if (!s_wrapper->is_connected()) {
		return PAHO_ERR_NOT_CONNECTED;
	}
	return s_wrapper->disconnect_to();
}

int publish(const std::string& p_topic, const std::string& p_payload, const int p_qos = 1, const bool p_retain = false) {
	if (!s_wrapper) {
		return PAHO_ERR_NOT_INIT;
	}
	if (!s_wrapper->is_connected()) {
		return PAHO_ERR_NOT_CONNECTED;
	}
	return s_wrapper->publish_to(p_topic, p_payload, p_qos, p_retain);
}

int subscribe(const std::string& p_sub, const int p_qos = 1) {
	if (!s_wrapper) {
		return PAHO_ERR_NOT_INIT;
	}
	if (!s_wrapper->is_connected()) {
		return PAHO_ERR_NOT_CONNECTED;
	}
	return s_wrapper->subscribe_to(p_sub, p_qos);
}

int unsubscribe(const std::string& p_sub) {
	if (!s_wrapper) {
		return PAHO_ERR_NOT_INIT;
	}
	if (!s_wrapper->is_connected()) {
		return PAHO_ERR_NOT_CONNECTED;
	}
	return s_wrapper->unsubscribe_to(p_sub);
}

int clean() {
	const auto& rc_unsubscribe = unsubscribe("TopicA");
	if (rc_unsubscribe) {
		std::cout << "Unsubscribe error, RC(" << rc_unsubscribe << ")" << std::endl;
	}
	const auto& rc_disconnect = disconnect();
	if (rc_disconnect) {
		std::cout << "Disconnect error, RC(" << rc_disconnect << ")" << std::endl;
	}
	return rc_unsubscribe || rc_disconnect ? EXIT_FAILURE : EXIT_SUCCESS;
}

int main() {
	const auto& rc_init = initialise("Client");
	if (rc_init) {
		std::cout << "Initialization error, RC(" << rc_init << ")" << std::endl;
		return clean();
	}
	const auto& rc_connect = connect();
	if (rc_connect) {
		std::cout << "Connect error, RC(" << rc_connect << ")" << std::endl;
		return clean();
	}

	while (!is_connected_to_broker());

	const auto& rc_subscribe = subscribe("TopicA");
	if (rc_subscribe) {
		std::cout << "Subscribe error, RC(" << rc_subscribe << ")" << std::endl;
		return clean();
	}
	const auto& rc_publish = publish("TopicA", "Hello World");
	if (rc_publish) {
		std::cout << "Publish error, RC(" << rc_publish << ")" << std::endl;
		return clean();
	}
	
	while (std::tolower(std::cin.get()) != 'q');
	return clean();
}
