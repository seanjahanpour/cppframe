#include <iostream>
#include <string>
#include <future>
#include <thread>
#include <filesystem>

#include "jahan/helper.hpp"
#include "jahan/file/config.hpp"
#include "jahan/server/secure_web_socket.h"
#include "jahan/server/wss_endpoint_handler.h"
#include "jahan/file/logger.h"
#include "jahan/signal/manager.h"
#include "jahan/concurrency/thread_pool.hpp"
#include "jahan/db/core.hpp"
#include "jahan/db/mariadb.h"
#include "jahan/object_container.hpp"

#include "db/auth.hpp"


int main(int argc, char **argv)
{
	using namespace jahan;
	using namespace jahan::file;
	using namespace jahan::db;

	std::filesystem::current_path("/home/sean/sites/cpp/wss/build");

	Config::load("app.conf");

	//=====================================================================
	// setup logger and save logs every "delay" min on a dedicated thread
	//=====================================================================
	Logger::setup(Config::get("log_folder", "log"), std::stoi(Config::get("log_save_frequency_minutes","5")));
	std::thread logger_thread(Logger::thread_loop);

	//=====================================================================
	// signal handling on a dedicated thread
	//=====================================================================
	signal::Manager::setup_handler([](const int signal){
		Logger::save();
	}, Logger::log);
	std::thread sig_thread(signal::Manager::thread_loop);

	//=====================================================================
	// thread pool
	//=====================================================================
	std::shared_ptr<concurrency::ThreadPool> thread_pool = std::make_shared<concurrency::ThreadPool>(20);
	ObjectContainer<concurrency::ThreadPool>::set(thread_pool);
	//=====================================================================
	// database connection and sql handler
	//=====================================================================
	//database connection and query handler setup
	std::string databaseurl(Config::get("database_url"));
	std::string user(Config::get("database_user"));
	std::string password(Config::get("database_password"));
	std::unique_ptr<MariaDB> db_connector = std::make_unique<MariaDB>(databaseurl, user, password, 2, Logger::log);
	Core::set_db(std::move(db_connector));
	Core::set_logger(Logger::log);
	std::thread db_dog_thread(Core::db_dog);


	//=====================================================================
	// wss server
	//=====================================================================
	//setup setver and run
	std::shared_ptr<jahan::server::SecureWebSocket> socket_server;
	std::shared_ptr<std::thread> socket_server_thread;
	bool res;

	socket_server_thread = std::make_shared<std::thread>([&socket_server,&res, &thread_pool](){
		pthread_setname_np(pthread_self(), "frame server");
		socket_server = std::make_shared<jahan::server::SecureWebSocket>();

		unsigned short socket_port = std::stoi(Config::get("ws_port","3000"));
		res = socket_server->setup_server("certificate.bundle", "private.key", socket_port, Logger::log);

		if(res) {
			//create wild card endpoint
			server::WssServer::Endpoint& admin01 = socket_server->add_endpoint("^/admin01/?$");

			//assign a handler to the endpoint created above
			unsigned int init_clients_per_channel = std::stoi(Config::get("ws_init_clients_per_channel", "10000"));
			server::WssEndpointHandler admin01_handler(admin01, init_clients_per_channel, Logger::log);
			admin01_handler.set_thread_pool(thread_pool);
			admin01_handler.set_authenticator(app::db::Auth::from_first_wss_message);


			res = socket_server->start_server();
			if(!res) {
				Logger::log("Unable to start the server", Logger::Critical);
			}
		} else {
			Logger::log("Certificate files were rejected", Logger::Critical);
		}

		if(!res) {
			Logger::save();
			exit(1);
		}
		
	});

	socket_server_thread->join();
	logger_thread.join();
	sig_thread.join();
}