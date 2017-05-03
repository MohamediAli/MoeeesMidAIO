#include "Extensions.h"

eMinionType Extensions::GetMinionType (IUnit* minion)
{
	static std::vector<std::string> normalMinions =
	{
		"sru_chaosminionmelee", "sru_chaosminionranged",
		"sru_orderminionmelee", "sru_orderminionranged",
		"ha_chaosminionmelee", "ha_chaosminionranged",
		"ha_orderminionmelee", "ha_orderminionranged"
	};
	static std::vector<std::string> siegeMinions =
	{
		"sru_chaosminionsiege", "sru_orderminionsiege",
		"ha_chaosminionsiege", "ha_orderminionsiege"
	};
	static std::vector<std::string> superMinions =
	{
		"sru_chaosminionsuper", "sru_orderminionsuper",
		"ha_chaosminionsuper", "ha_orderminionsuper"
	};
	static std::vector<std::string> smallJungleCreeps =
	{
		"sru_razorbeakmini", "sru_murkwolfmini",
		"sru_krugmini", "sru_krugminimini"
	};
	static std::vector<std::string> bigJungleCreeps =
	{
		"sru_razorbeak", "sru_murkwolf", "sru_gromp",
		"sru_krug", "sru_red", "sru_blue", "sru_crab"
	};
	static std::vector<std::string> epicJungleCreeps =
	{
		"sru_dragon_air", "sru_dragon_earth", "sru_dragon_fire",
		"sru_dragon_water", "sru_dragon_elder", "sru_riftherald",
		"sru_baron"
	};
	if (minion == nullptr)
	{
		return kMinionUnknown;
	}
	auto baseSkinName = regex_replace (std::string (minion->GetBaseSkinName()), std::regex ("[^a-zA-Z_]+"), "");
	transform (baseSkinName.begin(), baseSkinName.end(), baseSkinName.begin(), ::tolower);
	if (std::string (baseSkinName).find ("ward") != std::string::npos || std::string (baseSkinName).find ("trinket") != std::string::npos)
	{
		return kMinionWard;
	}
	if (find (normalMinions.begin(), normalMinions.end(), baseSkinName) != normalMinions.end())
	{
		return kMinionNormal;
	}
	if (find (siegeMinions.begin(), siegeMinions.end(), baseSkinName) != siegeMinions.end())
	{
		return kMinionSiege;
	}
	if (find (superMinions.begin(), superMinions.end(), baseSkinName) != superMinions.end())
	{
		return kMinionSuper;
	}
	if (find (smallJungleCreeps.begin(), smallJungleCreeps.end(), baseSkinName) != smallJungleCreeps.end())
	{
		return kMinionJungleSmall;
	}
	if (find (bigJungleCreeps.begin(), bigJungleCreeps.end(), baseSkinName) != bigJungleCreeps.end())
	{
		return kMinionJungleBig;
	}
	if (find (epicJungleCreeps.begin(), epicJungleCreeps.end(), baseSkinName) != epicJungleCreeps.end())
	{
		return kMinionJungleEpic;
	}
	return kMinionUnknown;
}

bool Extensions::isOnSegment (Vec2 * seg1, Vec2 * seg2, Vec2 * point)
{
	double maxX;
	double maxY;
	double minX;
	double minY;
	if (seg1->x > seg2->x)
	{
		maxX = seg1->x;
		minX = seg2->x;
	}
	else
	{
		maxX = seg2->x;
		minX = seg1->x;
	}
	if (seg1->y > seg2->y)
	{
		maxY = seg1->y;
		minY = seg2->y;
	}
	else
	{
		maxY = seg2->y;
		minY = seg1->y;
	}
	if (maxX < point->x || minX > point->x)
	{
		//POSTGIS_DEBUGF (3, "X value %.8f falls outside the range %.8f-%.8f", point->x, minX, maxX);
		return false;
	}
	else if (maxY < point->y || minY > point->y)
	{
		//POSTGIS_DEBUGF (3, "Y value %.8f falls outside the range %.8f-%.8f", point->y, minY, maxY);
		return false;
	}
	return true;
}

