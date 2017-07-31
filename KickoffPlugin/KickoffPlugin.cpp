#include "KickoffPlugin.h"
#include <ctime>
#include "helpers.h"

BAKKESMOD_PLUGIN(KickoffPlugin, "Kickoff plugin", "0.1", PLUGINTYPE_CUSTOM_TRAINING)

struct frame {
	float timestamp;
	ControllerInput input;
};
struct recording {
	int version = 1;
	float length_seconds;
	int framecount;
	vector<frame>* frames;
};

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
bool record = false;
const float kickoff_length = 5.f; //Record 5 seconds of the kickoff
recording* rec;
recording* playback;
float startTime = 0.f;
float playback_startTime = 0.f;
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

frame linterp_frame(frame f1, frame f2, float elaps)
{
	float frameDiff = f2.timestamp - f2.timestamp;
	frame newFrame;
	newFrame.input.ActivateBoost = f2.input.ActivateBoost;
	newFrame.input.Handbrake = f2.input.Handbrake;
	newFrame.input.HoldingBoost = f2.input.HoldingBoost;
	newFrame.input.Jump = f2.input.Jump;
	newFrame.input.Jumped = f2.input.Jumped;
	newFrame.input.DodgeStrafe = f2.input.DodgeStrafe;
	newFrame.input.DodgeForward = f2.input.DodgeForward;
	newFrame.input.Pitch = f2.input.Pitch + ((f2.input.Pitch - f2.input.Pitch) * elaps / frameDiff);
	newFrame.input.Yaw = f2.input.Yaw + ((f2.input.Yaw - f2.input.Yaw) * elaps / frameDiff);
	newFrame.input.Roll = f2.input.Roll + ((f2.input.Roll - f2.input.Roll) * elaps / frameDiff);
	newFrame.input.Throttle = f2.input.Throttle + ((f2.input.Throttle - f2.input.Throttle) * elaps / frameDiff);
	newFrame.input.Steer = f2.input.Steer + ((f2.input.Steer - f2.input.Steer) * elaps / frameDiff);
	return newFrame;
}


string last_filename;
void stop_recording()
{
	if (cons->getCvarBool("record_autosave", true) && rec && rec->frames->size() > 0)
	{
		std::time_t result = std::time(0);
		last_filename = to_string(result) + ".kickoff";
		string path = "./bakkesmod/data/kickoffs/" + to_string(currentKickoff - 1) + "/" + last_filename;

		rec->framecount = rec->frames->size();
		rec->length_seconds = --(rec->frames->end())->timestamp;
		std::ofstream out(path.c_str(), ios::out | ios::trunc | ios::binary);
		write_pod(out, rec->version);
		write_pod(out, rec->length_seconds); //This doesn't get written/read correctly?
		write_pod(out, rec->framecount);
		for (unsigned int i = 0; i < rec->frames->size(); i++)
		{
			frame f = rec->frames->at(i);
			write_pod(out, f.timestamp);
			write_pod(out, f.input);
		}
		out.flush();
		out.close();
		delete rec->frames;
		delete rec;
		rec = NULL;
		record = false;
	}
}

void load_random_recording() {
	if (playback) {
		delete playback->frames;
		delete playback;
		playback = NULL;
	}


	srand(time(NULL));
	HANDLE hFind;
	WIN32_FIND_DATA data;
	vector<string> filenames;
	hFind = FindFirstFile(("./bakkesmod/data/kickoffs/" + to_string(currentKickoff - 1) + "/*.kickoff").c_str(), &data); //Load all kickoffs of this type
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			filenames.push_back(string(data.cFileName));
		} while (FindNextFile(hFind, &data));
		FindClose(hFind);
	}
	if (filenames.size() == 0)
	{
		cons->log("No kickoff files found for this kickoff!");
		return;
	}
	int randomFile = random(0, filenames.size() - 1);
	string filename = filenames.at(randomFile);
	cons->log("Loading file " + to_string(randomFile) + "/" + to_string(filenames.size()) + " - " + filename);
	string fullPath = "./bakkesmod/data/kickoffs/" + to_string(currentKickoff - 1) + "/" + filename;
	playback = new recording();
	playback->frames = new vector<frame>();
	std::ifstream in(fullPath.c_str(), ios::binary);
	read_pod(in, playback->version);
	read_pod(in, playback->length_seconds);
	read_pod(in, playback->framecount);
	for (unsigned int i = 0; i < playback->framecount; i++) {
		frame f;
		read_pod(in, f.timestamp);
		read_pod(in, f.input);
		playback->frames->push_back(f);
	}
	in.close();

	cons->log("File loaded, version " + to_string(playback->version) + ", frames:" + to_string(playback->framecount) + ", length: " + to_string(playback->length_seconds) + ".");
}

