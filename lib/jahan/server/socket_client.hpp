// Author: Sean Jahanpour (sean@jahanpour.com)
//
#ifndef JAHAN_SERVER_SOCKET_CLIENT_H_
#define JAHAN_SERVER_SOCKET_CLIENT_H_

#include <memory>
#include <string>

#include "simple_webSocket_server/server_wss.hpp"

namespace jahan::type 
{
	struct SocketClient
	{	
		std::weak_ptr<SimpleWeb::SocketServer<SimpleWeb::WSS>::Connection> connection;
		std::time_t connection_tstamp;
		//std::time_t last_message_tstamp;
		//std::time_t last_activity_tstamp;
		bool	authenticated;
		std::string name;
	};
	
} //namespace jahan::type
#endif //JAHAN_SERVER_SOCKET_CLIENT_H_