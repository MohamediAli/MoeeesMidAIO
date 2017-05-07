#pragma once
#include "PluginSDK.h"
#include <regex>
#include <cmath>

#define PI 3.14159265
#define TO_RAD(deg) deg*PI/180;

struct ProjectionInfo
{
	ProjectionInfo() { }

	/// <summary>
	/// The is on segment
	/// </summary>
	bool IsOnSegment;

	/// <summary>
	/// The line point
	/// </summary>
	Vec2 LinePoint;

	/// <summary>
	/// The segment point
	/// </summary>
	Vec2 SegmentPoint;

	/// <summary>
	/// Initializes a new instance of the <see cref="ProjectionInfo"/> struct.
	/// </summary>
	/// <param name="isOnSegment">if set to <c>true</c> [is on segment].</param>
	/// <param name="segmentPoint">The segment point.</param>
	/// <param name="linePoint">The line point.</param>
	ProjectionInfo (bool isOnSegment, Vec2 segmentPoint, Vec2 linePoint)
	{
		this->IsOnSegment = isOnSegment;
		this->SegmentPoint = segmentPoint;
		this->LinePoint = linePoint;
	}
};

enum eMinionType
{
	kMinionUnknown = 0,
	kMinionWard = 1,
	kMinionNormal = 2,
	kMinionSiege = 3,
	kMinionSuper = 4,
	kMinionJungleSmall = 5,
	kMinionJungleBig = 6,
	kMinionJungleEpic = 7,
	kMinionNormalREAL = 8
};

struct FarmLocationVik
{
	/// <summary>
	/// The minions hit
	/// </summary>
	int MinionsHit;

	/// <summary>
	/// The start position
	/// </summary>
	Vec2 Position1;


	/// <summary>
	/// The end position
	/// </summary>
	Vec2 Position2;

	/// <summary>
	/// Initializes a new instance of the <see cref="FarmLocation"/> struct.
	/// </summary>
	/// <param name="position">The position.</param>
	/// <param name="minionsHit">The minions hit.</param>
	FarmLocationVik (Vec2 startpos, Vec2 endpos, int minionsHit)
	{
		Position1 = startpos;
		Position2 = endpos;
		MinionsHit = minionsHit;
	}
};

namespace Extensions
{
eMinionType GetMinionType (IUnit* minion);
ProjectionInfo ProjectOn (Vec2 point, Vec2 segmentStart, Vec2 segmentEnd);
bool isOnSegment (Vec2 * seg1, Vec2 * seg2, Vec2 * point);
void DrawLineRectangle (Vec3 start2, Vec3 end2, int radius, float width, Vec4 color);
int RandInt (int min, int max);
float GetDistanceVectors (Vec3 from, Vec3 to);
bool InFountain (IUnit *unit);
const char * const BoolToString (bool b);
Vec3 lerp (float t, Vec3 a, Vec3 b);
Vec3 Cross (Vec3 a, Vec3 b);
Vec3 RotateZ (Vec3 target, Vec3 Origin, float deg);
int EnemiesInRange (Vec3 Source, float range);
bool Validate (IUnit * obj, IUnit * unit = nullptr, float range = FLT_MAX);
float GetDistance (IUnit* Player, IUnit* target);
float GetDistanceSqr (Vec3 from, Vec3 to);
float GetDistanceSqr2D (Vec3 from, Vec3 to);
float GetDistance (Vec3 from, Vec3 to);
float GetDistance (IUnit* from, Vec3 to);
float GetDistance (Vec2 from, Vec2 to);
float GetDistanceSqr (Vec2 from, Vec2 to);
int CountEnemiesInRange (float range);
float SegmenDistance (Vec2 point, Vec2 segmentStart, Vec2 segmentEnd, bool onlyIfOnSegment, bool squared);
bool IsValid (Vec2 p);
bool IsValid (Vec3 p);
Vec3 To3D (Vec2 p);
Vec3 To3D (Vec2 p, float height);
float Dist2D (IUnit * to);
float Dist (IUnit * from, IUnit * to);
float Dist2D (IUnit * from, IUnit * to);
Vec2 To2D (Vec3 p);
float Dist2D (Vec3 from, Vec3 to);
float Dist2D (Vec2 from, Vec2 to);
float Dist2D (Vec2 from, Vec3 to);
int CountMinionsInTargetRange (Vec3 target, float range);
double distance_squared (double *a, double *b);
int AlliesInRange (Vec3 Source, float range);
}