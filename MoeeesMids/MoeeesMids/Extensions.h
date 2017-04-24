#include "PluginSDK.h"
#include <regex>
#include <cmath>

#define PI 3.14159265
#define TO_RAD(deg)(deg*PI)/180;

namespace Extensions
{

int RandInt (int min, int max);
float GetDistanceVectors (Vec3 from, Vec3 to);
bool InFountain (IUnit *unit);
const char * const BoolToString (bool b);
Vec3 lerp (float t, Vec3 a, Vec3 b);
Vec3 Cross (Vec3 a, Vec3 b);
Vec3 RotateZ (Vec3 target, Vec3 Origin, float deg);
int EnemiesInRange (Vec3 Source, float range);
float GetDistance (IUnit* Player, IUnit* target);
float GetDistance (Vec3 from, Vec3 to);
float GetDistance (IUnit* from, Vec3 to);
float GetDistance (Vec2 from, Vec2 to);
int CountEnemiesInRange (float range);
bool IsValid (Vec2 p);
bool IsValid (Vec3 p);
Vec3 To3D (Vec2 p);
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