bool mirror = false;
int atFrame = 0;
void mirror_ai(ControllerInput* i , CarWrapper cw , BallWrapper bw, ServerWrapper sw) {
	auto ui = sw.GetUserInput();
	if (mirror) {
		memcpy(i, &ui, sizeof(ControllerInput));
	}
	else {
		if (playback != NULL && playback->frames != NULL && playback->framecount > 0) {
			if (playback_startTime < .1f) {
				playback_startTime = sw.GetSecondsElapsed();
				atFrame = 0;
			}
			float elapsedSinceStart = sw.GetSecondsElapsed() - playback_startTime;

			if (atFrame >= playback->frames->size())
				return;//Reached end of playback, stop playing back

			while (elapsedSinceStart > playback->frames->at(atFrame).timestamp) {
				atFrame++;
				if (atFrame >= playback->frames->size())
					return;//Reached end of playback, stop playing back
			}
			frame currentFrame = playback->frames->at(atFrame);
			frame nextFrame;
			if (atFrame + 1 == playback->frames->size()) //is last frame
			{
				nextFrame = currentFrame;
			}
			else {
				currentFrame = playback->frames->at(atFrame + 1);
			}
			frame usedFrame = linterp_frame(currentFrame, nextFrame, elapsedSinceStart - currentFrame.timestamp);
			memcpy(i, &usedFrame, sizeof(ControllerInput));
		}
	}
	if (!record)
		return;
	if (startTime < .1f) {
		startTime = sw.GetSecondsElapsed();
	}
	frame f;
	f.timestamp = sw.GetSecondsElapsed() - startTime;
	if (f.timestamp > kickoff_length)
	{
		stop_recording();
		return;
	}
	f.input = ui;
	//if (rec->frames->size() > 0 && fabs(f.timestamp - (--(rec->frames->end())->timestamp)) < .0001f) { //Game is paused? Will this method even get called if it is paused?
	//	return;
	//}
	rec->frames->push_back(f);

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

	if (!gw->IsInTutorial())
		return;

	if (command.compare("kickoff_load") == 0)
	{
		if (playback) {
			delete playback->frames;
			delete playback;
		}
		load_random_recording();
		//tutorial.Redo();
	}
	else if (command.compare("kickoff_test") == 0)
	{

		if (params.size() == 1) {
			return;
		}
		
		ServerWrapper sw = gw->GetGameEventAsServer();

		//cons->log("Players " + to_string(sw.GetPlayers().Count()));
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
		
		if (rec) {
			stop_recording();
		}
		mirror = cons->getCvarBool("kickoff_mirror", false) || playback == NULL || playback->frames == 0;

		startTime = .0f;
		playback_startTime = .0f;
		if (rec)
		{
			delete rec->frames;
			delete rec;
			rec = NULL;
		}
		rec = new recording();
		rec->frames = new vector<frame>();
		record = true;
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

		record = true;

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
	cons->registerCvar("kickoff_autosave", "1");
	cons->registerCvar("kickoff_mirror", "0");
}

void KickoffPlugin::onUnload()
{
	cons->unregisterNotifier("kickoff_load", kickoffplugin_ConsoleNotifier);
}