#include "touchplugin.h"
#include "helpers.h"
#include <time.h>

BAKKESMOD_PLUGIN(TouchPlugin, "Touch plugin", "0.1.1", GameEventType::Tutorial_Free_Play)
bool touchEnabled = false;
void see();
void generateNewPosition();

GameWrapper* gw;
Console* cons;
float lastTouchTime = 0.0f;


//4077 + 4077,
//5976 + 5977,
//14 + 2027
void see() {
	if (!gw->IsInTutorial() || !touchEnabled) {
		touchEnabled = false;
		lastTouchTime = 0.0f;
		return;
	}

	float touchTime = gw->GetGameEventAsTutorial().GetBall().GetLastTouchTime();
	if (touchTime > lastTouchTime)
	{
		lastTouchTime = touchTime;
		generateNewPosition();
	}
	else
	{
		gw->SetTimeout([](GameWrapper* gameWrapper) {
			see();
		}, 1500);
	}
}

void generateNewPosition() {
	if (!gw->IsInTutorial() || !touchEnabled) {
		touchEnabled = false;
		lastTouchTime = 0.0f;
		return;
	}

	TutorialWrapper training = gw->GetGameEventAsTutorial();
	Vector newBallPosition(random(-3500, 3500), random(-4900, 4900), random(750, 1850));

	training.GetBall().Stop();
	training.GetBall().SetLocation(newBallPosition);

	gw->SetTimeout([](GameWrapper* gameWrapper) {
		see();
	}, 1500);
}

void redirectplugin_ConsoleNotifier(std::vector<std::string> params) {
	string command = params.at(0);
	if (!gw->IsInTutorial())
		return;
	if (command.compare("touch_start") == 0)
	{
		touchEnabled = true;
		generateNewPosition();
	}
	else if (command.compare("touch_stop") == 0)
	{
		touchEnabled = false;
	}

}

void TouchPlugin::onLoad()
{
	gw = gameWrapper;
	cons = console;
	srand(time(NULL));
	console->registerNotifier("touch_start", redirectplugin_ConsoleNotifier);
	console->registerNotifier("touch_stop", redirectplugin_ConsoleNotifier);
}

void TouchPlugin::onUnload()
{
}
