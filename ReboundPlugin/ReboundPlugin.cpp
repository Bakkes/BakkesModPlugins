#include "ReboundPlugin.h"

#include "helpers.h"
#include <math.h>

BAKKESMOD_PLUGIN(ReboundPlugin, "Rebound plugin", "0.1", 0)

GameWrapper* gw;
ConsoleWrapper* cons;

void reboundplugin_ConsoleNotifier(std::vector<std::string> params) {
	string command = params.at(0);
	if (!gw->IsInTutorial())
		return;

	if (command.compare("rebound_shoot") == 0)
	{
		TutorialWrapper tw = gw->GetGameEventAsTutorial();

		Vector ballLoc = tw.GetBall().GetLocation();// ball->Location;

													//Calculate nearest goal
		CarWrapper player = tw.GetGameCar();
		Vector playerLocLater = tw.GetGameCar().GetLocation() + (tw.GetGameCar().GetVelocity() * 200); //Calculate where player is facing
		if (abs(player.GetVelocity().X) < 1 && abs(player.GetVelocity().X) < 1) {
			playerLocLater = ballLoc; //if player is not moving, set rebound to the goal the ball is closest to
		}
		Vector goal1Diff = tw.GetGoalLocation(0) - playerLocLater;
		Vector goal2Diff = tw.GetGoalLocation(1) - playerLocLater;
		float goal1DiffF = abs(goal1Diff.X) + abs(goal1Diff.Y) + abs(goal1Diff.Z);
		float goal2DiffF = abs(goal2Diff.X) + abs(goal2Diff.Y) + abs(goal2Diff.Z);

		int target = 0;
		if (goal1DiffF > goal2DiffF)
		{
			target = 1;
		}
		float sideOffset = cons->getCvarFloat("rebound_side_offset", 0.0f);

		Vector goalTarget = tw.GenerateGoalAimLocation(target, ballLoc);

		goalTarget.Z = tw.GetGoalExtent(target).Z + cons->getCvarFloat("rebound_addedheight", 300.0f);
		goalTarget.X += sideOffset;

		float reboundSpeed = cons->getCvarFloat("rebound_shotspeed", 600.0f);

		Vector shot = tw.GenerateShot(ballLoc, goalTarget, reboundSpeed);
		tw.GetBall().Stop(); //so theres no spin
		tw.GetBall().SetVelocity(shot);
	}

}

void ReboundPlugin::onLoad()
{
	console->registerCvar("rebound_shotspeed", "780");
	console->registerCvar("rebound_addedheight", "(300, 1400)");

	console->registerCvar("rebound_side_offset", "0");

	gw = gameWrapper;
	cons = console;
	console->registerNotifier("rebound_shoot", reboundplugin_ConsoleNotifier);
}

void ReboundPlugin::onUnload()
{
	console->unregisterNotifier("rebound_shoot", reboundplugin_ConsoleNotifier);
}
