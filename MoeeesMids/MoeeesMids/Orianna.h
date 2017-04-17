#pragma once
#include "PluginSDK.h"
#include "Champion.h"
#include "Extensions.h"
#include <unordered_map>

class Orianna : public virtual Champion
{
public:IMenu*			OriannaMenu;
	   Orianna(IMenu* Parent, IUnit* Hero);
	   ~Orianna();
	   void OnNewPath(IUnit* Source, const std::vector<Vec3>& path_);
	   void OnGameUpdate();
	   void OnRender();
	   void OnInterrupt(InterruptibleSpell const& Args);
	   void OnSpellCast(CastedSpell const& args);
	   void OnCreate(IUnit* object);
	   bool onMouseWheel(HWND wnd, UINT message, WPARAM wparam, LPARAM lparam);
	   bool OnPreCast(int Slot, IUnit* Target, Vec3* StartPosition, Vec3* EndPosition);
	   void AntiGapclose(GapCloserSpell const& args);


private:

	int SpellCheck(Vec3 Location, int range, double delay);
	BOOL isBallMoving();
	Vec3 GetMovingBallPos();
	bool pairCompare(const std::pair<int, Vec2>& firstElem, const std::pair<int, Vec2>& secondElem);
	void TeamFightQ(Vec3 pos);
	Vec3 FarmQ(Vec3 pos);
	int GetEHits();
	void CastE(IUnit* target);
	void CastQ(IUnit* target);
	void eLogic();
	void Combo();
	void Harass();
	void LaneClear();
	void Automatic();
	float qDmg(IUnit* Target);
	float wDmg(IUnit* Target);
	float rDmg(IUnit* Target);
	void KillSteal();
	void Drawing();
	void DrawGagongReplicate(Vec3 BallPos);
	void dmgdraw();
	void DrawGagonDix(Vec3 BallPos, Vec4 Color);
	void PerformFlashUlt();
	Vec3 getPosToRflash(Vec3 target);
	void CastFlash();
	IUnit *StationaryBall, *MovingBall;
	Vec3 BallPosition;
	int Ticks;
	IMenuOption* PredictionType;
	IMenu* Prediction;
	
	float BallRad = 75;

	int xOffset = 0;
	int yOffset = 0;
	int Width = 103;
	int Height = 8;
	Vec4 Color = Vec4(105, 198, 5, 255);
	Vec4 FillColor = Vec4(198, 176, 5, 255);
	float lastTimeStamp;
	float gagongAngle;
	Vec4 GagongColors[9] =
	{ Vec4(255,255,26,255),Vec4(242,56,90,255),Vec4(245,165,3,255),
		Vec4(167,197,32,255),Vec4(73,63,11	,255),Vec4(247,233,103,255)
		,Vec4(169,207,84,255),Vec4(255,133,52,255),Vec4(201,215,135,255)
	};

	Vec4 GagongColorsMove[9] =
	{ Vec4(65,115,120,255),Vec4(255,53,139,255),Vec4(1,176,240,255),
		Vec4(174,238,0,255),Vec4(217,4,43,255),Vec4(44,29,255,255)
		,Vec4(183,0,0,255),Vec4(255,202,61,255),Vec4(0,146,178,255)
	};

	IMenu* rMenu;
	IMenu* HarassMenu;
	IMenu* LaneClearMenu;
	IMenu* JungleClearMenu;
	IMenu* RSettings;
	IMenu* Drawings;
	IMenu* MiscMenu;
	IMenu* qMenu;
	IMenu* wMenu;
	IMenu* eMenu;

	IMenuOption* ComboQ;
	IMenuOption* ComboQWorked;
	IMenuOption* ComboW;
	IMenuOption* ComboWManager;
	IMenuOption* ComboE;
	IMenuOption* ComboR;
	IMenuOption* BlockR;


	IMenuOption* DrawReady;
	IMenuOption* DrawQ;
	IMenuOption* DrawW;
	IMenuOption* DrawE;
	IMenuOption* DrawR;
	IMenuOption* drawDmg;

	IMenuOption* killStealQ;
	IMenuOption* harassQ;
	IMenuOption* harassQMana;
	IMenuOption* laneClearQ;
	IMenuOption* laneClearQMana;
	IMenuOption* JungleClearQ;
	IMenuOption* JungleClearQMana;
	IMenuOption* seperator;
	IMenuOption* gapcloseQ;

	IMenuOption* killStealW;
	IMenuOption* EonlyW;
	IMenuOption* harassW;
	IMenuOption* harassWMana;
	IMenuOption* laneClearW;
	IMenuOption* laneClearWMana;
	IMenuOption* JungleClearW;
	IMenuOption* JungleClearWMana;
	IMenuOption* seperator1;
	IMenuOption* gapCloserW;
	IMenuOption* interrupterW;
	IMenuOption* comboFullW;
	IMenuOption* FlashUlt;

	IMenuOption* killStealE;
	IMenuOption* harassE;
	IMenuOption* harassEMana;
	IMenuOption* laneClearE;
	IMenuOption* laneClearEMana;
	IMenuOption* JungleClearE;
	IMenuOption* JungleClearEMana;
	IMenuOption* seperator2;
	IMenuOption* gapCloserE;
	IMenuOption* autoQ;
	IMenuOption* autoW;
	IMenuOption *HealthPercent, *ShieldTeamate, *ShieldTeamatePercent, *ultMin, *drawBall, *ballSelect, *Laneclear, *KillStealR, *InterruptR;
};