#include "servertestplugin.h"
#include "./wrappers/carwrapper.h"
#include "./wrappers/teamwrapper.h"
#include "./wrappers/arraywrapper.h"
#include <iostream>
#include <fstream>
BAKKESMOD_PLUGIN(ServerTestPlugin, "ServerTestPlugin", "0.2", 0)

GameWrapper* gw;
ConsoleWrapper* cons;

void onEvent(ActorWrapper aw, string fname) 
{
	if (fname.find("Tick") != std::string::npos)
		return;
	ofstream myfile;
	myfile.open("ballevents.txt", std::ios_base::app);
	myfile << fname << "\r\n";
	myfile.close();
}

void ServerTestPlugin_ConsoleNotifier(std::vector<std::string> params) {
	string command = params.at(0);
	if (command.compare("test") == 0)
	{

		/*ServerWrapper sw = gw->GetGameEventAsServer();
		sw.SetTimeRemaining(sw.GetTimeRemaining() * 2);
		sw.GetTeam(1).SetScore(sw.GetTeam(1).GetScore() * 2);
		sw.SetPodiumTime(5000.0f);*/
		ServerWrapper sw = gw->GetGameEventAsServer();
		ArrayWrapper<TeamWrapper> teams = sw.GetTeams();

		TutorialWrapper tw = gw->GetGameEventAsTutorial();

		//Or you can do

		Rotator carRotation = tw.GetGameCar().GetRotation();
		carRotation.Pitch += 20000;
		tw.GetGameCar().SetRotation(carRotation);

		int t_count = teams.Count();
		for (int i = 0; i < t_count; i++) {
			TeamWrapper t = teams.Get(i);
			int d = 4;
			int score = t.GetScore();
			//teams.Get(i).SetScore(teams.Get(i).GetScore() + 2);
		}

		ArrayWrapper<CarWrapper> cars = sw.GetPlayers();
		int p_count = cars.Count();
		for (int i = 0; i < p_count; i++) {
			cars.Get(i).EnableGravity(false);
			cars.Get(i).SetRotation(cars.Get(i).GetRotation() + Rotator(0, 20000, 0));
		}
	}
	else if(command.compare("spawnball") == 0) {
		ServerWrapper sw = gw->GetGameEventAsServer();
		sw.SpawnBall(Vector(200, 200, 500), true);
	}
	else if (command.compare("removeball") == 0) {
		ServerWrapper sw = gw->GetGameEventAsServer();
		if (sw.GetBalls().Count() > 0)
			sw.RemoveBall(sw.GetBalls().Get(0));
	}
	else if (command.compare("freeze") == 0) {
		ServerWrapper sw = gw->GetGameEventAsServer();
		ArrayWrapper<CarWrapper> cars = sw.GetPlayers();
		int p_count = cars.Count();
		for (int i = 0; i < p_count; i++) {
			cars.Get(i).Freeze();
		}
	}
	else if (command.compare("resume") == 0) {
		ServerWrapper sw = gw->GetGameEventAsServer();
		ArrayWrapper<CarWrapper> cars = sw.GetPlayers();
		int p_count = cars.Count();
		for (int i = 0; i < p_count; i++) {
			cars.Get(i).Unfreeze();
		}
	}
	else if (command.compare("getdown") == 0) {
		ServerWrapper sw = gw->GetGameEventAsServer();
		ArrayWrapper<CarWrapper> cars = sw.GetPlayers();
		int p_count = cars.Count();
		for (int i = 0; i < p_count; i++) {
			sw.MoveToGround(cars.Get(i));
		}
	}
	else if (command.compare("balllisten") == 0) {
		TutorialWrapper tw = gw->GetGameEventAsTutorial();
		tw.ListenForEvents(onEvent);
	}
}



void ServerTestPlugin::onLoad()
{
	gw = gameWrapper;
	cons = console;


	console->registerNotifier("test", ServerTestPlugin_ConsoleNotifier);
	console->registerNotifier("spawnball", ServerTestPlugin_ConsoleNotifier);
	console->registerNotifier("removeball", ServerTestPlugin_ConsoleNotifier);

	console->registerNotifier("freeze", ServerTestPlugin_ConsoleNotifier);
	console->registerNotifier("resume", ServerTestPlugin_ConsoleNotifier);

	console->registerNotifier("getdown", ServerTestPlugin_ConsoleNotifier);
	console->registerNotifier("balllisten", ServerTestPlugin_ConsoleNotifier);
}


void ServerTestPlugin::onUnload()
{
	
}