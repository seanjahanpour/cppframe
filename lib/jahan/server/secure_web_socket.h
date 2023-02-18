// Author: Sean Jahanpour (sean@jahanpour.com)
//
#ifndef JAHAN_SERVER_SERVER_WSS_H_
#define JAHAN_SERVER_SERVER_WSS_H_

#include <string>
#include <memory>
#include <regex>
#include <functional>

#include "jahan/file/logger.h"

#include "simple_webSocket_server/server_wss.hpp"

namespace jahan::server 
{
	using WssServer = SimpleWeb::SocketServer<SimpleWeb::WSS>;
	using namespace jahan::file;
	
	class SecureWebSocket
	{
	public:
		SecureWebSocket();
		bool setup_server(const std::string cert_file_full_path, const std::string key_file_full_path, unsigned short port_number, std::function<void(std::string message, Logger::LogLevel log_level)> log_function = NULL) noexcept;
		WssServer::Endpoint& add_endpoint(const char* regex);
		bool start_server() noexcept;

	private:
		bool m_server_created = false;
		std::unique_ptr<WssServer> m_server;
		std::function<void(std::string message, Logger::LogLevel log_level)> m_logger;
	};
} //namespace jahan::server
#endif //JAHAN_SERVER_SERVER_WSS_H_