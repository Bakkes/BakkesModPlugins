#include "jsonshot.h"
#include <fstream>
#include "cvareval.h"
float get_safe_floats(string x) {
	 return 0;
}

string getLOL(json js, string thingy) {
	if (js.count(thingy) > 0) {
		if (js[thingy].is_string()) {
			return js[thingy].get<string>();
		}
		else if (js[thingy].is_number()) {
			return std::to_string(js[thingy].get<float>());
		}
	}
	return "0";
}

VectorString parseJSONVector(json js) {
	if (js.is_null())
		return VectorString("0", "0", "0");

	return VectorString(getLOL(js, "x"), getLOL(js, "y"), getLOL(js, "z"));
}

RotatorString parseJSONRotator(json js)
{
	if (js.is_null())
		return RotatorString("0", "0", "0");
	return RotatorString(getLOL(js, "pitch"), getLOL(js, "yaw"), getLOL(js, "roll"));
}

ball parseJSONBall(json js) {
	ball b;
	if (js.count("location") > 0) {
		b.location = parseJSONVector(js["location"]);
	}
	if (js.count("velocity") > 0) {
		b.velocity = parseJSONVector(js["velocity"]);
	}
	return b;
}

player parseJSONPlayer(json js) {
	player p;
	if (js.count("location") > 0) {
		p.location = parseJSONVector(js["location"]);
	}
	if (js.count("velocity") > 0) {
		p.velocity = parseJSONVector(js["velocity"]);
	}
	if (js.count("rotation") > 0) {
		p.rotation = parseJSONRotator(js["rotation"]);
	}
	return p;
}

void JsonShot::init(GameWrapper* gw, ConsoleWrapper* cons)
{
	json_shot currentshot;
	json child = the_json;
	if (child.count("options") > 0) {
		json opt = child["options"];
		if (opt.count("shootongoal") > 0) {
			int shoot_on_goal = opt["shootongoal"];
			currentshot.options.shoot_on_goal = shoot_on_goal;
			if ((shoot_on_goal == 1 || shoot_on_goal == 2) && gw->IsInTutorial()) {
				Vector location = gw->GetGameEventAsTutorial().GetGoalLocation(shoot_on_goal - 1);
				Vector extend = gw->GetGameEventAsTutorial().GetGoalExtent(shoot_on_goal - 1);
				VectorString vs;
				vs.x = "(" + to_string(location.X - extend.X / 2) + ", " + to_string(location.X + extend.X / 2) + ")";
				vs.y = "(" + to_string(location.Y - extend.Y / 2) + ", " + to_string(location.Y + extend.Y / 2) + ")";
				vs.x = "(" + to_string(location.Z - extend.Z / 2) + ", " + to_string(location.Z + extend.Z / 2) + ")";
				shot.end.locations.push_back(vs);
			}
		}
		if (opt.count("usevelocity") > 0) {
			currentshot.options.use_velocity = opt["usevelocity"];
		}
		if (opt.count("shootonground") > 0) {
			currentshot.options.shoot_on_ground = opt["shootonground"];
		}
		if (opt.count("script") > 0) {
			currentshot.options.script = opt["script"];
		}
	}

		
	if (child.count("start") > 0) {
		json start = child["start"];
		if (start.count("ball") > 0) {
			json ball = start["ball"];
			if (ball.is_array()) {
				for (auto& element : ball) {
					currentshot.start.balls.push_back(parseJSONBall(element));
				}
			}
			else {
				currentshot.start.balls.push_back(parseJSONBall(ball));
			}
		}
		if (start.count("player") > 0) {
			json player = start["player"];
			if (player.is_array()) {
				for (auto& element : player) {
					currentshot.start.players.push_back(parseJSONPlayer(element));
				}
			}
			else {
				currentshot.start.players.push_back(parseJSONPlayer(player));
			}
		}
	}

	if (child.count("end") > 0) {
		json end = child["end"];
		if (end.is_array()) {
			for (auto& element : end) {
				currentshot.end.locations.push_back(parseJSONVector(element));
			}
		}
		else {
			currentshot.end.locations.push_back(parseJSONVector(end));
		}
	}

	if (child.count("speed") > 0) {
		json speed = child["speed"];
		currentshot.speed.min = speed["min"];
		currentshot.speed.max = speed["max"];
	}
	
	shot = currentshot;
}

void JsonShot::setVelocity(GameWrapper* gw, ConsoleWrapper * cons, Vector ballLoc) {
	if (!shot.options.use_velocity) {
		if (gw->IsInTutorial()) {
			TutorialWrapper tw = gw->GetGameEventAsTutorial();
			VectorString dest = *select_randomly(shot.end.locations.begin(), shot.end.locations.end());
			Vector vecDest(get_safe_float(dest.x), get_safe_float(dest.y), get_safe_float(dest.z));
			Vector shotVec = tw.GenerateShot(ballLoc, vecDest, random(shot.speed.min, shot.speed.max));
			if (shot.options.shoot_on_ground) {
				shotVec.Z = 0;
			}
			cons->setCvar("shot_initial_ball_velocity_x", to_string(shotVec.X));
			cons->setCvar("shot_initial_ball_velocity_y", to_string(shotVec.Y));
			cons->setCvar("shot_initial_ball_velocity_z", to_string(shotVec.Z));
		}
	}
}

