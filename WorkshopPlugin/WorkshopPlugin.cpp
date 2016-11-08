#include "WorkshopPlugin.h"
#include "helpers.h"
#include <string>
#include <algorithm>
BAKKESMOD_PLUGIN(WorkshopPlugin, "Workshop plugin", "0.1", 0)

GameWrapper* gw;
ConsoleWrapper* cons;

vector<string> shotList;
string load_after_request;
int currentIndex = 0;
void next_shot() 
{
	if (cons->getCvarBool("workshop_shot_random", true)) 
	{
		currentIndex = random(0, shotList.size() - 1);
	}
	else {
		currentIndex++;
		if (currentIndex >= shotList.size())
			currentIndex = 0;
	}
	cons->executeCommand("workshop_shot_load " + shotList.at(currentIndex));
}

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
			cons->executeCommand("shot_generate");
			cons->executeCommand("shot_reset");
		}
		else 
		{
			cons->executeCommand("sendback requestshot \"" + shot_id + "\"");
			load_after_request = shot_id;
		}
	}
	else if (command.compare("workshop_playlist_load") == 0) {
		if (params.size() < 2)
			return;

		string playlist_id = params.at(1);
		cons->executeCommand("sendback requestplaylist \"" + playlist_id + " \"");
		
	}
	else if (command.compare("requestshot_ans") == 0) 
	{
		if (params.size() < 3)
			return;
		string shot_id = params.at(1);
		string shot_content = params.at(2);
		std::replace(shot_content.begin(), shot_content.end(), '|', '"');
		ofstream myfile;
		myfile.open("./bakkesmod/shots/cache/" + shot_id + ".json");
		myfile << shot_content;
		myfile.close();
		if (!load_after_request.empty()) 
		{
			cons->executeCommand("workshop_shot_load " + load_after_request);
			load_after_request = "";
		}
	}
	else if (command.compare("requestplaylist_ans") == 0) 
	{
		if (params.size() < 3)
			return;
		string playlist_id = params.at(1);
		string shots = params.at(2);
		shotList.empty();

		if (shots.size() == 0)
			return;
		split(shots, shotList, ',');
		currentIndex = -1;
		next_shot();
	}
	else if (command.compare("workshop_playlist_next") == 0) 
	{
		if (!shotList.empty()) 
		{
			next_shot();
		}
	}
}

void WorkshopPlugin::onLoad()
{

	gw = gameWrapper;
	cons = console;

	cons->registerNotifier("workshop_shot_load", workshop_notifier);
	cons->registerNotifier("workshop_playlist_load", workshop_notifier);
	cons->registerNotifier("workshop_playlist_next", workshop_notifier);
	//cons->registerNotifier("workshop_playlist_prev", workshop_notifier);
	cons->registerCvar("workshop_playlist_random", "1");
	cons->registerNotifier("requestshot_ans", workshop_notifier);
	cons->registerNotifier("requestplaylist_ans", workshop_notifier);
}

void WorkshopPlugin::onUnload()
{
}