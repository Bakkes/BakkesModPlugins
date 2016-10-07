#include "DribblePlugin.h"


BAKKESMOD_PLUGIN(DribblePlugin, "Dribble plugin", "0.1", 0)

GameWrapper* gw;
ConsoleWrapper* cons;

void dribblePlugin_onCommand(std::vector<std::string> params) {
	string command = params.at(0);
	if (!gw->IsInTutorial())
		return;

	if (command.compare("ballontop") == 0)
	{
		TutorialWrapper tutorial = gw->GetGameEventAsTutorial();
		BallWrapper ball = tutorial.GetBall();
		CarWrapper car = tutorial.GetGameCar();

		Vector playerVelocity = car.GetVelocity();
		Vector addToBall = Vector(playerVelocity.X, playerVelocity.Y, 170);

		addToBall.X = max(min(50.0f, addToBall.X), -50.0f);
		addToBall.Y = max(min(50.0f, addToBall.Y), -50.0f);

		ball.SetLocation(car.GetLocation() + addToBall);
		ball.SetVelocity(playerVelocity);
	}
}

void DribblePlugin::onLoad()
{
	gw = gameWrapper;
	cons = console;
	console->registerNotifier("ballontop", dribblePlugin_onCommand);
}

void DribblePlugin::onUnload()
{
}
