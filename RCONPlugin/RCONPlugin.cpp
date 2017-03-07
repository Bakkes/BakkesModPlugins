#include "RCONPlugin.h"
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include "helpers.h"
#include <iostream>

typedef websocketpp::server<websocketpp::config::asio> server;

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

BAKKESMOD_PLUGIN(RCONPlugin, "RCON plugin", "0.1", -1337)

GameWrapper* gw;
ConsoleWrapper* cons;

// pull out the type of messages sent by our config
typedef server::message_ptr message_ptr;
typedef server::connection_ptr connection_ptr;

struct connection_data
{
	bool authenticated = false;
	int tries = 0;
	std::vector<string> listeners;
};
std::map<connection_ptr, connection_data> auths;
typedef std::map<connection_ptr, connection_data >::iterator auth_iter;

bool is_authenticated(connection_ptr hdl) {
	if (auths.find(hdl) == auths.end()) {
		connection_data conData;
		conData.authenticated = false;

		auths.insert(std::pair<connection_ptr, connection_data>(hdl, conData));
	}
	return auths[hdl].authenticated;
}

server ws_server;



string stringify(std::vector<std::string> params) 
{
	string str = "";
	for (size_t i = 1; i < params.size(); ++i)
	{
		str += params.at(i);
		if (i != params.size() - 1) {
			str += " ";
		}
	}
	str += "";
	return str;
}

void rconplugin_Notifier(std::vector<std::string> params) 
{
	string command = params.at(0);
	if (command.compare("sendback") == 0 && params.size() > 1)
	{
		for (auth_iter iterator = auths.begin(); iterator != auths.end(); iterator++)
		{
			if (is_authenticated(iterator->first) && iterator->first.get()->get_state() == websocketpp::session::state::open)
			{
				iterator->first->send(stringify(params));
			}
		}
	}
}

void on_message(server* s, websocketpp::connection_hdl hdl, message_ptr msg) {
	connection_ptr con = s->get_con_from_hdl(hdl);
	try {
		auto input = parseConsoleInput(msg->get_payload());
		if (!is_authenticated(con))
		{
			if (input->size() > 0 && input->at(0).size() == 2 && input->at(0).at(0).compare("rcon_password") == 0)
			{
				string authKey = cons->getCvarValue("rcon_password", "password");
				string userAuthKey = input->at(0).at(1);
				if (!authKey.empty() && !userAuthKey.empty())
				{
					if (authKey.compare(userAuthKey) == 0) {
						auths[con].authenticated = true;
						s->send(hdl, "authyes", msg->get_opcode());
						delete input;
						return;
					}
				}
			}
			auths[con].tries += 1;
			s->send(hdl, "authno", msg->get_opcode());
			delete input;
			return;
		}
		string payload = msg->get_payload();
		gw->Execute([payload](GameWrapper* gw) {
			cons->executeCommand(payload);
		});
		
		delete input;
	}
	catch (const websocketpp::lib::error_code& e) {
		std::cout << "Echo failed because: " << e
			<< "(" << e.message() << ")" << std::endl;
	}
}


void run_server() {
	try {
		// Set logging settings
		//ws_server.set_access_channels(websocketpp::log::alevel::all);
		//ws_server.clear_access_channels(websocketpp::log::alevel::frame_payload);

		// Initialize Asio
		ws_server.init_asio();

		// Register our message handler
		ws_server.set_message_handler(bind(&on_message, &ws_server, ::_1, ::_2));

		// Listen on port 9002
		string port = cons->getCvarValue("rcon_port", "9002");
		int p = is_number(port) ? atoi(port.c_str()) : 9002;
		ws_server.listen(p);

		// Start the server accept loop
		ws_server.start_accept();

		// Start the ASIO io_service run loop
		ws_server.run();
	}
	catch (websocketpp::exception const & e) {
		std::cout << e.what() << std::endl;
	}
	catch (...) {
		std::cout << "other exception" << std::endl;
	}
}

void RCONPlugin::onLoad()
{
	gw = gameWrapper;
	cons = console;
	cons->registerCvar("rcon_password", "password");
	cons->registerCvar("rcon_port", "9002"); //Registered in the main dll now
	cons->registerCvar("rcon_timeout", "5");
	cons->registerNotifier("sendback", rconplugin_Notifier);

	//thread t(run_server);
	run_server();
	//ws_server.run();
}

void RCONPlugin::onUnload()
{
	ws_server.stop();
	cons->unregisterNotifier("sendback", rconplugin_Notifier);
}
