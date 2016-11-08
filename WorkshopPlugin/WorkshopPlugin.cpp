#include "WorkshopPlugin.h"
#include "helpers.h"
#include <string>
#include <algorithm>
BAKKESMOD_PLUGIN(WorkshopPlugin, "Workshop plugin", "0.1", 0)

GameWrapper* gw;
ConsoleWrapper* cons;

void workshop_notifier(std::vector<std::string> params) 
{
	string command = params.at(0);
	if (command.compare("workshop_shot_load") == 0) {
		if (params.size() < 2)
			return;

		string shot_id = params.at(1);
		if (file_exists("./bakkesmod/shots/cache/" + shot_id + ".json"))
		{
			cons->executeCommand("shot_load \"cache/" + shot_id + "\"");
		}
		else 
		{
			cons->executeCommand("sendback requestshot \"" + shot_id + "\"");
		}
	}
	else if (command.compare("workshop_playlist_load") == 0) {

	}
	else if (command.compare("requestshot_ans") == 0) 
	{
		if (params.size() < 3)
			return;
		string shot_id = params.at(1);
		string shot_content = params.at(2);
		std::replace(shot_content.begin(), shot_content.end(), '|', '"');
		ofstream myfile;
		myfile.open("./bakkesmod/data/cache/" + shot_id + ".json");
		myfile << shot_content;
		myfile.close();
	}
}

void WorkshopPlugin::onLoad()
{

	gw = gameWrapper;
	cons = console;

	cons->registerNotifier("workshop_shot_load", workshop_notifier);
	cons->registerNotifier("workshop_playlist_load", workshop_notifier);
	cons->registerNotifier("workshop_playlist_next", workshop_notifier);
	cons->registerNotifier("workshop_playlist_prev", workshop_notifier);
	cons->registerNotifier("workshop_playlist_random", workshop_notifier);
	cons->registerNotifier("requestshot_ans", workshop_notifier);
}

void WorkshopPlugin::onUnload()
{
}