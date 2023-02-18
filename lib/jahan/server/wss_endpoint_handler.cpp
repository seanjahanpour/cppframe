// Author: Sean Jahanpour (sean@jahanpour.com)
//
#include <iostream>
#include <cctype>

#include "jahan/server/wss_endpoint_handler.h"

namespace jahan
{
	std::string time_t_to_string(const time_t& time, std::string format = std::string("%Y-%m-%d %H:%M:%S"));
}

namespace jahan::server
{
	WssEndpointHandler::WssEndpointHandler(WssServer::Endpoint &endpoint, const unsigned int channel_client_init = 10000, std::function<void(std::string message, Logger::LogLevel log_level)> logger) 
	:max_client_per_channel(channel_client_init), m_logger(logger), m_tpool(nullptr)
	{
		endpoint.on_message = [&](std::shared_ptr<WssServer::Connection> connection, std::shared_ptr<WssServer::InMessage> in_message) {
			this->on_message(connection, in_message);
		};

		endpoint.on_open = [&](std::shared_ptr<WssServer::Connection> connection) {
			this->on_open(connection);
		};

		endpoint.on_close = [&](std::shared_ptr<WssServer::Connection> connection, int status, const std::string &reason) {
			this->on_close(connection, status, reason);
		};

		endpoint.on_error = [&](std::shared_ptr<WssServer::Connection> connection, const SimpleWeb::error_code &ec) {
			this->on_error(connection, ec);
		};

		m_clients.reserve(10000); //10k channels
	}

	void WssEndpointHandler::on_message([[maybe_unused]] std::shared_ptr<WssServer::Connection> connection, [[maybe_unused]] std::shared_ptr<WssServer::InMessage> in_message)
	{
		auto h = [](WssEndpointHandler* weh,std::shared_ptr<WssServer::Connection> con,[[maybe_unused]] std::string in_message) {
			std::string client_key = con->remote_endpoint().address().to_string() + ":" + std::to_string(con->remote_endpoint().port());

			if(in_message.length() > 1000000) {
				// Over 1M message: iqnore it
				if(weh->m_logger) {
					weh->m_logger(client_key + ": Message over threashold.", Logger::Info);
				}
				return;
			}
			
			std::string channel = con->query_string;
			channel.erase(std::remove_if(channel.begin(), channel.end(), [](char const &c){
				return !std::isalnum(c);
			}),channel.end());

			if(channel.empty()) {
				return;
			}

			auto& this_client = weh->m_clients[channel][client_key];
			//if there is a authenticator set, we need to authenticate the clients
			if(weh->m_authenticator && (!this_client.authenticated)) {
				//need to authenticate the client, and this is the first time the unauthenticated user is sending a message.
				std::string name = weh->m_authenticator(in_message);
				if(name.empty()) {
					//failed authentication, kik out the client
					weh->m_client_mutex.lock();
					weh->m_clients[channel].erase(client_key);
					weh->m_client_mutex.unlock();

					con->send_close(3000, "Authentication Failed");
					if(weh->m_logger) {
						weh->m_logger(client_key + ": Failed Authentication.", Logger::Debug);
					}
					return;
				} else {
					//passed authentication. Mark client authenticated
					weh->m_client_mutex.lock();
					this_client.authenticated = true;
					this_client.name = name;
					weh->m_client_mutex.unlock();
					con->send("ok");
					if(weh->m_logger) {
						weh->m_logger(client_key + ": Authenticated", Logger::Debug);
					}
					weh->send_channel_presence(channel);
					return; //no need to broadcast authentication message
				}
			}

			if(this_client.authenticated && (this_client.name == "Sean J" || this_client.name == "Ash D" || this_client.name == "Alex D") && in_message == "CSTATUS") {
				con->send(weh->get_all_clients_in_json());
			} else {
				weh->broadcast("MESSAGE" + in_message, client_key, channel);
			}
		};

		if(m_tpool) {
			m_tpool->add_task(h, this, connection, in_message->string());
		} else {
			h(this, connection, in_message->string());
		}
	}