ProjectionInfo Extensions::ProjectOn (Vec2 point, Vec2 segmentStart, Vec2 segmentEnd)
{
	auto cx = point.x;
	auto cy = point.y;
	auto ax = segmentStart.x;
	auto ay = segmentStart.y;
	auto bx = segmentEnd.x;
	auto by = segmentEnd.y;
	auto rL = ( (cx - ax) * (bx - ax) + (cy - ay) * (by - ay))
	          / ( (float) pow (bx - ax, 2) + (float) pow (by - ay, 2));
	auto pointLine = Vec2 (ax + rL * (bx - ax), ay + rL * (by - ay));
	float rS;
	if (rL < 0)
	{
		rS = 0;
	}
	else if (rL > 1)
	{
		rS = 1;
	}
	else
	{
		rS = rL;
	}
	auto isOnSegment = rS == (rL) == 0;
	auto pointSegment = isOnSegment ? pointLine : Vec2 (ax + rS * (bx - ax), ay + rS * (by - ay));
	return  ProjectionInfo (isOnSegment, pointSegment, pointLine);
}
int Extensions::RandInt (int min, int max)
{
	return min + (rand() * (int) (max - min) / RAND_MAX);
}

float Extensions::GetDistanceVectors (Vec3 from, Vec3 to)
{
	float x1 = from.x;
	float x2 = to.x;
	float y1 = from.y;
	float y2 = to.y;
	float z1 = from.z;
	float z2 = to.z;
	return static_cast<float> (sqrt (pow ( (x2 - x1), 2.0) + pow ( (y2 - y1), 2.0) + pow ( (z2 - z1), 2.0)));
}

bool Extensions::InFountain (IUnit *unit)
{
	//TODO: Implement
	return unit->HasBuff ("kappachino");
}

const char * const Extensions::BoolToString (bool b)
{
	return b ? "true" : "false";
}


Vec3 Extensions::lerp (float t, Vec3 a, Vec3 b)
{
	return (1 - t) *a + t*b;
}

Vec3 Extensions::Cross (Vec3 a, Vec3 b)
{
	return Vec3 (a.y * b.z - a.z * b.y,
	             a.z * b.x - a.x * b.z,
	             a.x * b.y - a.y * b.x);
}

Vec3 Extensions::RotateZ (Vec3 target, Vec3 Origin, float deg)
{
	float x = ( (target.x - Origin.x) * cos (deg)) - ( (Origin.z - target.z) * sin (deg));
	float z = ( (Origin.z - target.z) * cos (deg)) - ( (target.x - Origin.x) * sin (deg));
	return Vec3 (x, target.y, z);
}

int Extensions::EnemiesInRange (Vec3 Source, float range)
{
	auto Targets = GEntityList->GetAllHeros (false, true);
	auto enemiesInRange = 0;
	for (auto target : Targets)
	{
		if (Extensions::Validate (target) && !target->IsDead() && target->IsHero())
		{
			auto flDistance = (target->GetPosition() - Source).Length();
			if (flDistance <= range)
			{
				enemiesInRange++;
			}
		}
	}
	return enemiesInRange;
}

int Extensions::AlliesInRange (Vec3 Source, float range)
{
	auto Targets = GEntityList->GetAllHeros (true, false);
	auto alliesInRange = 0;
	for (auto target : Targets)
	{
		if (target != nullptr && target->IsValidTarget() && !target->IsDead() && target->IsHero())
		{
			auto flDistance = (target->GetPosition() - Source).Length();
			if (flDistance < range)
			{
				alliesInRange++;
			}
		}
	}
	return alliesInRange;
}

float Extensions::GetDistance (IUnit* Player, IUnit* target)
{
	return (Player->GetPosition() - target->GetPosition()).Length2D();
}

//follow this convention
float Extensions::GetDistanceSqr (Vec3 from, Vec3 to)
{
	return (from - to).LengthSqr();
}
float Extensions::GetDistanceSqr2D (Vec3 from, Vec3 to)
{
	return (from - to).LengthSqr2D();
}

float Extensions::GetDistance (Vec3 from, Vec3 to)
{
	return (from - to).Length();
}

float Extensions::GetDistance (IUnit* from, Vec3 to)
{
	return (from->GetPosition() - to).Length();
}
float Extensions::GetDistance (Vec2 from, Vec2 to)
{
	return (from - to).Length();
}

