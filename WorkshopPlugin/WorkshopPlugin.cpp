#include "WorkshopPlugin.h"
#include "helpers.h"
#include <string>
#include <algorithm>

#include <Windows.h>
#include <stdio.h>

BAKKESMOD_PLUGIN(WorkshopPlugin, "Workshop plugin", "0.1", PLUGINTYPE_FREEPLAY | PLUGINTYPE_CUSTOM_TRAINING | PLUGINTYPE_REPLAY)

static const string REPLAY_SHOT_DIRECTORY = "./bakkesmod/shots/replay/";

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

std::string getSafeFileName(std::string folder, std::string baseName) {
	int currentFile = 0;
	string fileName;
	do {
		fileName = baseName + "_" + to_string(currentFile) + ".json";
		currentFile++;
	} while (file_exists(folder + "//" + fileName));

	return fileName;
}

string createReplaySnapshot() {
	ReplayWrapper gew = gw->GetGameEventAsReplay();
	BallWrapper b = gew.GetBall();
	ActorWrapper aw = gew.GetViewTarget();
	//data needed: ball loc, ball vel, vt loc, vt rotation

	std::ifstream t("./bakkesmod/data/replay_template.json");
	std::string json_template((std::istreambuf_iterator<char>(t)),
		std::istreambuf_iterator<char>());

	replace(json_template, "{{player_loc_x}}", to_string(aw.GetLocation().X));
	replace(json_template, "{{player_loc_y}}", to_string(aw.GetLocation().Y));
	replace(json_template, "{{player_loc_z}}", to_string(aw.GetLocation().Z));

	replace(json_template, "{{player_rot_pitch}}", to_string(aw.GetRotation().Pitch));
	replace(json_template, "{{player_rot_roll}}", to_string(aw.GetRotation().Roll));
	replace(json_template, "{{player_rot_yaw}}", to_string(aw.GetRotation().Yaw));


	replace(json_template, "{{ball_loc_x}}", to_string(b.GetLocation().X));
	replace(json_template, "{{ball_loc_y}}", to_string(b.GetLocation().Y));
	replace(json_template, "{{ball_loc_z}}", to_string(b.GetLocation().Z));

	replace(json_template, "{{ball_vel_x}}", to_string(b.GetVelocity().X));
	replace(json_template, "{{ball_vel_y}}", to_string(b.GetVelocity().Y));
	replace(json_template, "{{ball_vel_z}}", to_string(b.GetVelocity().Z));



	string fileName = getSafeFileName(REPLAY_SHOT_DIRECTORY, "replay");
	//replace(json_template, "{{name}}", fileName);
	std::ofstream outputFile(REPLAY_SHOT_DIRECTORY + fileName);
	outputFile << json_template;
	outputFile.close();
	return fileName;
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
	else if (command.compare("replay_snapshot") == 0) 
	{
		if (!gw->IsInReplay()) {
			return;
		}
		createReplaySnapshot();
	}
	else if (command.compare("replay_snapshot_request") == 0) {
		if (!gw->IsInReplay()) {
			cons->executeCommand("sendback \"echo You need to be watching a replay to use this.\"");
			return;
		}
		string snapshotName = createReplaySnapshot();
		std::ifstream t(REPLAY_SHOT_DIRECTORY + snapshotName);
		std::string shotFile((std::istreambuf_iterator<char>(t)),
			std::istreambuf_iterator<char>());
		std::replace(shotFile.begin(), shotFile.end(), '"', '|');
		cons->executeCommand("sendback \"replay_snapshot_request_ans " + shotFile + "\"");
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
	cons->registerNotifier("replay_snapshot", workshop_notifier);
	cons->registerNotifier("replay_snapshot_request", workshop_notifier);
}

void WorkshopPlugin::onUnload()
{
}