	void WssEndpointHandler::on_open([[maybe_unused]] std::shared_ptr<WssServer::Connection> connection)
	{
		auto h = [](WssEndpointHandler* weh,std::shared_ptr<WssServer::Connection> con) {

			//create unique client key
			std::string client_key = con->remote_endpoint().address().to_string() + ":" + std::to_string(con->remote_endpoint().port());


			//create alphanumeric string to use as channel name
			std::string channel = con->query_string;
			channel.erase(std::remove_if(channel.begin(), channel.end(), [](char const &c){
				return !std::isalnum(c);
			}),channel.end());

			if(channel.empty()) {
				if(weh->m_logger) {
					weh->m_logger(client_key + ": Empty Channel Name", Logger::Info);
				}
				con->send_close(1008, "Empty channel name is not allowed");
				return;
			}

			//add new client to list of clients under connected channel. Create channel if doesn't exists.
			weh->m_client_mutex.lock();
			try {
				if(weh->m_clients.find(channel) == weh->m_clients.end()) {
					auto &ch = weh->m_clients[channel];
					ch.reserve(weh->max_client_per_channel);
				}
				jahan::type::SocketClient &client = weh->m_clients[channel][client_key];
				client.authenticated = false;
				client.connection = con;
				client.connection_tstamp = std::time(nullptr);
			} catch(...) {
				if(weh->m_logger) {
					weh->m_logger(channel + " " + client_key + ": Failed to lock clients on open.", Logger::Debug);
				}

				//log failed new connection. When implementing this, move mutex unloack before any logging, and at the end of try block.
			}
			weh->m_client_mutex.unlock();
			if(weh->m_logger) {
				weh->m_logger(channel + " " + client_key + ": Connection opened?", Logger::Debug);
			}
		};

		//if threadpool exists, use thread pool. Otherwise just execute above function.	
		if(m_tpool) {
			m_tpool->add_task(h, this, connection);
		} else {
			h(this, connection);
		}
	}

	void WssEndpointHandler::on_close([[maybe_unused]] std::shared_ptr<WssServer::Connection> connection,[[maybe_unused]] int status, [[maybe_unused]] const std::string &reason)
	{
		//on close remove client from channel list of clients. Delete channel if no clients left.
		auto h = [](WssEndpointHandler* weh,std::shared_ptr<WssServer::Connection> con) {
			//create alphanumeric string to use as channel name
			std::string channel = con->query_string;
			channel.erase(std::remove_if(channel.begin(), channel.end(), [](char const &c){
				return !std::isalnum(c);
			}),channel.end());

			if(channel.empty()) {
				return;
			}

			//create unique client key using ip+port
			std::string client_key = con->remote_endpoint().address().to_string() + ":" + std::to_string(con->remote_endpoint().port());

			bool was_authenticated = weh->m_clients[channel][client_key].authenticated;
			std::string name = weh->m_clients[channel][client_key].name;

			//remove client from channel
			//erase channel is no clients left
			weh->remove_client(channel, client_key, "on close");

			if((weh->m_authenticator && was_authenticated) || (!weh->m_authenticator)) {
				weh->send_channel_presence(channel);
			}
		};

		//if threadpool exists, use thread pool. Otherwise just execute above function.		
		if(m_tpool) {
			m_tpool->add_task(h, this, connection);
		} else {
			h(this, connection);
		}
	}

	void WssEndpointHandler::on_error([[maybe_unused]] std::shared_ptr<WssServer::Connection> connection, [[maybe_unused]] const SimpleWeb::error_code &ec)
	{
		if(m_logger) {
			std::string message("Conn: ");
			message.append(connection->remote_endpoint().address().to_string()+ "-" + std::to_string(connection->remote_endpoint().port()));
			message.append(" Message: ");
			message.append(ec.message());
			m_logger(message , Logger::Error);
		}

		std::string channel = connection->query_string;
		channel.erase(std::remove_if(channel.begin(), channel.end(), [](char const &c){
			return !std::isalnum(c);
		}),channel.end());

		std::string client_key = connection->remote_endpoint().address().to_string() + ":" + std::to_string(connection->remote_endpoint().port());
		remove_client(channel, client_key, "Connection Error " + ec.message());
	}

