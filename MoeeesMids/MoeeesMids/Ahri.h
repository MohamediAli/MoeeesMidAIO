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

private:
    Vec3 RunPrediction (IUnit* Target, ISpell2* Skillshot);
    IMenuOption* HPBar;
    void CastE (IUnit* target);
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