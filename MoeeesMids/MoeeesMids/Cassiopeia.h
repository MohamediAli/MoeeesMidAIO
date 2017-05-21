#pragma once
#include "PluginSDK.h"
#include "Champion.h"


class Cassiopeia : public virtual Champion
{
public:
	Cassiopeia (IMenu* Parent, IUnit* Hero);
	~Cassiopeia();

	void OnGameUpdate();
	void OnRender();
	void AntiGapclose (GapCloserSpell const& args);
	bool onMouseWheel (HWND wnd, UINT message, WPARAM wparam, LPARAM lparam);
	bool OnPreCast (int Slot, IUnit* Target, Vec3* StartPosition, Vec3* EndPosition);
	void OnOrbwalkPreAttack (IUnit* Target);
	void OnDash (UnitDash* Args);
	void OnSpellCast (CastedSpell const& args);
	void OnInterrupt (InterruptibleSpell const& Args);


private:
	std::vector<Vec2> rClose (std::vector< Vec3> pos);
	float WWidth();
	float WMaxRangeSqr();
	float WMaxRange();
	float DPS (IUnit* target, bool dpsQ, bool dpsW, bool dpsE, bool dpsR, int eTimes = 1);
	void TryMassUlt();
	void PerformFlashUlt();
	void CastAssistedUlt();
	void CastFlash();
	void Combo();
	void Harass();
	void LaneClear();
	void LastHit();
	void dmgDraw();
	void Automated();
	void Drawing();
	void CastW (IUnit* target);
	void qLogic (IUnit* target);
	void CastE (IUnit* target);
	void eLogic (IUnit* target);
	void CastWInterrupt (IUnit* target);
	bool IsInWRange (Vec2 position);
	bool validRPos (IUnit* target, ISpell2* spell, Vec3& cast);
	bool GetBestPosition (std::vector<IUnit*> targets, std::vector<std::pair<Vec3, int>>& result, ISpell2* spell, Vec3 disPos);
	bool GetImpactTime (ISpell2* spell, IUnit* source, IUnit* unit, float& impact);
	bool FarmQ (Vec3 pos, ISpell2* spell, std::vector<std::pair<int, Vec2>>& result);
	bool eCheck (IUnit* target);
	bool GetCastPosition (ISpell2* spell, IUnit* source, IUnit* unit, Vec3& cast);
	Vec3 getPosToRflash (Vec3 target);


	IMenu       * CassiopeiaMenu;
	IMenu       * MainMenu;
	IMenu       * ComboMenu;
	IMenu       * LaneClearMenu;
	IMenu       * JungleClearMenu;
	IMenu       * ksSettings;
	IMenu       * Drawings;
	IMenu       * MiscMenu;

	IMenuOption * ComboQ;
	IMenuOption * ComboW;
	IMenuOption * ComboE;

	IMenuOption *LaneClearQ, *LaneClearE;
	IMenuOption * LaneClearW;
	IMenuOption * LaneClearManaManager;
	IMenuOption * LaneClearWMin;
	IMenuOption * LaneClearQMin;

	IMenuOption * JungleClearQ;
	IMenuOption * JungleClearE;
	IMenuOption * JungleClearManaManager;

	IMenuOption * Fleemode;
	IMenuOption * interruptE;
	IMenuOption * gapcloseR;
	IMenuOption * gapcloseW;
	IMenuOption * ComboAA;
	IMenuOption * ComboAAkey;
	IMenuOption * ComboAALevel;

	IMenuOption * harassQ;
	IMenuOption *harassE, *harassW;

	IMenuOption * DrawReady;
	IMenuOption * DrawQ;
	IMenuOption * DrawW;
	IMenuOption * DrawE;
	IMenuOption * DrawR;
	IMenuOption * DrawDmg;
	IMenuOption * FlashUlt;

	ISpell2     * Q;
	ISpell2     * W;
	ISpell2     * E;
	ISpell2     * R;
	ISpell2     * Flash;
	ISpell2     * RFlash;


	IMenu *ultMenu, *harassMenu, *laneClearMenu, *lastHitMenu, *burstModeMenu,
	      *laneDominant, *gapcloserMenu, *manaManagerMenu;
	IMenuOption *minFacing, *minNotfacing, *autoUlt, *ultKillable, *blockUlt, *assistedUlt,
	            *autoHarass, *HarassManaManager, *burstKey, *autoBurst, *eDelay, *qPoision,
	            *stackT, *poisonTS, *lhPriority, *lhE, *lhQ, *lhqMana, *lhqMin, *laneDom,
	            *minHealth, *ComboR, *mouseClear, *Laneclear, *drawLC, *draw_Dmg, *HPBarColor,
	            *harassWMinMana, *InterruptR;

	bool scriptR = false;
	bool notified = false;
	bool didiPress = false;
	bool burstMode = false;
	bool laneMode = false;
	bool lhAble = false;

};