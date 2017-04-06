#include "redirectplugin.h"
#include "helpers.h"

BAKKESMOD_PLUGIN(RedirectPlugin, "Redirect plugin", "0.2", PLUGINTYPE_FREEPLAY)



GameWrapper* gw;
ConsoleWrapper* cons;

void redirectplugin_ConsoleNotifier(std::vector<std::string> params) {
	string command = params.at(0);
	if (!gw->IsInTutorial())
		return;
	if (command.compare("redirect_shoot") == 0)
	{
		TutorialWrapper training = gw->GetGameEventAsTutorial();

		Vector playerPosition = training.GetGameCar().GetLocation();
		Vector ballPosition = training.GetBall().GetLocation();
		Vector playerVelocity = training.GetGameCar().GetVelocity();


		float ballSpeed = 1000;
		ballSpeed = cons->getCvarFloat("redirect_shot_speed", 1000);

		float offset = cons->getCvarFloat("redirect_pass_offset", 300);
		float offset_z = cons->getCvarFloat("redirect_pass_offset_z", 800);

		bool predict = cons->getCvarBool("redirect_pass_predict", true);
		float predictMultiplierX = cons->getCvarFloat("redirect_predict_multiplier_x", 2.0f);
		float predictMultiplierY = cons->getCvarFloat("redirect_predict_multiplier_y", 2.0f);
		bool onGround = cons->getCvarBool("redirect_on_ground", false);

		int offsetX = random(0, (int)offset);
		int offsetY = random(0, (int)offset);
		int offsetZ = random(offset_z / 3, offset_z);
		Vector offsetVec = Vector(offsetX, offsetY, offsetZ);

		Vector velMultiplied;
		if (predict)
			velMultiplied = playerVelocity * Vector(predictMultiplierX, predictMultiplierY, 1);
		offsetVec = offsetVec + velMultiplied;
		Vector shotData = training.GenerateShot(ballPosition, playerPosition + offsetVec, ballSpeed);
		if (onGround)
			shotData.Z = 0;
		training.GetBall().SetVelocity(shotData);
	}

}

void RedirectPlugin::onLoad()
{
	console->registerCvar("redirect_shot_speed", "(850, 1100)");

	console->registerCvar("redirect_pass_offset", "50");
	console->registerCvar("redirect_pass_offset_z", "200");
	console->registerCvar("redirect_pass_predict", "1");
	console->registerCvar("redirect_on_ground", "0");
	console->registerCvar("redirect_predict_multiplier_x", "2");
	console->registerCvar("redirect_predict_multiplier_y", "2");
	gw = gameWrapper;
	cons = console;
	console->registerNotifier("redirect_shoot", redirectplugin_ConsoleNotifier);
}

void RedirectPlugin::onUnload()
{
	console->unregisterNotifier("redirect_shoot", redirectplugin_ConsoleNotifier);
}