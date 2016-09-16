#include "trainingplugin.h"
#include "boost/log/trivial.hpp"
#include "cvareval.h"

using namespace boost::log::trivial;
JsonShot currentShot;
BAKKESMOD_PLUGIN(TrainingPlugin, "Training plugin", "0.1.1", GameEventType::Tutorial_Striker)

struct shot_data {
	float ball_location_x = 0;
	float ball_location_y = 0;
	float ball_location_z = 0;

	float ball_velocity_x = 0;
	float ball_velocity_y = 0;
	float ball_velocity_z = 0;

	float ball_torque_pitch = 0;
	float ball_torque_roll = 0;
	float ball_torque_yaw = 0;

	float player_location_x = 0;
	float player_location_y = 0;
	float player_location_z = 0;

	float player_velocity_x = 0;
	float player_velocity_y = 0;
	float player_velocity_z = 0;

	int player_rotation_pitch = 0;
	int player_rotation_yaw = 0;
	int player_rotation_roll = 0;

	Vector get_ball_start_location() {
		return Vector(ball_location_x, ball_location_y, ball_location_z);
	};

	Vector get_ball_start_velocity() {
		return Vector(ball_velocity_x, ball_velocity_y, ball_velocity_z);
	};

	Vector get_ball_start_rotation() {
		return Vector(ball_torque_pitch, ball_torque_roll, ball_torque_yaw);
	};

	Vector get_player_start_location() {
		return Vector(player_location_x, player_location_y, player_location_z);
	};

	Vector get_player_start_velocity() {
		return Vector(player_velocity_x, player_velocity_y, player_velocity_z);
	};

	Rotator get_player_start_rotation() {
		return Rotator(player_rotation_pitch, player_rotation_yaw, player_rotation_roll);
	};
};

struct shot_list {
	std::vector<shot_data> shots;
};


void get_shot_data_from_console(shot_data* data) {
	ball b;
	player p;
	speed s;
	
	data->ball_location_x = cons->getCvarFloat("shot_initial_ball_location_x", 0.0f);
	data->ball_location_y = cons->getCvarFloat("shot_initial_ball_location_y", 0.0f);
	data->ball_location_z = cons->getCvarFloat("shot_initial_ball_location_z", 0.0f);

	b.location.x = to_string(data->ball_location_x);
	b.location.y = to_string(data->ball_location_y);
	b.location.z = to_string(data->ball_location_z);
	data->ball_velocity_x = cons->getCvarFloat("shot_initial_ball_velocity_x", 0.0f);
	data->ball_velocity_y = cons->getCvarFloat("shot_initial_ball_velocity_y", 0.0f);
	data->ball_velocity_z = cons->getCvarFloat("shot_initial_ball_velocity_z", 0.0f);

	b.velocity.x = to_string(data->ball_velocity_x);
	b.velocity.y = to_string(data->ball_velocity_y);
	b.velocity.z = to_string(data->ball_velocity_z);

	data->ball_torque_pitch = cons->getCvarFloat("shot_initial_ball_torque_pitch", 0.0f);
	data->ball_torque_roll = cons->getCvarFloat("shot_initial_ball_torque_roll", 0.0f);
	data->ball_torque_yaw = cons->getCvarFloat("shot_initial_ball_torque_yaw", 0.0f);

	data->player_location_x = get_safe_float(parse(cons->getCvarValue("shot_initial_player_location_x", "0.0"), p, b, s));
	data->player_location_y = get_safe_float(parse(cons->getCvarValue("shot_initial_player_location_y", "0.0"), p, b, s));
	data->player_location_z = get_safe_float(parse(cons->getCvarValue("shot_initial_player_location_z", "0.0"), p, b, s));

	data->player_velocity_x = get_safe_float(parse(cons->getCvarValue("shot_initial_player_velocity_x", "0.0"), p, b, s));
	data->player_velocity_y = get_safe_float(parse(cons->getCvarValue("shot_initial_player_velocity_y", "0.0"), p, b, s));
	data->player_velocity_z = get_safe_float(parse(cons->getCvarValue("shot_initial_player_velocity_z", "0.0"), p, b, s));

	data->player_rotation_pitch = get_safe_int(parse(cons->getCvarValue("shot_initial_player_rotation_pitch", "0.0"), p, b, s));
	data->player_rotation_yaw = get_safe_int(parse(cons->getCvarValue("shot_initial_player_rotation_yaw", "0.0"), p, b, s));
	data->player_rotation_roll = get_safe_int(parse(cons->getCvarValue("shot_initial_player_rotation_roll", "0.0"), p, b, s));
}

Vector mirror_it(Vector v, bool mir) {
	if (mir) {
		v.X = -v.X;
		//v.Y = -v.Y;
	}
	return v;
}

Rotator mirror_it(Rotator r, bool mir) {
	if (mir) {
		if (r.Yaw > 0) {
			if (r.Yaw > 16383) {
				r.Yaw = 16383 - (r.Yaw - 16383);
			}
			else {
				r.Yaw = 16383 + (16383 - r.Yaw);
				
			}
		}
		else {
			if (r.Yaw > -16383) {
				r.Yaw = -16383 - (16383 - abs(r.Yaw));
			}
			else {
				r.Yaw = -16383 + (abs(r.Yaw) - 16383);
			}
		}

		//r.Yaw += (r.Yaw <= 0) ? 32766 : -32767;
	}
	return r;
}

