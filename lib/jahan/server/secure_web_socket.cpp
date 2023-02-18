// Author: Sean Jahanpour (sean@jahanpour.com)
//
#include <filesystem>

#include "jahan/server/secure_web_socket.h"

namespace jahan::server
{
	std::function<void(std::string message, Logger::LogLevel log_level)> logger;

	SecureWebSocket::SecureWebSocket() 
	{
	}

	bool SecureWebSocket::setup_server(const std::string cert_file, const std::string key_file, unsigned short port_number, std::function<void(std::string message, Logger::LogLevel log_level)> logger) noexcept
	{
		m_logger = logger;
		bool success;
		std::string log("Setting up WS server... ");

		success = true;

		try {
			std::string cert_file_full_path(std::filesystem::absolute(cert_file));
			std::string key_file_full_path(std::filesystem::absolute(key_file));
			m_server = std::make_unique<WssServer>(cert_file_full_path, key_file_full_path);
			m_server->config.port = port_number;
			m_server_created = true;
		} catch(const std::exception &exc) {
			m_server_created = false;
			success = false;
			log.append(exc.what());
		}

		if(logger) {
			if(success) {
				logger(log, Logger::Info);
			} else {
				logger(log, Logger::Critical);
			}
		}

		return success;
	}

	WssServer::Endpoint& SecureWebSocket::add_endpoint(const char* regex)
	{
		return m_server->endpoint[regex];
	}

	bool SecureWebSocket::start_server() noexcept
	{
		bool ret;
		ret = true;

		try {
			m_server->start();
		} catch(const std::exception &exc) {
			m_server_created = false;
			ret = false;
			if(m_logger) {
				m_logger(exc.what(), Logger::Critical);
			}
		}

		return ret;
	}
}