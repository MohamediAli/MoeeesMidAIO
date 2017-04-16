#pragma once
#include "PluginSDK.h"
#include <algorithm>


struct FarmLocation
{
	Vec3 CastPosition;
	Vec3 CastPositionFrom;
	int HitCount = 0;
	IUnit* CastOnUnit = nullptr;
	std::vector<IUnit*> UnitsHit;
};

namespace Rembrandt
{
	auto DamageModifierFromMasteries(IUnit* Hero, IUnit* Target) -> double;
	auto DrawDamageOnChampionHPBar(IUnit* Hero, double Damage, const char *Text, Vec4 BarColor) -> void;
	auto DrawDamageOnChampionHPBar(IUnit* Hero, double Damage, Vec4 BarColor) -> void;
	auto LengthSqr(Vec3 v) -> float;
	auto IsInRange(Vec2 PositionA, Vec2 PositionB, float Range) -> bool;
	auto IsInRange(Vec3 PositionA, Vec3 PositionB, float Range) -> bool;
	auto FindBestLineCastPosition(std::vector<Vec3> RangeCheckFroms, float range, float castrange, float radius, bool Minions, bool Heroes, FarmLocation& Output) -> void;

}
