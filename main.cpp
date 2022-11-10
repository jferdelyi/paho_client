#include "mqtt.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
	MQTTClient l_mqtt_client("MQTTClient");
	std::cout << l_mqtt_client.connect_to();

	while (!l_mqtt_client.is_connected());

	l_mqtt_client.subscribe_to("TopicA");
	l_mqtt_client.publish_to("TopicA", "Hello World");

	while (std::tolower(std::cin.get()) != 'q');

	l_mqtt_client.unsubscribe_to("TopicA");
	l_mqtt_client.disconnect_to_broker();
	return EXIT_SUCCESS;
}

