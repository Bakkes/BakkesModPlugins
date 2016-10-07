#include "DefenderPlugin.h"

#include "helpers.h"
BAKKESMOD_PLUGIN(DefenderPlugin, "Defender plugin", "0.1", 0)

GameWrapper* gw;
ConsoleWrapper* cons;

bool defenderEnabled = false;
float lastTouchTime = 0;
int destinationGoal = 0;
float randomTime = random(5.0f, 9.0f);
int touchTimeout = 6; //6 checking iterations

void SetShot() {
	if (!gw->IsInTutorial() || !defenderEnabled)
		return;
	TutorialWrapper training = gw->GetGameEventAsTutorial();
	BallWrapper b = training.GetBall();

	int coolDown = cons->getCvarInt("defender_cooldown", 3000);

	touchTimeout = coolDown / 1000;
	float shotSpeed = cons->getCvarFloat("defender_shotspeed", 1100);

	Vector target = training.GenerateGoalAimLocation(destinationGoal, b.GetLocation());
	Vector shot = training.GenerateShot(b.GetLocation(), target, shotSpeed);
	b.Stop();
	b.SetVelocity(shot);
	randomTime = random(5.0f, 8.0f);
}

int checks = 0;
long long ExecuteNewShot() {
	if (!gw->IsInTutorial() || !defenderEnabled)
		return 500;
	TutorialWrapper training = gw->GetGameEventAsTutorial();
	BallWrapper b = training.GetBall();
	float touchTime = b.GetLastTouchTime();

	int coolDown = cons->getCvarInt("defender_cooldown", 3000);


	//if scored
	if (training.IsInGoal(b.GetLocation()))
	{
		b.Stop();
		b.SetLocation(Vector(random(-2000.0f, 2000.0f), random(400.0f, 400.0f), random(140.0f, 600.0f)));
		b.Stop();
		return coolDown;
	}
	if (training.IsBallMovingTowardsGoal(destinationGoal)) {
		return 750;
	}
	if (touchTime != lastTouchTime)
	{
		checks = -1;
		lastTouchTime = touchTime;
		return coolDown;
	}
	else if (checks >= touchTimeout || checks == -1) //-1 for after touch 
	{
		checks = 0;
		SetShot();
	}
	else {
		checks++;
	}
	return 750;

}

void go() {
	if (!gw->IsInTutorial() || !defenderEnabled)
		return;
	gw->SetTimeout([](GameWrapper* gameWrapper) {
		go();
	}, ExecuteNewShot());
}


void defenderplugin_ConsoleNotifier(std::vector<std::string> params) {
	string command = params.at(0);

	if (!gw->IsInTutorial())
		return;

	if (command.compare("defender_start") == 0)
	{
		defenderEnabled = true;
		cons->executeCommand("training_scoring 0");
		go();
	}
	else if (command.compare("defender_stop") == 0)
	{
		defenderEnabled = false;
	}

}

void DefenderPlugin::onLoad()
{
	gw = gameWrapper;
	cons = console;
	srand(time(NULL));
	console->registerNotifier("defender_start", defenderplugin_ConsoleNotifier);
	console->registerNotifier("defender_stop", defenderplugin_ConsoleNotifier);
	console->registerCvar("defender_shotspeed", "(800, 1100)");
	console->registerCvar("defender_cooldown", "(3000, 6000)"); //Cooldown in MS
}

void DefenderPlugin::onUnload()
{
}