	void WssEndpointHandler::set_thread_pool(std::shared_ptr<jahan::concurrency::ThreadPool> thread_pool)
	{
		m_tpool = thread_pool;
	}

	void WssEndpointHandler::set_authenticator(std::function<std::string(std::string first_message)> authenticator)
	{
		m_authenticator = authenticator;
	}

	void WssEndpointHandler::broadcast(const std::string message, const std::string client_key, std::string channel)
	{
		try {
			//broadcast message to all clients subscribed to the same channel, except the sender.
			for(auto& kv : m_clients[channel]) {
				if(kv.first != client_key) {
					if(m_authenticator) {
						//must be authenticated to receive messages
						if(!kv.second.authenticated) {
							continue;
						}
					}

					auto con_ptr = kv.second.connection.lock();	//weak_ptr to shared_ptr
					if(con_ptr) {
						if(m_logger) {
							m_logger(channel + " " + kv.first + ": Sending broadcast message", Logger::Debug);
						}
						con_ptr->send(message);
					} else {
						if(m_logger) {
							m_logger(channel + " " + kv.first + ": Couldn't lock.", Logger::Debug);
						}
						//Mean the client has gone away
						remove_client(channel, kv.first, "found inactive in broadcast.");
					}
				}
			}

			for(auto& kv : m_clients) {
				if(m_clients[kv.first].empty()) {
					m_client_mutex.lock();
					m_clients.erase(kv.first);
					m_client_mutex.unlock();
				}
			}
		} catch(...) {
			//log failed new connection. When implementing this, move mutex unloack before any logging, and at the end of try block.
		}
	}

	void WssEndpointHandler::remove_client(std::string channel, std::string client_key, std::string why)
	{
		if(m_logger) {
			m_logger(channel + " " + client_key + ": Removing Connection " + why, Logger::Debug);
		}

		try {
			m_client_mutex.lock();
			m_clients[channel].erase(client_key);
			if(m_clients[channel].empty()) {
				m_clients.erase(channel);
			}
			m_client_mutex.unlock();
		} catch(...) {
			if(m_logger) {
				m_logger(channel + " " + client_key + ": Tried to remove a client but not successful.", Logger::Error);
			}
		}
	}
	
	std::string WssEndpointHandler::get_all_clients_in_json()
	{
		std::string json("ALL    { ");

		for(const auto& [channel, value] : m_clients) {
			json.append("\"").append(channel).append("\":{ ");
			for(const auto& [client, wss_client] : value) {
				json.append("\"").append(client).append("\":{");

				json.append("\"Name\":\"").append(wss_client.name).append("\",");
				json.append("\"Authenticated\":\"").append(wss_client.authenticated ? "Yes" : "No").append("\",");
				json.append("\"Connection Timestamp\":\"").append(jahan::time_t_to_string(wss_client.connection_tstamp)).append("\"");

				json.append("},");
			}
			json.pop_back();
			json.append("},");
		}
		json.pop_back();

		json.append("}");

		return json;
	}

	void WssEndpointHandler::send_channel_presence(const std::string channel)
	{
		if(m_clients.find(channel) == m_clients.end()) {
			//channel doesn't exists.
			return;
		}

		for(const auto& [client, wss_client] : m_clients[channel]) {
			if(wss_client.authenticated) {
				std::string json("OTHERS { ");
				int i = 0;
				for(const auto& [c_key, wss_c] : m_clients[channel]) {
					if(wss_c.authenticated && c_key != client) {	//don't include the person receiving the presence json
						json.append("\"" + std::to_string(i++) + "\":").append(get_presence_json(wss_c)).append(",");
					}
				}
				json.pop_back();
				json.append("}");

				try{
					auto con_ptr = wss_client.connection.lock();	//weak_ptr to shared_ptr
					if(con_ptr) {
						con_ptr->send(json);
					} else {
						//Mean the client has gone away ?
						remove_client(channel, client, "found inactive in presence");
					}
				} catch (...) {

				}
			}
		}
	}

	std::string WssEndpointHandler::get_presence_json(const jahan::type::SocketClient& client)
	{
		std::string json("{");
		json.append("\"Name\":\"").append(client.name).append("\"");
		json.append("}");
		return json;
	}
}