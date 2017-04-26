#pragma once
#include "PluginSDK.h"
#include "Champion.h"


class Taliyah : public virtual Champion
{
public:
	Taliyah (IMenu* Parent, IUnit* Hero);
	~Taliyah();

	void OnGameUpdate();
	void OnRender();
	void AntiGapclose (GapCloserSpell const& args);
	void OnInterrupt (InterruptibleSpell const& args);
	void OnCreate (IUnit* object);
	void OnDelete (IUnit* object);
	void OnSpellCast (CastedSpell const& args);

private:
	Vec3 CalculateReturnPos();
	void CastE();
	void Combo();
	void Harass();
	void LaneClear();
	void JungleClear();
	void autoE();
	void Automatic();
	float qDmg (IUnit* Target);
	float wDmg (IUnit* Target);
	float eDmg (IUnit* Target);
	void dmgdraw();
	void KillSteal();
	void Drawing();
	void zigzag();
	bool qFive = true;
	bool eOnGround = false;
	Vec3 PredPos (IUnit* Hero, float Delay);
	float czx = 0, czy = 0, czx2 = 0, czy2 = 0;
	bool cz = false;
	IUnit  *QTarget, *Qing;


	IMenu* TaliyahMenu;
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

	IMenuOption* HPBar;
	IMenuOption* ComboQ;
	IMenuOption* ComboQWorked;
	IMenuOption* ComboW;
	IMenuOption* ComboWManager;
	IMenuOption* ComboE;
	IMenuOption* ComboR;
	IMenuOption* RideR;


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
	IMenuOption* LockQ;

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


	IMenuOption* killStealE;
	IMenuOption* harassE;
	IMenuOption* harassEMana;
	IMenuOption* laneClearE;
	IMenuOption* laneClearEMana;
	IMenuOption* JungleClearE;
	IMenuOption* JungleClearEMana;
	IMenuOption* seperator2;
	IMenuOption* gapCloserE;


};