float Extensions:: GetDistanceSqr (Vec2 from, Vec2 to)
{
	return (from - to).LengthSqr();
}

int Extensions::CountEnemiesInRange (float range)
{
	int enemies = 0;
	for (auto enemy : GEntityList->GetAllHeros (false, true))
	{
		if (enemy != nullptr && GetDistance (GEntityList->Player(), enemy) <= range)
		{
			enemies++;
		}
	}
	return enemies;
}

bool Extensions::IsValid (Vec2 p)
{
	return p.x != 0 && p.y != 0;
}

bool Extensions::IsValid (Vec3 p)
{
	return p.x != 0 && p.z != 0;
}

//Divine's Divine Code.
bool Extensions::Validate (IUnit * obj, IUnit * unit, float range)
{
	return obj != nullptr && ( (!unit && obj->IsValidObject()) || (unit ? obj->IsValidTarget (unit,range) :obj->IsValidTarget()));
}

Vec3 Extensions::To3D (Vec2 p, float height)
{
	return Vec3 (p.x, height, p.y);
}

Vec3 Extensions::To3D (Vec2 p)
{
	return Vec3 (p.x, GNavMesh->GetHeightForPoint (p), p.y);
}
float Extensions::Dist2D (IUnit * to)
{
	return (GEntityList->Player()->GetPosition() - to->GetPosition()).Length2D() -
	       (GEntityList->Player()->BoundingRadius() + to->BoundingRadius());
}

float Extensions::Dist (IUnit * from, IUnit * to)
{
	return (from->GetPosition() - to->GetPosition()).Length();
}

float Extensions::Dist2D (IUnit * from, IUnit * to)
{
	return (from->GetPosition() - to->GetPosition()).Length2D();
}

Vec2 Extensions::To2D (Vec3 p)
{
	return Vec2 (p.x, p.z);
}

float Extensions::Dist2D (Vec3 from, Vec3 to)
{
	return (from - to).Length2D();
}

float Extensions::Dist2D (Vec2 from, Vec2 to)
{
	return (from - to).Length();
}

float Extensions::Dist2D (Vec2 from, Vec3 to)
{
	return (from - To2D (to)).Length();
}


int Extensions::CountMinionsInTargetRange (Vec3 target, float range)
{
	auto minions = 0;
	for (auto minion : GEntityList->GetAllMinions (false, true, true))
	{
		if (minion != nullptr && !minion->IsWard() && GetDistance (target, minion->GetPosition()) <= range)
		{
			if (!minion->IsDead())
			{
				minions++;
			}
		}
	}
	return minions;
}

double Extensions::distance_squared (double *a, double *b)
{
	double dx = a[0] - b[0];
	double dy = a[1] - b[1];
	double dz = a[2] - b[2];
	return dx*dx + dy*dy + dz*dz;
}

void Extensions::DrawLineRectangle (Vec3 start2, Vec3 end2, int radius, float width, Vec4 color)
{
	Vec2 start = start2.To2D();
	Vec2 end = end2.To2D();
	auto dir = (end - start).VectorNormalize();
	auto pDir = dir.Perpendicular();
	auto rightStartPos = start + pDir * radius;
	auto leftStartPos = start - pDir * radius;
	auto rightEndPos = end + pDir * radius;
	auto leftEndPos = end - pDir * radius;
	Vec2 rStartPos, lStartPos, rEndPos, lEndPos;
	GGame->Projection (Vec3 (rightStartPos.x, GEntityList->Player()->GetPosition().y, rightStartPos.y), &rStartPos);
	GGame->Projection (Vec3 (leftStartPos.x, GEntityList->Player()->GetPosition().y, leftStartPos.y), &lStartPos);
	GGame->Projection (Vec3 (rightEndPos.x, GEntityList->Player()->GetPosition().y, rightEndPos.y), &rEndPos);
	GGame->Projection (Vec3 (leftEndPos.x, GEntityList->Player()->GetPosition().y, leftEndPos.y), &lEndPos);
	GRender->DrawLine (rStartPos, rEndPos, color);
	GRender->DrawLine (lStartPos, lEndPos, color);
	GRender->DrawLine (rStartPos, lStartPos, color);
	GRender->DrawLine (lEndPos, rEndPos, color);
}