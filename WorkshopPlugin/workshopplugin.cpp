#include "workshopplugin.h"
#include "boost/log/trivial.hpp"
#include "helpers.h"
#include <map>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include <iostream>

typedef websocketpp::server<websocketpp::config::asio> server;

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

// pull out the type of messages sent by our config
typedef server::message_ptr message_ptr;
typedef server::connection_ptr connection_ptr;

using namespace boost::log::trivial;

struct connection_data
{
	bool authenticated = false;
	int tries = 0;
	std::vector<string> listeners;
};

BAKKESMOD_PLUGIN(WorkshopPlugin, "Workshop plugin", "0.1.1", GameEventType::Tutorial)

std::map<connection_ptr, connection_data> auths;
GameWrapper* gw;
Console* cons;

void workshopplugin_ConsoleNotifier(std::vector<std::string> params) 
{
	string command = params.at(0);
	for (auto it = auths.begin(); it != auths.end(); it++) {
		if (!it->second.authenticated) {
			continue;
		}

		if (it->first->get_state() != websocketpp::session::state::open) {
			auths.erase(it);
			continue;
		}

		for (auto listener = it->second.listeners.begin(); listener != it->second.listeners.end(); listener++) 
		{
			if (listener->compare(command) == 0) 
			{
				string str = command + " \"";
				for (size_t i = 1; i < params.size(); ++i)
				{
					str += params.at(i);
					if (i != params.size() - 1) {
						str += " ";
					}
				}
				str += "\"";
				break;
			}
		}
	}

}

bool is_authenticated(connection_ptr hdl) {
	if (auths.find(hdl) == auths.end()) {
		connection_data conData;
		conData.authenticated = false;

		auths.insert(std::pair<connection_ptr, connection_data>(hdl, conData));
	}
	return auths[hdl].authenticated;
}

void on_message(server* s, websocketpp::connection_hdl hdl, message_ptr msg) {
	std::cout << "on_message called with hdl: " << hdl.lock().get()
		<< " and message: " << msg->get_payload()
		<< std::endl;

	connection_ptr con = s->get_con_from_hdl(hdl);
	
	// check for a special command to instruct the server to stop listening so
	// it can be cleanly exited.
	/*if (msg->get_payload() == "stop-listening") {
		s->stop_listening();
		return;
	}*/
	
	try {
		auto input = parseConsoleInput(msg->get_payload());
		if (!is_authenticated(con))
		{
			if (input->size() > 0 && input->at(0).size() == 2 && input->at(0).at(0).compare("auth") == 0)
			{
				string authKey = cons->getCvarValue("auth", "");
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
		if (!gw->IsInTutorial()) {
			s->send(hdl, "notintraining", msg->get_opcode());
			return;
		}
		if (string_starts_with(msg->get_payload(), "listen|")) 
		{
			std::vector<string> listenersToAdd;
			if (split(msg->get_payload(), listenersToAdd, '|') > 1) 
			{
				for (auto it = listenersToAdd.begin() + 1; it != listenersToAdd.end(); it++) 
				{
					auths[con].listeners.push_back((*it));
					cons->registerNotifier((*it), workshopplugin_ConsoleNotifier);
					s->send(hdl, "Listen: " + (*it), msg->get_opcode());
				}
			}
		}
		else if (string_starts_with(msg->get_payload(), "shot_load2")) {
			auto cmd = parseConsoleInput(msg->get_payload());
			string shot_id = cmd->at(0).at(1);
			if (file_exists("./bakkesmod/shots/cache/" + shot_id + ".json")) 
			{
				cons->executeCommand("shot_load \"cache/" + shot_id + "\"");
				s->send(hdl, "shot_loaded", msg->get_opcode());
			}
			else {
				s->send(hdl, "requestshot " + shot_id, msg->get_opcode());
			}
		}
		else if (string_starts_with(msg->get_payload(), "file|")) {
			string newPayload = msg->get_payload().substr(5);
			int cutOff = newPayload.find("|");
			string fileName = newPayload.substr(0, cutOff);
			string fileContents = newPayload.substr(cutOff + 1);
			ofstream myfile;
			myfile.open("./bakkesmod/shots/cache/" + fileName + ".json");
			myfile << fileContents;
			myfile.close();
		}
		else {
			cons->executeCommand(msg->get_payload());
			s->send(hdl, "Executed " + msg->get_payload(), msg->get_opcode());
		}

		
		delete input;
	}
	catch (const websocketpp::lib::error_code& e) {
		std::cout << "Echo failed because: " << e
			<< "(" << e.message() << ")" << std::endl;
	}
}

server ws_server;

void run_server() {
	try {
		// Set logging settings


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


void WorkshopPlugin::onLoad()
{
	gw = gameWrapper;
	cons = console;

	cons->registerCvar("auth", "authkey");
	BOOST_LOG_TRIVIAL(info) << "Workshopplugin loaded";

	// Create a server endpoint
	//
	ws_server.set_access_channels(websocketpp::log::alevel::all);
	ws_server.clear_access_channels(websocketpp::log::alevel::frame_payload);

	// Initialize Asio
	ws_server.init_asio();

	// Register our message handler
	ws_server.set_message_handler(bind(&on_message, &ws_server, ::_1, ::_2));

	// Listen on port 9002
	ws_server.listen(9002);

	// Start the server accept loop
	ws_server.start_accept();
	ws_server.run();
	//thread t(&ws_server::run);
}

void WorkshopPlugin::onUnload()
{
	BOOST_LOG_TRIVIAL(info) << "Testplugin unloaded";
	ws_server.stop();
}
