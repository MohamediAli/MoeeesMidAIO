#pragma once
#include "PluginSDK.h"
#include "Champion.h"
#include "Extensions.h"

class Ahri : public virtual Champion
{
public:
	Ahri (IMenu* Parent, IUnit* Hero);
	~Ahri();

	void OnGameUpdate();
	void OnRender();
	void AntiGapclose (GapCloserSpell const& args);
	void AntiInterrupt (InterruptibleSpell const& args);
	void OnNewPath (IUnit* Source, const std::vector<Vec3>& path_);
	void OnCreate (IUnit* object);

private:
	std::string MissileName, MissileReturnName;
	ISpell2* QWER;
	IUnit* missileSource;
	IMissileData* Missile;
	Vec3 CalculateReturnPos();
	Vec3 MissileEndPos;
	void MissileReturn (std::string missile, std::string missileReturn, ISpell2* qwer);
	void DrawLineRectangle (Vec3 start2, Vec3 end2, int radius, float width, Vec4 color);
	Vec2 vect2d (Vec2 p1, Vec2 p2);
	bool PointInRectangle (Vec2 A, Vec2 B, Vec2 C, Vec2 D, Vec2 m);
	bool IsInRectangle (Vec3 Start, Vec3 End, Vec3 pointTest, int radius);
	Vec3 GetPredictedUnitPosition (IUnit* Unit, ISpell2* Skillshot, float& TravelTime);
	bool CheckForCollision (ISpell2* Skillshot, Vec3 CheckAtPosition);
	bool BestCastPosition (IUnit* Unit, ISpell2* Skillshot, Vec3& CastPosition, bool CheckCollision);
	IMenuOption* HPBar;
	void CastE (IUnit* target);
	void CastQ (IUnit * target);
	Vec3 getPosToEflash (Vec3 target);
	void CastFlash();
	void PerformFlashCharm();
	void dmgdraw();
	void Combo();
	void Harass();
	void LaneClear();
	void FleeMode();
	void killSteal();
	void Automated();
	void Drawing();

	IMenu* AhriMenu;
	IMenu* MainMenu;
	IMenu* ComboMenu;
	IMenu* HarassMenu;
	IMenu* LaneClearMenu;
	IMenu* JungleClearMenu;
	IMenu* ksSettings;
	IMenu* Drawings;
	IMenu* MiscMenu;
	IMenuOption* PredictionType;
	IMenu* Prediction;
	std::vector<std::string> PredType;

	IMenuOption* ComboQ;
	IMenuOption* ComboW;
	IMenuOption* ComboE;

	IMenuOption* HarassW;
	IMenuOption* HarassE;
	IMenuOption* HarassQ;
	IMenuOption* HarassManaManager;

	IMenuOption* LaneClearQ;
	IMenuOption* LaneClearManaManager;
	IMenuOption* LaneClearMin;

	IMenuOption* JungleClearQ;
	IMenuOption* JungleClearE;
	IMenuOption* JungleClearManaManager;

	IMenuOption* Fleemode;
	IMenuOption* autoQ;
	IMenuOption* interruptE;
	IMenuOption* gapcloseE;
	IMenuOption* ComboAA;
	IMenuOption* ComboAAkey;
	IMenuOption* ComboAALevel;

	IMenuOption* killstealQ;
	IMenuOption* killstealW;
	IMenuOption* killstealE;

	IMenuOption* DrawReady;
	IMenuOption* DrawQ;
	IMenuOption* DrawW;
	IMenuOption* DrawE;
	IMenuOption* DrawR;
	IMenuOption* DrawDmg;
	IMenuOption* FlashCondemn;

};