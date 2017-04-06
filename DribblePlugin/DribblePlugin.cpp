#include "DribblePlugin.h"


BAKKESMOD_PLUGIN(DribblePlugin, "Dribble plugin", "0.1", PLUGINTYPE_FREEPLAY)

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

		addToBall.X = max(min(20.0f, addToBall.X), -20.0f);//maybe limit the X a bit more
		addToBall.Y = max(min(30.0f, addToBall.Y), -30.0f);

		ball.SetLocation(car.GetLocation() + addToBall);
		ball.SetVelocity(playerVelocity);
	}
	else if (command.compare("shootatme") == 0)
	{
		TutorialWrapper tutorial = gw->GetGameEventAsTutorial();
		BallWrapper ball = tutorial.GetBall();
		CarWrapper car = tutorial.GetGameCar();
		Vector location = car.GetLocation();
		location = location + Vector(cons->getCvarFloat("shootatme_bounds_x"), cons->getCvarFloat("shootatme_bounds_y"), cons->getCvarFloat("shootatme_bounds_z"));
		Vector shot = tutorial.GenerateShot(ball.GetLocation(), location, cons->getCvarFloat("shootatme_speed"));
		ball.SetVelocity(shot);
	}
}

void DribblePlugin::onLoad()
{
	gw = gameWrapper;
	cons = console;
	console->registerCvar("shootatme_bounds_x", "(-200, 200)");
	console->registerCvar("shootatme_bounds_y", "(-200, 200)");
	console->registerCvar("shootatme_bounds_z", "(-400, 400)");
	console->registerCvar("shootatme_speed", "(800, 1000)");
	console->registerNotifier("ballontop", dribblePlugin_onCommand);
	console->registerNotifier("shootatme", dribblePlugin_onCommand);
}

void DribblePlugin::onUnload()
{
	console->unregisterNotifier("ballontop", dribblePlugin_onCommand);
	console->unregisterNotifier("shootatme", dribblePlugin_onCommand);
}