bool should_mirror() {
	return cons->getCvarBool("shot_mirror", false);
}
shot_data s_data;
//Reload still doesn't fully work
//Create extra functions with #ADMIN flag?
void trainingplugin_ConsoleNotifier(std::vector<std::string> params) 
{
	string command = params.at(0);
	if (!gw->IsInTutorial()) 
	{
		return;
	}
	if (command.compare("shot_reset") == 0) 
	{
		
		get_shot_data_from_console(&s_data);
		TutorialWrapper tw = gw->GetGameEventAsTutorial();
		bool mirror = should_mirror();
		if (tw.IsTraining(GameEventType::Tutorial_Free_Play))
		{
			float waitTime = cons->getCvarFloat("shot_waitbeforeshot", 0.5f);
			
			BallWrapper ball = tw.GetBall();
			ball.Stop();
			ball.SetLocation(mirror_it(s_data.get_ball_start_location(), mirror));
			ball.Stop();
			CarWrapper car = tw.GetGameCar();
			car.Stop();
			car.SetLocation(mirror_it(s_data.get_player_start_location(), mirror));
			car.SetVelocity(mirror_it(s_data.get_player_start_velocity(), mirror));
			car.SetCarRotation(mirror_it(s_data.get_player_start_rotation(), mirror));
			if (!currentShot.getShot().options.script.empty()) {
				cons->executeCommand(currentShot.getShot().options.script);
			}
			gw->SetTimeout([&](GameWrapper* gw) {
				if (!gw->IsInTutorial())
					return;
				TutorialWrapper tw = gw->GetGameEventAsTutorial();

				if (tw.IsTraining(GameEventType::Tutorial_Free_Play))
				{
					
					bool mirror = should_mirror();
					BallWrapper ball = tw.GetBall();
					currentShot.setVelocity(gw, cons, mirror_it(ball.GetLocation(), mirror));
					get_shot_data_from_console(&s_data);
					ball.SetVelocity(mirror_it(s_data.get_ball_start_velocity(), mirror));
					ball.SetTorque(mirror_it(s_data.get_ball_start_rotation(), mirror));
				}
				

			}, max(0, waitTime * 1000));
		}
		else if (tw.IsTraining(GameEventType::Tutorial_Striker) || tw.IsTraining(GameEventType::Tutorial_Goalie) || tw.IsTraining(GameEventType::Tutorial_Aerial))
		{
			tw.SetCountdownTime(cons->getCvarFloat("shot_countdowntime", 1.0f));
			tw.SetBallSpawnLocation(mirror_it(s_data.get_ball_start_location(), mirror));
			tw.SetBallStartVelocity(mirror_it(s_data.get_ball_start_velocity(), mirror));
			tw.SetCarSpawnLocation(mirror_it(s_data.get_player_start_location(), mirror));
			tw.SetCarSpawnRotation(mirror_it(s_data.get_player_start_rotation(), mirror));
		}
	}
	else if (command.compare("shot_load") == 0)
	{
		if (params.size() == 1)
			return;
		string file = params.at(1);
		currentShot = JsonShot(file);
		currentShot.init(gw, cons);
	}
	else if (command.compare("shot_generate") == 0)
	{
		currentShot.set(gw, cons);
	}
}

void TrainingPlugin::onLoad()
{
	gw = gameWrapper;
	cons = console;
	cons->registerCvar("shot_mirror", "false");
	cons->registerCvar("shot_initial_ball_location_x", "0");
	cons->registerCvar("shot_initial_ball_location_y", "0");
	cons->registerCvar("shot_initial_ball_location_z", "0");

	cons->registerCvar("shot_initial_ball_velocity_x", "0");
	cons->registerCvar("shot_initial_ball_velocity_y", "0");
	cons->registerCvar("shot_initial_ball_velocity_z", "0");

	cons->registerCvar("shot_initial_ball_torque_pitch", "0");
	cons->registerCvar("shot_initial_ball_torque_roll", "0");
	cons->registerCvar("shot_initial_ball_torque_yaw", "0");

	cons->registerCvar("shot_initial_player_location_x", "0");
	cons->registerCvar("shot_initial_player_location_y", "0");
	cons->registerCvar("shot_initial_player_location_z", "0");

	cons->registerCvar("shot_initial_player_velocity_x", "0");
	cons->registerCvar("shot_initial_player_velocity_y", "0");
	cons->registerCvar("shot_initial_player_velocity_z", "0");

	cons->registerCvar("shot_initial_player_rotation_pitch", "0");
	cons->registerCvar("shot_initial_player_rotation_yaw", "0");
	cons->registerCvar("shot_initial_player_rotation_roll", "0");

	cons->registerCvar("shot_countdowntime", "1");
	cons->registerCvar("shot_waitbeforeshot", "0.5");
	
	cons->registerNotifier("shot_reset", trainingplugin_ConsoleNotifier);
	cons->registerNotifier("shot_load", trainingplugin_ConsoleNotifier);
	cons->registerNotifier("shot_generate", trainingplugin_ConsoleNotifier); 
	BOOST_LOG_TRIVIAL(info) << "Trainingplugin loaded";

	//BOOST_LOG_TRIVIAL(info) << "Engine name is" << gameWrapper->GetEngine()->GetFullName();
	//BOOST_LOG_TRIVIAL(info) << "Engine name (CPP) is" << gameWrapper->GetEngine()->GetNameCPP();
}

void TrainingPlugin::onUnload()
{
	BOOST_LOG_TRIVIAL(info) << "Trainingplugin unloaded";
}
