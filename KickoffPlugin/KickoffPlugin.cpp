#include "KickoffPlugin.h"

#include "helpers.h"

BAKKESMOD_PLUGIN(KickoffPlugin, "Kickoff plugin", "0.1", PLUGINTYPE_THREADED | PLUGINTYPE_CUSTOM_TRAINING)

struct kickoff {
	Vector PlayerSpawnLocation;
	Rotator PlayerRotation;
};

const kickoff ball_kickoff = { {0, 0, 92.775124f}, {0, 0, 0} };

const kickoff kickoffs[2][5] = {
	{
		{ { 0.f, -4607.985840f, 22.f },{ -72, 16376, 0 } },
		{ { -256.001984f, -3839.990967f, 22.f },{ -16, 16388, 0 } },
		{ { 255.999039f, -3839.990723f, 22.f },{ -44, 16376, 0 } },
		{ { -1951.988892f, -2463.979736f, 22.f },{ -44, 8184, 0 } },
		{ { 1951.995117f, -2463.991699f, 22.f },{ -72, 24568, 0 } }
	},
	{
		{ { 0.f, 4607.985840f, 22.f },{ -16, -16384, 0 } },
		{ { 257.474640f, 3839.892578f, 22.f },{ -56, -16388, 0 } },
		{ { -254.526306f, 3840.064453f, 22.f },{ -88, -16400, 0 } },
		{ { 1952.945435f, 2463.242676f, 22.f },{ -56, -24588, 0 } },
		{ { -1951.039185f, 2464.729980f, 22.f },{ -44, -8204, 0 } }
	}
};


GameWrapper* gw;
ConsoleWrapper* cons;
int currentKickoff = 1;

//struct ControllerInput {
//	float Throttle = 0.0f;
//	float Steer = 0.0f;
//	float Pitch = 0.0f;
//	float Yaw = 0.0f;
//	float Roll = 0.0f;
//	bool Jump = false;
//	bool Jumped = false;
//	bool ActivateBoost = false;
//	bool HoldingBoost = false;
//	bool Handbrake = false;
//};

void mirror_ai(ControllerInput* i , CarWrapper cw , BallWrapper bw, ServerWrapper sw) {
	auto ui = sw.GetUserInput();
	memcpy(i, &ui, sizeof(ControllerInput));
	//i->Pitch = -i->Pitch;
	//i->Roll = -i->Roll;
	//i->Steer = -i->Steer;
	//i->Yaw = -i->Yaw;
}


void init_kickoff(CarWrapper cw, int teamNo, int kickoffNo)
{
	kickoff usedKickoff = kickoffs[teamNo][kickoffNo];
	cw.SetLocation(usedKickoff.PlayerSpawnLocation);
	cw.SetRotation(usedKickoff.PlayerRotation);
}

void kickoffplugin_ConsoleNotifier(std::vector<std::string> params) {
	string command = params.at(0);

	/*if (!gw->IsInTutorial())
		return;*/

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
		kickoff kd = kickoffs[0][currentKickoff - 1];
		float countdownTime = cons->getCvarFloat("kickoff_countdown", 1.0f);
		TutorialWrapper tutorial = gw->GetGameEventAsTutorial();
		tutorial.SetBallSpawnLocation(ball_kickoff.PlayerSpawnLocation);
		tutorial.SetBallStartVelocity(Vector(0.0f));
		tutorial.SetCarSpawnLocation(kd.PlayerSpawnLocation);
		tutorial.SetCarSpawnRotation(kd.PlayerRotation);
		tutorial.SetIsUnlimitedBoost(false);
		tutorial.SetCountdownTime(countdownTime);
		//tutorial.Redo();
	}
	else if (command.compare("kickoff_test") == 0)
	{
		
		if (params.size() == 1) {
			return;
		}
		ServerWrapper sw = gw->GetGameEventAsServer();
		
		cons->log("Players " + to_string(sw.GetPlayers().Count()));
		if (sw.GetPlayers().Count() < 2) {
			sw.SpawnBotWithAI(&mirror_ai);
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

		//gw->GetGameEventAsTutorial().SetSecondsRemainingCountdown(3);// 
		sw.SetCountdownTime(3);
		sw.SetGameStateTime(3);
		sw.ResetBalls();
		sw.SetAllDriving(false);
		sw.StartCountdown();
		//auto sw = gw->GetGameEventAsServer();


		auto ourCar = sw.GetPRICar(0);
		auto botCar = sw.GetPRICar(1);
		if (ourCar.IsNull() || botCar.IsNull()) {
			cons->log("Something went wrong when creating a bot car");
			return;
		}
		ourCar.SetIsDriving(false);
		botCar.Stop();
		ourCar.Stop();
		gw->SetTimeout([](GameWrapper* gw) {
			auto sw = gw->GetGameEventAsServer();
			auto ourCar = sw.GetPRICar(0);
			auto botCar = sw.GetPRICar(1);
			botCar.Stop();
			ourCar.Stop();
		}, 10);

		init_kickoff(ourCar, 0, currentKickoff - 1);
		init_kickoff(botCar, 1, currentKickoff - 1);

		gw->SetTimeout([](GameWrapper* gw) {
			auto sw = gw->GetGameEventAsServer();
			auto ourCar = sw.GetPRICar(0);
			auto botCar = sw.GetPRICar(1);
			float defaultBoost = ourCar.GetBoost().GetStartBoostAmount();
			ourCar.GetBoost().SetBoostAmount(defaultBoost);
			if(!botCar.IsNull() && !botCar.GetBoost().IsNull())
				botCar.GetBoost().SetBoostAmount(defaultBoost);

			sw.SetCountdownTime(3);
			sw.SetGameStateTime(3);
			sw.ResetBalls();
			sw.SetAllDriving(false);
			sw.StartCountdown();
		}, 100);
	}
}


void KickoffPlugin::onLoad()
{
	gw = gameWrapper;
	cons = console;
	srand(time(NULL));

	cons->registerCvar("kickoff_countdown", "1.0");
	cons->registerNotifier("kickoff_load", kickoffplugin_ConsoleNotifier);
	cons->registerNotifier("kickoff_test", kickoffplugin_ConsoleNotifier);
}

void KickoffPlugin::onUnload()
{
	cons->unregisterNotifier("kickoff_load", kickoffplugin_ConsoleNotifier);
}