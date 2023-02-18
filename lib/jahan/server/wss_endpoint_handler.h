#ifndef JAHAN_SERVER_WSS_ENDPOINT_HANDLER_H_
#define JAHAN_SERVER_WSS_ENDPOINT_HANDLER_H_

#include <memory>
#include <string>

#include "jahan/server/socket_client.hpp"

#include "simple_webSocket_server/server_wss.hpp"
#include "jahan/concurrency/thread_pool.hpp"
#include "jahan/file/logger.h"

namespace jahan::server {
	using WssServer = SimpleWeb::SocketServer<SimpleWeb::WSS>;
	using namespace jahan::file;

	class WssEndpointHandler
	{
	public:
		WssEndpointHandler(WssServer::Endpoint &endpoint, const unsigned int channel_client_init, std::function<void(std::string message, Logger::LogLevel log_level)> logger = NULL);
		virtual void on_message(std::shared_ptr<WssServer::Connection> connection, std::shared_ptr<WssServer::InMessage> in_message);
		virtual void on_open(std::shared_ptr<WssServer::Connection> connection);
		virtual void on_close(std::shared_ptr<WssServer::Connection> connection, int status, const std::string &reason);
		virtual void on_error(std::shared_ptr<WssServer::Connection> connection, const SimpleWeb::error_code &ec);
		virtual void set_thread_pool(std::shared_ptr<jahan::concurrency::ThreadPool> thread_pool);
		virtual void set_authenticator(std::function<std::string(std::string first_message)> authenticator);
		unsigned int max_client_per_channel;
		
	protected:
		std::function<void(std::string message, Logger::LogLevel log_level)> m_logger;
		std::function<std::string(std::string first_message)> m_authenticator;
		std::unordered_map<std::string, std::unordered_map<std::string, jahan::type::SocketClient>> m_clients;
		std::mutex m_client_mutex;
		std::shared_ptr<jahan::concurrency::ThreadPool> m_tpool;
		
		std::string get_all_clients_in_json();
		virtual void broadcast(const std::string message, const std::string client_key, std::string channel);
		virtual void send_channel_presence(const std::string channel);
		virtual std::string get_presence_json(const jahan::type::SocketClient& client);
		virtual void remove_client(std::string channel, std::string client_key, std::string why);
	
	};
} //namespace jahans::server
#endif //JAHAN_SERVER_WSS_ENDPOINT_HANDLER_H_