void JsonShot::set(GameWrapper* gw, ConsoleWrapper * cons)
{
	ball b = *select_randomly(shot.start.balls.begin(), shot.start.balls.end());
	player p = *select_randomly(shot.start.players.begin(), shot.start.players.end());
	speed s = shot.speed;
	
	cons->setCvar("shot_mirror", shot.options.disablemirror ? "0": rand() % 2 + 1 == 1 ? "1" : "0");
	cons->setCvar("shot_initial_ball_location_x", b.location.x);
	cons->setCvar("shot_initial_ball_location_y", b.location.y);
	cons->setCvar("shot_initial_ball_location_z", b.location.z);
	if (shot.options.use_velocity) {
		cons->setCvar("shot_initial_ball_velocity_x", b.velocity.x);
		cons->setCvar("shot_initial_ball_velocity_y", b.velocity.y);
		cons->setCvar("shot_initial_ball_velocity_z", b.velocity.z);
	}
	else {
		if (gw->IsInTutorial()) {
			TutorialWrapper tw = gw->GetGameEventAsTutorial();
			VectorString dest = *select_randomly(shot.end.locations.begin(), shot.end.locations.end());
			Vector vecDest(get_safe_float(dest.x), get_safe_float(dest.y), get_safe_float(dest.z));
			Vector shotVec = tw.GenerateShot(b.location.getVector(), vecDest, random(shot.speed.min, shot.speed.max));
			if (shot.options.shoot_on_ground) {
				shotVec.Z = 0;
			}
			cons->setCvar("shot_initial_ball_velocity_x", to_string(shotVec.X));
			cons->setCvar("shot_initial_ball_velocity_y", to_string(shotVec.Y));
			cons->setCvar("shot_initial_ball_velocity_z", to_string(shotVec.Z));
		}
	}
	cons->setCvar("shot_initial_player_location_x", p.location.x);
	cons->setCvar("shot_initial_player_location_y", p.location.y);
	cons->setCvar("shot_initial_player_location_z", p.location.z);

	cons->setCvar("shot_initial_player_velocity_x", p.velocity.x);
	cons->setCvar("shot_initial_player_velocity_y", p.velocity.x);
	cons->setCvar("shot_initial_player_velocity_z", p.velocity.x);

	cons->setCvar("shot_initial_player_rotation_pitch", p.rotation.pitch);
	cons->setCvar("shot_initial_player_rotation_yaw", p.rotation.yaw);
	cons->setCvar("shot_initial_player_rotation_roll", p.rotation.roll);
	/*cons->getCvarFloat("shot_initial_ball_location_x", 0.0f);
	cons->getCvarFloat("shot_initial_ball_location_y", 0.0f);
	cons->getCvarFloat("shot_initial_ball_location_z", 0.0f);

	cons->getCvarFloat("shot_initial_ball_velocity_x", 0.0f);
	cons->getCvarFloat("shot_initial_ball_velocity_y", 0.0f);
	data->ball_velocity_z = cons->getCvarFloat("shot_initial_ball_velocity_z", 0.0f);

	data->ball_torque_pitch = cons->getCvarFloat("shot_initial_ball_torque_pitch", 0.0f);
	data->ball_torque_roll = cons->getCvarFloat("shot_initial_ball_torque_roll", 0.0f);
	data->ball_torque_yaw = cons->getCvarFloat("shot_initial_ball_torque_yaw", 0.0f);

	data->player_location_x = cons->getCvarFloat("shot_initial_player_location_x", 0.0f);
	data->player_location_y = cons->getCvarFloat("shot_initial_player_location_y", 0.0f);
	data->player_location_z = cons->getCvarFloat("shot_initial_player_location_z", 0.0f);

	data->player_velocity_x = cons->getCvarFloat("shot_initial_player_velocity_x", 0.0f);
	data->player_velocity_y = cons->getCvarFloat("shot_initial_player_velocity_y", 0.0f);
	data->player_velocity_z = cons->getCvarFloat("shot_initial_player_velocity_z", 0.0f);

	data->player_rotation_pitch = cons->getCvarInt("shot_initial_player_rotation_pitch", 0);
	data->player_rotation_yaw = cons->getCvarInt("shot_initial_player_rotation_yaw", 0);
	data->player_rotation_roll = cons->getCvarInt("shot_initial_player_rotation_roll", 0);*/
}

json_shot JsonShot::getShot()
{
	return shot;
}




JsonShot::JsonShot()
{
}

JsonShot::JsonShot(string file) : shotfile(file)
{
	string newFile = "./bakkesmod/shots/" + file + ".json";
	if (file_exists(file)) {

	}
	else if (file_exists(newFile)) {
		file = newFile;
	}
	else {
		file = "./bakkesmod/data/emptyshot.json";
	}
	
	ifstream inFile;
	inFile.open(file);//open the input file

	stringstream strStream;
	strStream << inFile.rdbuf();//read the file
	string str = strStream.str();
	the_json = json::parse(str);
}


JsonShot::~JsonShot()
{
}
