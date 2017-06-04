#include "ReboundPlugin.h"

#include "helpers.h"
#include <math.h>

BAKKESMOD_PLUGIN(ReboundPlugin, "Rebound plugin", "0.1", PLUGINTYPE_FREEPLAY)

GameWrapper* gw;
ConsoleWrapper* cons;
Vector projectedBounce;

void on_draw(CanvasWrapper cw) {
	//auto ball = gw->GetGameEventAsTutorial().GetBall();
	//Vector ballLoc = ball.GetLocation();
	//Vector2 loc = cw.Project(ballLoc - Vector(50, 0, 50));

	//Vector2 loc2 = cw.Project(ballLoc + Vector(50, 0, 50));

	//Vector2 topLeft = loc.X > loc2.X ? loc : loc2;

	//cw.SetPosition(loc);
	//
	//cw.SetColor(255, 0, 0, 122);

	//cw.DrawRect(loc, loc2);
	//cw.FillBox({ loc2.X - loc.X, loc2.Y - loc.Y });
	//cw.SetColor(122, 255, 0, 180);

	//topLeft.X -= 30;
	//cw.SetPosition(topLeft);

	//cw.DrawString(to_string(loc.X) + "," + to_string(loc.Y));


	//cw.SetPosition({ topLeft.X, topLeft.Y + 20 });
	//cw.DrawString(to_string(ballLoc.X) + "," + to_string(ballLoc.Y) + "," + to_string(ballLoc.Z));
}



void repeat_rebound() {
	cons->log(to_string(cons->getCvarBool("rebound_repeat")));
	cons->log(to_string(cons->getCvarFloat("rebound_delay", 10000)));
	if (cons->getCvarBool("rebound_repeat")) {
		if (gw->IsInTutorial()) {
			cons->executeCommandNoLog("rebound_shoot");
		}
		gw->SetTimeout([](GameWrapper* gw) {
			repeat_rebound();
		}, cons->getCvarFloat("rebound_delay", 10000));
	}
}

void reboundplugin_ConsoleNotifier(std::vector<std::string> params) {
	string command = params.at(0);

	if (command.compare("rebound_repeat") == 0 && params.size() > 1)
	{
		bool shouldRepeat = !(params.at(1).compare("0") == 0);
		if (shouldRepeat) {
			gw->SetTimeout([](GameWrapper* gw) {
				repeat_rebound();
			}, 100); //execute after 100 ms so rebound_repeat is set
		}
	}

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
		projectedBounce = goalTarget;
		Vector shot = tw.GenerateShot(ballLoc, goalTarget, reboundSpeed);
		if (cons->getCvarBool("rebound_resetspin", false)) {
			tw.GetBall().Stop();
		}
		Vector addedSpin = { cons->getCvarFloat("rebound_addedspin", 0.0f),
			cons->getCvarFloat("rebound_addedspin", 0.0f),
			cons->getCvarFloat("rebound_addedspin", 0.0f) };
		//tw.GetBall().Stop(); //so theres no spin, nevermind, we want some spin
		tw.GetBall().SetVelocity(shot);
		tw.GetBall().SetAngularVelocity(addedSpin, true);
	}
	

}

void ReboundPlugin::onLoad()
{
	console->registerCvar("rebound_shotspeed", "780");
	console->registerCvar("rebound_addedheight", "(300, 1400)");

	console->registerCvar("rebound_side_offset", "0");
	console->registerCvar("rebound_addedspin", "0");
	console->registerCvar("rebound_resetspin", "0");

	console->registerCvar("rebound_delay", "(7000, 15000)");
	console->registerCvar("rebound_repeat", "0");

	
	gw = gameWrapper;
	cons = console;

	console->registerNotifier("rebound_shoot", reboundplugin_ConsoleNotifier);
	console->registerNotifier("rebound_repeat", reboundplugin_ConsoleNotifier);
	//gw->RegisterDrawable(on_draw);
}

void ReboundPlugin::onUnload()
{
	console->unregisterNotifier("rebound_shoot", reboundplugin_ConsoleNotifier);
	console->unregisterNotifier("rebound_repeat", reboundplugin_ConsoleNotifier);
}
