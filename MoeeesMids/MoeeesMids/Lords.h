#pragma once
#include "PluginSDK.h"
#include "Extensions.h"

static std::unordered_map<int, float>Timers;
static std::vector<Vec3>path_;
void OnRunPath (IUnit* Source, const std::vector<Vec3>& path_)
{
	int id = Source->GetNetworkId();
	Timers[id] = GGame->Time();
	if (Source->IsHero() && Source->IsEnemy (GEntityList->Player()) || Source == GEntityList->Player())
	{
		/*
		if (PA[id] [#PA[unit.networkID] - 1]) {
			auto p1 = PA[unit.networkID][#PA[unit.networkID] - 1].p
			          auto p2 = PA[unit.networkID][#PA[unit.networkID]].p
			                    auto angle = Vec3 (unit.x, unit.y, unit.z) :angleBetween (Vec3 (p2.x, p2.y, p2.z), Vec3 (p1.x, p1.y, p1.z))
				                                 if angle > 20
		{
			auto submit = { t = os.clock(), p = endPos }
				table.insert (PA[unit.networkID], submit)
			}
			else
				auto submit = { t = os.clock(), p = endPos }
				              table.insert (PA[unit.networkID], submit)
			}*/
	}
}



static float lastQ;

float Distances (Vec2 from, Vec2 to)
{
	return (from - to).Length();
}

float Distances (Vec3 from, Vec3 to)
{
	return (from - to).Length2D();
}

static bool QCast (ISpell2* spell, IUnit* target)
{
	if (!target->IsValidTarget())
	{ return false; }
	int eId = target->GetNetworkId();
	Vec2 targetpos = target->GetPosition().To2D();
	float targetspeed = target->MovementSpeed();
	path_ = target->GetWaypointList();
	int length = path_.size() /*GetAvgPathLenght(target)*/;
	Vec3 predpos = Vec3 (0, 0, 0);
	if (length > 1)
	{
		float zedtime = targetspeed * (GGame->Time() - (Timers[eId]) + ( (GGame->Latency() / 2) * 0.001f));
		float dis = 0.0f;
		for (int i = 0; i < length - 1; i++)
		{
			Vec2 path1 = path_[i].To2D();
			Vec2 path2 = path_[i + 1].To2D();
			dis += Distances (path1, path2);
			if (dis >= zedtime)
			{
				float tarpath = Distances (targetpos, path2);
				float tarspeed = targetspeed * 0.5f;
				if (tarpath >= tarspeed)
				{
					predpos = Extensions::To3D ( (targetpos + ( (path2 - targetpos).VectorNormalize() * tarspeed)));
					break;
				}
				if (i + 1 == length - 1)
				{
					predpos = Extensions::To3D ( (targetpos + ( (path2 - targetpos).VectorNormalize() * tarpath)));
					break;
				}
				for (int j = i + 1; j < length - 1; j++)
				{
					Vec2 vec2path = path_[j].To2D();
					Vec2 vec2path2 = path_[j + 1].To2D();
					tarspeed -= tarpath;
					tarpath = Distances (vec2path, vec2path2);
					if (tarpath >= tarspeed)
					{
						predpos = Extensions::To3D ( (vec2path + ( (vec2path2 - vec2path).VectorNormalize() * tarspeed)));
						break;
					}
					if (j + 1 == length - 1)
					{
						predpos = Extensions::To3D ( (vec2path + ( (vec2path2 - vec2path).VectorNormalize() * tarpath)));
						break;
					}
				}
				break;
			}
			if (i + 1 == length - 1)
			{
				predpos = Extensions::To3D ( (path1 + ( (path2 - path1).VectorNormalize() * Distances (path1, path2))));
				break;
			}
		}
	}
	else
	{
		predpos = target->GetPosition();
	}
	float dist = Distances (predpos, GEntityList->Player()->ServerPosition());
	if (predpos == Vec3 (0, 0, 0))
	{ return false; }
	spell->CastOnPosition (predpos);
	return true;
}