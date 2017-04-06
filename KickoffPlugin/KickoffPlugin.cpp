#include "KickoffPlugin.h"

#include "helpers.h"

BAKKESMOD_PLUGIN(KickoffPlugin, "Kickoff plugin", "0.1", PLUGINTYPE_THREADED | PLUGINTYPE_CUSTOM_TRAINING)

struct kickoff_data {
	Vector BallLocation;
	Vector PlayerSpawnLocation;
	Rotator PlayerRotation;
};

GameWrapper* gw;
ConsoleWrapper* cons;
int currentKickoff = 1;

void kickoffplugin_ConsoleNotifier(std::vector<std::string> params) {
	string command = params.at(0);

	if (!gw->IsInTutorial())
		return;

	if (command.compare("kickoff_load") == 0)
	{
		if (params.size() == 1) {
			return;
		}

		string kickoffId = params.at(1);

		if (kickoffId.compare("0") == 0) {
			if (currentKickoff > 6) {
				currentKickoff = 1;
			}
			else {
				currentKickoff++;
			}
		}
		else if (is_parsable(kickoffId) || is_number(kickoffId)) {
			int kickoffNumber = get_safe_int(kickoffId.c_str());
			if (kickoffNumber < 1 || kickoffNumber > 5) {
				return;
			}
			currentKickoff = kickoffNumber;
		}

		string kickoff_file = "./bakkesmod/data/kickoff_" + std::to_string(currentKickoff) + ".sp";
		kickoff_data kd = load_struct<kickoff_data>(kickoff_file);
		float countdownTime = cons->getCvarFloat("kickoff_countdown", 1.0f);
		TutorialWrapper tutorial = gw->GetGameEventAsTutorial();
		tutorial.SetBallSpawnLocation(kd.BallLocation);
		tutorial.SetBallStartVelocity(Vector(0.0f));
		tutorial.SetCarSpawnLocation(kd.PlayerSpawnLocation);
		tutorial.SetCarSpawnRotation(kd.PlayerRotation);
		tutorial.SetUnlimitedBoost(false);
		tutorial.SetCountdownTime(countdownTime);
		//tutorial.Redo();
	}
}

void KickoffPlugin::onLoad()
{
	gw = gameWrapper;
	cons = console;
	srand(time(NULL));

	cons->registerCvar("kickoff_countdown", "1.0");
	cons->registerNotifier("kickoff_load", kickoffplugin_ConsoleNotifier);
}

void KickoffPlugin::onUnload()
{
	cons->unregisterNotifier("kickoff_load", kickoffplugin_ConsoleNotifier);
}