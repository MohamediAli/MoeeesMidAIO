#pragma once
#include "PluginSDK.h"
#include "Champion.h"



class Karthus : public virtual Champion
{
public:
    Karthus (IMenu* Parent, IUnit* Hero);
    ~Karthus();

    void OnGameUpdate();
    void OnRender();
    void AntiGapclose (GapCloserSpell const& args);

private:

    void automaticE();
    void eToggle();
    float qWidthChange (IUnit* target);
    float wWidthChange (IUnit* target);
    void zigzag();
    Vec3 PredPos (IUnit* Hero, float Delay);
    Vec3 FarmQ (Vec3 pos);
    void Combo();
    void Harass();
    float qDmg (IUnit* Target);
    float rDmg (IUnit* Target);
    void dmgdraw();
    void LaneClear();
    void LastHit();
    void Drawing();

    IMenu*			KarthusMenu;
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