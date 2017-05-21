#include "Core.h"
PluginSetup ("Moeee's Mid AIO");

PLUGIN_EVENT (void) OnGameUpdate()
{
	ChampHandler->OnGameUpdate();
}

PLUGIN_EVENT (void) OnRender()
{
	ChampHandler->OnRender();
}

PLUGIN_EVENT (void) OnInterrupt (InterruptibleSpell const& Args)
{
	ChampHandler->OnInterrupt (Args);
}

PLUGIN_EVENT (void) OnSpellCast (CastedSpell const& args)
{
	ChampHandler->OnSpellCast (args);
}

PLUGIN_EVENT (void) OnDoCast (CastedSpell const& args)
{
	ChampHandler->OnSpellCast (args);
}

PLUGIN_EVENT (void) OnOrbwalkPreAttack (IUnit* Target)
{
	ChampHandler->OnCreate (Target);
}

PLUGIN_EVENT (void) OnCreate (IUnit* object)
{
	ChampHandler->OnCreate (object);
}
PLUGIN_EVENT (void) OnDelete (IUnit* object)
{
	ChampHandler->OnDelete (object);
}

PLUGIN_EVENT (void) AntiGapclose (GapCloserSpell const& args)
{
	ChampHandler->AntiGapclose (args);
}

PLUGIN_EVENT (void) OnNewPath (IUnit* Source, const std::vector<Vec3>& path_)
{
	ChampHandler->OnNewPath (Source, path_);
}

PLUGIN_EVENT (bool) onMouseWheel (HWND wnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	return ChampHandler->onMouseWheel (wnd,message,wparam,lparam);
}

PLUGIN_EVENT (bool) OnPreCast (int Slot, IUnit* Target, Vec3* StartPosition, Vec3* EndPosition)
{
	return ChampHandler->OnPreCast (Slot, Target, StartPosition, EndPosition);
}

PLUGIN_EVENT (void) OnExitVisible (IUnit* Args)
{
	ChampHandler->OnExitVisible (Args);
}

PLUGIN_EVENT (void) OnPlayAnimation (IUnit* Source, std::string const Args)
{
	ChampHandler->OnPlayAnimation (Source, Args);
}

PLUGIN_EVENT (void) OnDash (UnitDash* Args)
{
	ChampHandler->OnDash (Args);
}

void  LoadEvents()
{
	GEventManager->AddEventHandler (kEventOnGameUpdate, OnGameUpdate);
	GEventManager->AddEventHandler (kEventOnRender, OnRender);
	GEventManager->AddEventHandler (kEventOnInterruptible, OnInterrupt);
	GEventManager->AddEventHandler (kEventOnSpellCast, OnSpellCast);
	GEventManager->AddEventHandler (kEventOnPreCast, OnPreCast);
	GEventManager->AddEventHandler (kEventOnGapCloser, AntiGapclose);
	GEventManager->AddEventHandler (kEventOnCreateObject, OnCreate);
	GEventManager->AddEventHandler (kEventOnWndProc, onMouseWheel);
	GEventManager->AddEventHandler (kEventOnDestroyObject, OnDelete);
	GEventManager->AddEventHandler (kEventOnNewPath, OnNewPath);
	GEventManager->AddEventHandler (kEventOnDoCast, OnDoCast);
	GEventManager->AddEventHandler (kEventOrbwalkBeforeAttack, OnOrbwalkPreAttack);
	GEventManager->AddEventHandler (kEventOnExitVisible, OnExitVisible);
	GEventManager->AddEventHandler (kEventOnPlayAnimation, OnPlayAnimation);
	GEventManager->AddEventHandler (kEventOnDash, OnDash);
	GGame->PrintChat ("<font color=\"#ff4dee\"><b>Moeee's Mid AIO (Ahri, Orianna, Karthus, Taliyah, Viktor, </b></font> <font color = \"#62ff4d\"><b>Cassiopeia)</b></font><b><font color=\"#FFFFFF\"><b><font color=\"#FFFFFF\">  Loaded!</font></b>");
	GGame->PrintChat ("<b><font color=\"#FF0000\"> DELETE Config if you are having any issues!</font></b>");
}

void  UnloadEvents()
{
	GEventManager->RemoveEventHandler (kEventOnGameUpdate, OnGameUpdate);
	GEventManager->RemoveEventHandler (kEventOnRender, OnRender);
	GEventManager->RemoveEventHandler (kEventOnInterruptible, OnInterrupt);
	GEventManager->RemoveEventHandler (kEventOnSpellCast, OnSpellCast);
	GEventManager->RemoveEventHandler (kEventOnPreCast, OnPreCast);
	GEventManager->RemoveEventHandler (kEventOnGapCloser, AntiGapclose);
	GEventManager->RemoveEventHandler (kEventOnCreateObject, OnCreate);
	GEventManager->RemoveEventHandler (kEventOnWndProc, onMouseWheel);
	GEventManager->RemoveEventHandler (kEventOnDestroyObject, OnDelete);
	GEventManager->RemoveEventHandler (kEventOnNewPath, OnNewPath);
	GEventManager->RemoveEventHandler (kEventOnDoCast, OnDoCast);
	GEventManager->RemoveEventHandler (kEventOrbwalkBeforeAttack, OnOrbwalkPreAttack);
	GEventManager->RemoveEventHandler (kEventOnExitVisible, OnExitVisible);
	GEventManager->RemoveEventHandler (kEventOnPlayAnimation, OnPlayAnimation);
	GEventManager->RemoveEventHandler (kEventOnDash, OnDash);
}

// Called when plugin is first loaded
PLUGIN_API void OnLoad (IPluginSDK* PluginSDK)
{
	// Initializes global interfaces for core access
	PluginSDKSetup (PluginSDK);
	Player = GEntityList->Player();
	//Initialize Menus
	MainMenu = GPluginSDK->AddMenu ("Moeees Mid AIO");
	//LOAD CHAMP PLUGIN
	if (strstr (Player->ChampionName(), "Orianna"))
	{
		ChampMenu = MainMenu->AddMenu ("Orianna");
		ChampHandler = new Orianna (ChampMenu, Player);
	}
	//Load default empty handler if champ not found
	else if (strstr (Player->ChampionName(), "Ahri"))
	{
		ChampMenu = MainMenu->AddMenu ("Ahri");
		ChampHandler = new Ahri (ChampMenu, Player);
	}
	else if (strstr (Player->ChampionName(), "Taliyah"))
	{
		ChampMenu = MainMenu->AddMenu ("Taliyah");
		ChampHandler = new Taliyah (ChampMenu, Player);
	}
	else if (strstr (Player->ChampionName(), "Karthus"))
	{
		ChampMenu = MainMenu->AddMenu ("Karthus");
		ChampHandler = new Karthus (ChampMenu, Player);
	}
	else if (strstr (Player->ChampionName(), "Viktor"))
	{
		ChampMenu = MainMenu->AddMenu ("Viktor");
		ChampHandler = new Viktor (ChampMenu, Player);
	}
	else if (strstr (Player->ChampionName(), "Cassiopeia"))
	{
		ChampMenu = MainMenu->AddMenu ("Cassiopeia");
		ChampHandler = new Cassiopeia (ChampMenu, Player);
	}
	else
	{ ChampHandler = new Champion (ChampMenu, Player); }
	LoadEvents();
}

// Called when plugin is unloaded
PLUGIN_API void OnUnload()
{
	GGame->PrintChat ("<font color=\"#FFFFFF\"><b>Moeee's Mid AIO</b></font><b><font color=\"#FF0000\"> UnLoaded!</font></b>");
	MainMenu->Remove();
	UnloadEvents();
}