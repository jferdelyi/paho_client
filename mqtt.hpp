#ifndef MQTT_H
#define MQTT_H

#include <mqtt/async_client.h>

/**
 * Callbacks for the success or failures of requested actions.
 * This could be used to initiate further action
 */
class ActionListener : public virtual mqtt::iaction_listener {

	public:

		/**
		 * Constructor
		 * @param p_name the name of the callback
		 */
		ActionListener() {}

	protected:

		/**
		 * This method is invoked when an action fails.
		 * @param p_token the token
		 */
		void on_failure(const mqtt::token& p_token) override {
			std::cout << " failure";
			if (p_token.get_message_id() != 0) {
				std::cout << " for token: [" << p_token.get_message_id() << "]" << std::endl;
			}
			std::cout << std::endl;
		}

		/**
		 * This method is invoked when an action has completed successfully.
		 * @param p_token the token
		 */
		void on_success(const mqtt::token& p_token) override {
			std::cout << " success";
			if (p_token.get_message_id() != 0) {
				std::cout << " for token: [" << p_token.get_message_id() << "]" << std::endl;
			}
			auto l_top = p_token.get_topics();
			if (l_top && !l_top->empty()) {
				std::cout << "\ttoken topic: '" << (*l_top)[0] << "', ..." << std::endl;
			}
			std::cout << std::endl;
		}
};

/**
 * Local callback & listener class for use with the client connection.
 * This is primarily intended to receive messages, but it will also monitor
 * the connection to the broker. If the connection is lost, it will attempt
 * to restore the connection and re-subscribe to the topic.
 */
class Callback : public virtual mqtt::callback, public virtual mqtt::iaction_listener {
	protected:

		/**
		 * The client
		 */
		mqtt::async_client& m_client;

		/**
		 * Options to use if we need to reconnect
		 */
		mqtt::connect_options& m_connecton_options;
	
	public:

		/**
		 * Constructor
		 * @param p_client the client
		 * @param p_connecton_options options to use if we need to reconnect
		 */
		Callback(mqtt::async_client& p_client, mqtt::connect_options& p_connecton_options) : 
			m_client(p_client), 
			m_connecton_options(p_connecton_options) {}


		/**
		 * Re-connection failure
		 * @param p_token the token
		 */
		void on_failure(const mqtt::token& p_token) override {
			std::cout << "Connection attempt failed" << std::endl;
		}

		/**
		 * (Re)connection success
		 * Either this or connected() can be used for callbacks.
		 * @param p_token the token
		 */
		void on_success(const mqtt::token& p_token) override {
			std::cout << "\tListener success for token: " << p_token.get_message_id() << std::endl;
		}

		/**
		 * (Re)connection success
		 * @param p_token the token
		 */		
		void connected(const std::string& p_cause) override {
			std::cout << "\nConnection success" << std::endl;
		}

		/**
		 * Callback for when the connection is lost.
		 * This will initiate the attempt to manually reconnect.
		 * @param p_cause the cause
		 */	
		void connection_lost(const std::string& p_cause) override {
			std::cout << "\nConnection lost" << std::endl;
			if (!p_cause.empty()) {
				std::cout << "\tcause: " << p_cause << std::endl;
			}
		}

		/**
		 * Callback for when a message arrives.
		 * @param p_message the message
		 */	
		void message_arrived(mqtt::const_message_ptr p_message) override {
			std::cout << "Message arrived" << std::endl;
			std::cout << "\ttopic: '" << p_message->get_topic() << "'" << std::endl;
			std::cout << "\tpayload: '" << p_message->to_string() << "'\n" << std::endl;
		}

		/**
		 * Called when delivery for a message has been completed, and all
		 * acknowledgments have been received.
		 * @param p_token the token
		 */	
		void delivery_complete(mqtt::delivery_token_ptr p_token) override {
			std::cout << "\tDelivery complete for token: " << (p_token ? p_token->get_message_id() : -1) << std::endl;
		}
};

/**
 * The MQTTClient wrapper
 */
class MQTTClient : public mqtt::async_client {
		protected:

			/**
			 * The server address.
			 */
			std::string m_server_address;

			/**
			 * An action listener to display the result of actions.
			 */
			ActionListener m_subscription_listener;

			/**
			 * Connection options.
			 */
			mqtt::connect_options m_connection_options;

			/**
			 * Callback.
			 */
			Callback* m_callback;

		public:

			/**
			 * Constructor
			 * @param p_id the client ID
			 * @param p_host the hostname or ip address of the broker to connect to
			 */
			MQTTClient (const std::string& p_id, const std::string& p_host = "tcp://localhost:1883") :
				mqtt::async_client(p_host, p_id),
				m_server_address(p_host),
				m_subscription_listener(),
				m_connection_options(),
				m_callback(nullptr) {
			}

			/**
			 * Destructor
			 */
			virtual ~MQTTClient() {
				delete m_callback;
			}
			
			/**
			 * Connect to the broker
			 * @param p_clean_session set to true to instruct the broker to clean all messages and subscriptions on disconnect, false to instruct it to keep them
			 * @param p_keep_alive keep alive time
			 * @return the reason code, if something wrong happen. 0 = OK (see https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901031)
			 */
			int connect_to(const bool p_clean_session = true, const int p_keep_alive = 60) {
				m_connection_options.set_clean_session(p_clean_session);
				m_connection_options.set_keep_alive_interval(p_keep_alive);
				m_callback = new Callback(*this, m_connection_options);

				set_callback(*m_callback);

				try {
					const auto& l_token = connect(m_connection_options, nullptr, *m_callback);
					return l_token->get_reason_code();
				} catch (const mqtt::exception& p_exception) {
					return p_exception.get_reason_code();
				}
			}

			/**
			 * Reconnect to the broker
			 * @return the reason code, if something wrong happen. 0 = OK (see https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901031)
			 */
			int reconnect_to() {
				const auto& l_token = reconnect();
				return l_token->get_reason_code();
			}

			/**
			 * Connect to the broker
			 * @return the reason code, if something wrong happen. 0 = OK (see https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901031)
			 */
			int disconnect_to() {
				const auto& l_token = disconnect();
				return l_token->get_reason_code();
			}

			/**
			 * Connect to the broker
			 * @param p_topic the name of the topic
			 * @param p_qos the QoS used
			 * @return the reason code, if something wrong happen. 0 = OK (see https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901031)
			 */
			int subscribe_to(const std::string& p_topic, const int p_qos = 1) {
				const auto& l_token = subscribe(p_topic, p_qos, nullptr, m_subscription_listener);
				return l_token->get_reason_code();
			}

			/**
			 * Connect to the broker
			 * @param p_topic the name of the topic
			 * @return the reason code, if something wrong happen. 0 = OK (see https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901031)
			 */
			int unsubscribe_to(const std::string& p_topic) {
				const auto& l_token = unsubscribe(p_topic);
				return l_token->get_reason_code();
			}

			/**
			 * Connect to the broker
			 * @param p_topic the name of the topic
			 * @param p_data data to send
			 * @param p_qos the QoS used
			 * @param p_retain if true the data is retained
			 * @return the reason code, if something wrong happen. 0 = OK (see https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901031)
			 */
			int publish_to(const std::string& p_topic, const std::string& p_data, const int p_qos = 1, const bool p_retain = false) {
				const auto& l_token = publish(p_topic, p_data, p_qos, p_retain);
				return l_token->get_reason_code();
			}
};

#endif //MQTT_H
