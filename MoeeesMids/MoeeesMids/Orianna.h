#pragma once
#include "PluginSDK.h"
#include "Champion.h"
#include "Extensions.h"
#include <unordered_map>


class Orianna : public virtual Champion
{
public:
	IMenu*			OriannaMenu;
	Orianna (IMenu* Parent, IUnit* Hero);
	~Orianna();
	void OnNewPath (IUnit* Source, const std::vector<Vec3>& path_);
	void OnGameUpdate();
	void OnRender();
	void OnInterrupt (InterruptibleSpell const& Args);
	void OnSpellCast (CastedSpell const& args);
	void OnCreate (IUnit* object);
	bool onMouseWheel (HWND wnd, UINT message, WPARAM wparam, LPARAM lparam);
	bool OnPreCast (int Slot, IUnit* Target, Vec3* StartPosition, Vec3* EndPosition);
	void AntiGapclose (GapCloserSpell const& args);



private:
	std::vector<std::string> PredType;
	std::vector<std::string> ballAnimation;
	std::vector<std::string> ballColor;

	void eAssist();
	IUnit* BallMissile;
	bool PriorityHit();
	bool IsOneVsOne();
	float rD1v1 (IUnit* Target);
	bool isChasing (IUnit* Target);
	bool SpellCheckKS (IUnit* pos, int range, float delay, IUnit* target);
	bool BestCastPosition (IUnit* Unit, ISpell2* Skillshot, Vec3& CastPosition, bool CheckCollision);
	bool CheckForCollision (ISpell2* Skillshot, Vec3 CheckAtPosition);
	Vec3 GetPredictedUnitPosition (IUnit* Unit, ISpell2* Skillshot, float& TravelTime);
	bool IsInRectangle (Vec3 Start, Vec3 End, Vec3 pointTest, int radius);
	bool PointInRectangle (Vec2 A, Vec2 B, Vec2 C, Vec2 D, Vec2 m);
	Vec2 vect2d (Vec2 p1, Vec2 p2);
	void SaveTeam();
	float BallDelay();
	//std::tuple<int, Vec3> GetBestQLocation (IUnit* mainTarget);
	int SpellCheck (IUnit* Location, int range, float delay);
	BOOL isBallMoving();
	Vec3 GetMovingBallPos();
	Vec3 GetMovingBallPosW();
	bool pairCompare (const std::pair<int, Vec2>& firstElem, const std::pair<int, Vec2>& secondElem);
	void TeamFightQ (Vec3 pos);
	void FarmQ (Vec3 pos);
	int GetEHits();
	void CastE (IUnit* target);
	void CastQ (IUnit* target);
	void eLogic();
	int enemyT;
	void Combo();
	void Harass();
	void LaneClear();
	void Automatic();
	float qDmg (IUnit* Target);
	float wDmg (IUnit* Target);
	float rDmg (IUnit* Target);
	void KillSteal();
	void Drawing();
	void DrawGagongReplicate (Vec3 BallPos);
	void dmgdraw();
	void DrawGagonDix (Vec3 BallPos, Vec4 Color);
	void PerformFlashUlt();
	Vec3 getPosToRflash (Vec3 target);
	void CastFlash();
	IUnit *StationaryBall, *MovingBall;
	Vec3 BallPosition;
	int Ticks;
	IMenuOption* PredictionType;
	IMenu* Prediction;
	std::vector<std::pair<int, Vec3>> GetBestQLocation (IUnit* mainTarget);

	float BallRad = 75;
	float lastTimeStamp;
	float gagongAngle;
	Vec4 GagongColors[9] =
	{
		Vec4 (255,255,26,255),Vec4 (242,56,90,255),Vec4 (245,165,3,255),
		Vec4 (167,197,32,255),Vec4 (73,63,11	,255),Vec4 (247,233,103,255)
		,Vec4 (169,207,84,255),Vec4 (255,133,52,255),Vec4 (201,215,135,255)
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
	IMenu *oneVerusOne, *DivineBall ;
	IMenuOption* HPBarColor;

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
	IMenuOption *FlashUlt, *DiscoMode, *RandomMode;
	Vec4 Color1, Color2, Color3, Color4, Color5, Color6, Color7, Color8;


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
	IMenuOption *HealthPercentage, *ShieldTeamate, *ShieldTeamatePercent, *ultMin, *drawBall, *ballSelect, *Laneclear, *KillStealR, *InterruptR, *autoEiniti, *mouseClear, *drawLC, *extraAutos, *onev1, *priorityMin, *eHelper, *eHelperKey,
	            *DivineColor1, *DivineColor2, *DivineColor3, *DivineColor4, *DivineColor5, *DivineColor6, *DivineColor7, *DivineColor8 ;
};