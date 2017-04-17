#pragma once
#include "PluginSDK.h"

class Champion
{
public:
	Champion(IMenu* Parent, IUnit* Hero) :Hero(Hero), ParentMenu(Parent) {}
	~Champion();

	IUnit * GetHero();
	IMenu* GetMenu();

	//events
	virtual void OnGameUpdate();
	virtual void OnRender();
	virtual void OnInterrupt(InterruptibleSpell const& Args);
	virtual void OnSpellCast(CastedSpell const& args);
	virtual void OnCreate(IUnit* object);
	virtual void AntiGapclose(GapCloserSpell const& args);
	virtual bool onMouseWheel(HWND wnd, UINT message, WPARAM wparam, LPARAM lparam);
	virtual bool OnPreCast(int Slot, IUnit* Target, Vec3* StartPosition, Vec3* EndPosition);
	virtual void OnDelete(IUnit* object);
	virtual void OnNewPath(IUnit* Source, const std::vector<Vec3>& path_);
	/*virtual void OnRealSpellCast(CastedSpell const& Args);
	virtual void OnOrbwalkAttack(IUnit* Source, IUnit* Target);
	virtual void BeforeAttack(IUnit* Target);
	virtual void OnLevelUp(IUnit* Source, int NewLevel);
	virtual void OnPlayAnimation(IUnit* Source, std::string const Args);
	virtual void OnExitVisible(IUnit* Source);*/

	/*	You can store variables here that are accessable by all champions - no need to redefine */
protected:
	IMenu*			ParentMenu;
	IUnit*			Hero;
	ISpell2*		Q;
	ISpell2*		W;
	ISpell2*		E;
	ISpell2*		R;
	ISpell2* Flash;
	ISpell2* RFlash;
	ISpell2* EFlash;
	int xOffset = 0;
	int yOffset = 0;
	int Width = 103;
	int Height = 8;
	Vec4 Color = Vec4(105, 198, 5, 255);
	Vec4 FillColor = Vec4(198, 176, 5, 255);
};