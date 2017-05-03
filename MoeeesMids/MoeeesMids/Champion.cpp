#include "Champion.h"

Champion::~Champion()
{
	ParentMenu->Remove();
	if (Q) { delete Q; }
	if (W) { delete W; }
	if (E) { delete E; }
	if (R) { delete R; }
}

IUnit* Champion::GetHero()
{
	return Hero;
}

IMenu* Champion::GetMenu()
{
	return ParentMenu;
}

//ADD NEW PLUGIN EVENTS HERE
void Champion::OnGameUpdate()
{
}

void Champion::OnRender()
{
}

void Champion::OnInterrupt (InterruptibleSpell const& Args)
{
}

void Champion::OnSpellCast (CastedSpell const& args)
{
}

void Champion::OnCreate (IUnit* object)
{
}

void Champion::AntiGapclose (GapCloserSpell const& args)
{
}

bool Champion::onMouseWheel (HWND wnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	return true;
}

bool Champion::OnPreCast (int Slot, IUnit* Target, Vec3* StartPosition, Vec3* EndPosition)
{
	return true;
}

void Champion::OnDelete (IUnit* object)
{
}

void Champion::OnNewPath (IUnit* Source, const std::vector<Vec3>& path_)
{
}

void Champion::OnDoCast (CastedSpell const& args)
{
}

void Champion::OnOrbwalkPreAttack (IUnit* Target)
{
}