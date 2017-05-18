#pragma once
#include "PluginSDK.h"
#include "Champion.h"
#include "Extensions.h"


class Viktor : public virtual Champion
{
public:
	Viktor (IMenu* Parent, IUnit* Hero);
	~Viktor();

	void OnGameUpdate();
	void OnRender();
	void OnInterrupt (InterruptibleSpell const& Args);
//	void OnSpellCast (CastedSpell const& args);
	void OnCreate (IUnit* object);
	void AntiGapclose (GapCloserSpell const& args);
	bool onMouseWheel (HWND wnd, UINT message, WPARAM wparam, LPARAM lparam);
//	bool OnPreCast (int Slot, IUnit* Target, Vec3* StartPosition, Vec3* EndPosition);
	void OnDoCast (CastedSpell const& args);
	void OnOrbwalkPreAttack (IUnit* Target);
	void OnPlayAnimation (IUnit* Source, std::string const AnimationId);

private:
	IMenuOption* ComboAA;
	IMenuOption* ComboAAkey;
	IMenuOption* ComboAALevel;
	std::vector<std::string> PredType;
	IMenuOption* PredictionType;
	IMenu* Prediction;
	void FleeMode();
	void WLogic (IUnit* target);
	FarmLocationVik FindBestLaserLineFarm (bool jg);
	float LaserDistance (Vec2 point, Vec2 segmentStart, Vec2 segmentEnd, bool onlyIfOnSegment = false, bool squared = false);
	float qDmg (IUnit* Target, bool stage);
	float eDmg (IUnit* Target, bool augment);
	float rDmg (IUnit* Target, int ticks);
	float DPS (IUnit* target, bool dpsQ, bool checkQ, bool dpsE, bool dpsR, int rTicks = 2);
	//bool myfunctionHP (IUnit* i, IUnit* j);
	bool jgLaser();
	bool minionLaser();
	void eCast (IUnit* target);
	void Combo();
	void Harass();
	void LaneClear();
	void JungleClear();
	void KillSteal();
	void autoE();
	void Automatic();
	void RFollowLogic (IUnit* target);
	void dmgdraw();
	void Drawing();


	Vec3 TeamFightR (IUnit* pos);
	IMenu* ViktorMenu;
	IMenu* MainMenu;
	IMenu* ComboMenu;
	IMenu* HarassMenu;
	IMenu* LaneClearMenu;
	IMenu* JungleClearMenu;
	IMenu* RSettings;
	IMenu* Drawings;
	IMenu* MiscMenu;
	IMenu* qMenu;
	IMenu* wMenu;
	IMenu* eMenu;
	IMenu* rMenu;

	IMenuOption* ComboQ;
	IMenuOption* ComboQWorked;
	IMenuOption* ComboW;
	IMenuOption* ComboWManager;
	IMenuOption* ComboE;
	IMenuOption* ComboR;
	IMenuOption* rInterrupt;


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
	IMenuOption* comboFullQ;
	IMenuOption* harassFullQ;
	IMenuOption* laneFullQ;
	IMenuOption* jungleFullQ;


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


	IMenuOption* killstealing_e_;
	IMenuOption* harassmentE;
	IMenuOption* harassEMana;
	IMenuOption* laneClearE;
	IMenuOption* laneClearEMana;
	IMenuOption* JungleClearE;
	IMenuOption* JungleClearEMana;
	IMenuOption* seperator2;
	IMenuOption* gapCloserE;
	Vec3 position;

	IUnit *rFollow, *rObject;
	IMenuOption *RInterveral, *ultMin, *eMin, *hunderAuto, *killStealR, *mouseClear, *Laneclear, *drawLC, *HPBarColor, *Fleemode;

	float lastRMoveTick;
	IUnit* QMis;
	ISpell2* Q;
	ISpell2* W;
	ISpell2* E;
	ISpell2* R;

};
