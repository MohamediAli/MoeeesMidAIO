#include "Cassiopeia.h"
#include "PluginSDK.h"
#include "cmath"
#include "Rembrandt.h"
#include "Extensions.h"
#include "MEC.h"
#define M_PI 3.14159265358979323846
#define NOMINMAX


Cassiopeia::~Cassiopeia()
{
	CassiopeiaMenu->Remove();
}

Cassiopeia::Cassiopeia (IMenu* Parent, IUnit* Hero) :Champion (Parent, Hero)
{
	Q = GPluginSDK->CreateSpell2 (kSlotQ, kCircleCast, true, false, kCollidesWithNothing);
	Q->SetSkillshot (0.3f, 160.f, 2000.f, 850.f);
	W = GPluginSDK->CreateSpell2 (kSlotW, kLineCast, true, true, kCollidesWithYasuoWall);
	W->SetSkillshot (0.85f, 180.f, 1600.f, 800.f); //0.25
	E = GPluginSDK->CreateSpell2 (kSlotE, kTargetCast, true, false, kCollidesWithYasuoWall);
	R = GPluginSDK->CreateSpell2 (kSlotR, kConeCast, false, true, kCollidesWithNothing); //825 range, 210 radius, 0.5 delay
	R->SetSkillshot (0.5f, (float) (210 * PI / 180), FLT_MAX, 825.f);
	R->SetTriggerEvents (false);
	RFlash = GPluginSDK->CreateSpell2 (kSlotE, kLineCast, false, false, static_cast<eCollisionFlags> (kCollidesWithMinions | kCollidesWithYasuoWall));
	RFlash->SetSkillshot (0.5f, (float) (210 * PI / 180), FLT_MAX, 1150);
	if (strcmp (Hero->GetSpellName (kSummonerSlot1), "SummonerFlash") == 0)
	{
		Flash = GPluginSDK->CreateSpell2 (kSummonerSlot1, kCircleCast, false, false, kCollidesWithNothing);
		Flash->SetOverrideRange (425.f);
	}
	if (strcmp (Hero->GetSpellName (kSummonerSlot2), "SummonerFlash") == 0)
	{
		Flash = GPluginSDK->CreateSpell2 (kSummonerSlot2, kCircleCast, false, false, kCollidesWithNothing);
		Flash->SetOverrideRange (425.f);
	}

	CassiopeiaMenu       = Parent->AddMenu ("Cassiopeia Menu");
	ComboMenu            = Parent->AddMenu ("Combo");
	ultMenu              = Parent->AddMenu ("Ultimate Settings");
	harassMenu           = Parent->AddMenu ("Harass");
	laneClearMenu        = Parent->AddMenu ("Lane Clear");
	lastHitMenu          = Parent->AddMenu ("Last Hit");
	burstModeMenu        = Parent->AddMenu ("Burst Mode");
	laneDominant         = Parent->AddMenu ("Lane Pressure Mode");
	gapcloserMenu        = Parent->AddMenu ("Gapcloser");
	manaManagerMenu      = Parent->AddMenu ("Mana Manager");
	MiscMenu             = Parent->AddMenu ("Miscs");
	Drawings             = Parent->AddMenu ("Spell Drawings");

	minFacing            = ultMenu->AddInteger ("Min Enemies (Facing)", 1, 5, 2);
	minNotfacing         = ultMenu->AddInteger ("->Min Enemies (Not Facing)", 1, 5, 4);
	autoUlt              = ultMenu->CheckBox ("->Only in Combo", true);
	ultKillable          = ultMenu->CheckBox ("Ult if Target Killiable with Combo", true);
	minHealth            = ultMenu->AddInteger ("Only R if Target has Health % > Than", 0, 100, 20);
	blockUlt             = ultMenu->CheckBox ("Block R that Won't Hit", true);
	FlashUlt             = ultMenu->AddKey ("Ult Flash ", 84);
	assistedUlt          = ultMenu->AddKey ("Assisted Ult", 82);
	InterruptR           = ultMenu->CheckBox ("Ult On Interruptible Spells", true);

	ComboQ               = ComboMenu->CheckBox ("Use Q", true);
	qPoision             = ComboMenu->CheckBox ("->Only Q if not poisoned", false);
	ComboW               = ComboMenu->CheckBox ("Use W", true);
	ComboE               = ComboMenu->CheckBox ("Use E", true);
	eDelay               = ComboMenu->AddInteger ("->Random E Delay Range", 0, 250, 50);
	ComboR               = ComboMenu->CheckBox ("Use Ult", true);
	ComboAALevel         = ComboMenu->AddInteger ("At what level disable AA", 1, 18, 6);
	ComboAA              = ComboMenu->CheckBox ("Disable AA", false);
	ComboAAkey           = ComboMenu->AddKey ("Disable key", 32);

	harassQ              = harassMenu->CheckBox ("Use Q", true);
	harassW              = harassMenu->CheckBox ("Use W", false);
	harassWMinMana       = harassMenu->AddInteger ("->Min Mana %", 0, 100, 85);
	harassE              = harassMenu->CheckBox ("Use E", true);
	autoHarass           = harassMenu->CheckBox ("Auto Harass Q", false);
	HarassManaManager    = harassMenu->AddInteger ("->Min Mana %", 0, 100, 60);

	Laneclear            = laneClearMenu->CheckBox ("Use Spells", true);
	LaneClearQ           = laneClearMenu->CheckBox ("Use Q", true);
	LaneClearQMin        = laneClearMenu->AddInteger ("->Minimum Minions", 1, 6, 2);
	LaneClearW           = laneClearMenu->CheckBox ("Use W", true);
	LaneClearWMin        = laneClearMenu->AddInteger ("->Minimum Minions", 1, 10, 4);
	LaneClearE           = laneClearMenu->CheckBox ("Use E", true);
	LaneClearManaManager = laneClearMenu->AddInteger ("->Min Mana %", 0, 100, 60);
	mouseClear           = laneClearMenu->CheckBox ("Mouse Scroll to Toggle Wave Clear", true);


	lhE                  = lastHitMenu->CheckBox ("Use E", true);
	lhQ                  = lastHitMenu->CheckBox ("Use Q", false);
	lhqMana              = lastHitMenu->AddInteger ("->Min Mana %", 0, 100, 60);
	lhqMin               = lastHitMenu->AddInteger ("->Min Minions Hit", 0, 6, 3);

	stackT               = MiscMenu->CheckBox ("Stack Tear", true);
	poisonTS             = MiscMenu->CheckBox ("Poison Effects TS", false);
	lhPriority           = MiscMenu->CheckBox ("Lasthit > Harass with E in mixed mode", false);

	burstKey             = burstModeMenu->AddKey ("Burst Mode Enabled", 78);
	autoBurst            = burstModeMenu->AddInteger ("Auto-Burst Mode if My Health % < Than", 0, 100, 25);

	laneDom              = laneDominant->AddKey ("Lane Dominant Mode", 74);

	gapcloseR            = gapcloserMenu->AddInteger ("Use R if My Health % <", 0, 100, 40);
	gapcloseW            = gapcloserMenu->CheckBox ("Else use W", true);

	DrawReady            = Drawings->CheckBox ("Draw Ready Spells", true);
	DrawQ                = Drawings->CheckBox ("Draw Q Range", true);
	DrawW                = Drawings->CheckBox ("Draw W Zone", true);
	DrawE                = Drawings->CheckBox ("Draw E Range", true);
	drawLC               = Drawings->CheckBox ("Draw Lance Clear Status", true);
	draw_Dmg             = Drawings->CheckBox ("Draw Damage", true);
	HPBarColor           = Drawings->AddColor ("Change Health Bar", 69, 64, 185, 100);

}

void Cassiopeia::OnGameUpdate()
{
	Automated();
	srand (time (NULL));
	if (!notified && GGame->Time() > 27 * 60)
	{
		notified = true;
		std::string noti = "Disable AA in Combo For";
		std::string noti2 = "Better Late Game DPS";
		GRender->Notification (Vec4 (52, 152, 219, 255), 10, noti.c_str());
		GRender->Notification (Vec4 (52, 152, 219, 255), 10, noti2.c_str());
	}

	if (!GUtility->IsLeagueWindowFocused() || GGame->IsChatOpen() || GGame->IsShopOpen())
	{
		return;
	}

	if (GOrbwalking->GetOrbwalkingMode() == kModeCombo)
	{
		TryMassUlt();
		if (laneMode)
		{
			Harass();
		}
		else
		{
			Combo();
		}
	}
	if (GOrbwalking->GetOrbwalkingMode() == kModeMixed)
	{
		Harass();
		LastHit();
	}
	if (GOrbwalking->GetOrbwalkingMode() == kModeLastHit)
	{
		LastHit();
	}
	if (GOrbwalking->GetOrbwalkingMode() == kModeLaneClear)
	{
		LaneClear();
	}
	if (GUtility->IsKeyDown (assistedUlt->GetInteger()))
	{
		CastAssistedUlt();
	}

	if (GUtility->IsKeyDown (FlashUlt->GetInteger()) && !GGame->IsChatOpen())
	{
		PerformFlashUlt();
	}
	if (GUtility->IsKeyDown (laneDom->GetInteger()) && !didiPress)
	{
		didiPress = true;
		GPluginSDK->DelayFunctionCall (300, [=]()
		{
			laneMode = !laneMode;
			didiPress = false;
		});

	}
	if (GUtility->IsKeyDown (burstKey->GetInteger()) && !didiPress)
	{
		didiPress = true;
		GPluginSDK->DelayFunctionCall (300, [=]()
		{
			burstMode = !burstMode;
			didiPress = false;
		});

	}
	if (GUtility->IsKeyDown (ComboAAkey->GetInteger()))
	{
		auto level = GEntityList->Player()->GetLevel();
		if (ComboAA->Enabled() && level >= ComboAALevel->GetInteger() && GEntityList->Player()->GetMana() > 100)
		{
			GOrbwalking->SetAttacksAllowed (false);
		}
	}
	if (!GUtility->IsKeyDown (ComboAAkey->GetInteger()) || GEntityList->Player()->GetMana() < 100)
	{
		{
			GOrbwalking->SetAttacksAllowed (true);
		}
	}


}
void Cassiopeia::OnRender()
{

	TryMassUlt();
	dmgDraw();
	if (!autoUlt->Enabled())
	{
		TryMassUlt();
	}
	Drawing();
	if (Laneclear->Enabled() && drawLC->Enabled())
	{
		Vec2 pos;
		if (GGame->Projection (Hero->GetPosition(), &pos))
		{
			GRender->DrawTextW (Vec2 (pos.x + 72, pos.y + 10), Vec4 (0, 255, 0, 255), "LANE CLEAR ON");
		}
	}
	if ( (burstMode || Hero->HealthPercent() < autoBurst->GetInteger()) && !laneMode)
	{
		Vec2 pos;
		if (GGame->Projection (Hero->GetPosition(), &pos))
		{
			GRender->DrawTextW (Vec2 (pos.x + 72, pos.y + 30), Vec4 (255, 0, 0, 255), "BURST MODE ON");
		}
	}
	if (laneMode)
	{
		Vec2 pos;
		if (GGame->Projection (Hero->GetPosition(), &pos))
		{
			GRender->DrawTextW (Vec2 (pos.x + 72, pos.y + 30), Vec4 (253, 116, 0, 255), "LANE DOM MODE ON");
		}
	}
}
void Cassiopeia::AntiGapclose (GapCloserSpell const& args)
{
	if (args.Source == nullptr || args.Source->IsDead() || strstr (args.Source->ChampionName(), "Zed") || strstr (args.Source->ChampionName(), "Master Yi") || !args.Source->IsEnemy (Hero))
	{
		return;
	}
	if (args.Source != nullptr)
	{
		if (W->IsReady() && gapcloseW->Enabled() && args.Source->IsValidTarget() && !GEntityList->Player()->IsDead() && args.Source->IsEnemy (GEntityList->Player()))
		{
			if (Extensions::GetDistance (Hero, args.EndPosition) > 450 && Extensions::GetDistance (Hero, args.EndPosition) < W->Range() && (Hero->HealthPercent() >gapcloseR->GetInteger() || !R->IsReady()))
			{
				auto delay = (GBuffData->GetEndTime (args.Data) - GGame->Time() - W->GetDelay());
				if (delay > 0)
				{
					GPluginSDK->DelayFunctionCall (delay, [=]()
					{
						W->CastOnPosition (args.EndPosition);
						return;
					});
				}
				else
				{
					W->CastOnPosition (args.EndPosition);
					return;
				}

			}
		}

		if (R->IsReady() && args.Source->IsValidTarget() && !Hero->IsDead() && args.Source->IsEnemy (Hero) && args.Source->IsFacing (Hero) && Hero->HealthPercent() <gapcloseR->GetInteger())
		{
			if (Extensions::GetDistance (Hero, args.EndPosition) < 350)
			{
				auto delay = (GBuffData->GetEndTime (args.Data) - GGame->Time() - R->GetDelay());
				if (delay > 0)
				{
					GPluginSDK->DelayFunctionCall (delay, [=]()
					{
						R->CastOnPosition (args.EndPosition);
						return;
					});
				}
				else
				{
					R->CastOnPosition (args.EndPosition);
					return;
				}

			}
		}
	}
}
Vec3 Cassiopeia::getPosToRflash (Vec3 target)
{
	return  Hero->GetPosition().Extend (GGame->CursorPosition(), Flash->Range());
}
void Cassiopeia::CastFlash()
{
	auto target = GTargetSelector->GetFocusedTarget() != nullptr
	              ? GTargetSelector->GetFocusedTarget()
	              : GTargetSelector->FindTarget (QuickestKill, SpellDamage, RFlash->Range());
	Flash->CastOnPosition (getPosToRflash (target->GetPosition()));
}
bool Cassiopeia::onMouseWheel (HWND wnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	if (!mouseClear->Enabled())
	{
		return true;
	}
	if (message != 0x20a)
	{
		return true;
	}
	Laneclear->UpdateInteger (!Laneclear->Enabled());
	return false;
}
void Cassiopeia::OnOrbwalkPreAttack (IUnit* Target)
{
	if (!Target->IsHero() && Hero->GetSpellLevel (kSlotE) > 0 && (GOrbwalking->GetOrbwalkingMode() == kModeLastHit || GOrbwalking->GetOrbwalkingMode() == kModeMixed))
	{
		if (Target != nullptr && Target->IsCreep())
		{
			GOrbwalking->DisableNextAttack();
		}
	}
}
float Cassiopeia::DPS (IUnit* target, bool dpsQ, bool dpsW, bool dpsE, bool dpsR, int eTimes)
{
	if (target == nullptr || !target->IsValidTarget() || target->IsDead() || !target->IsVisible())
	{
		return 0;
	}
	auto total = 0.f;
	if (dpsQ && Q->IsReady())
	{
		auto Level = Hero->GetSpellLevel (kSlotQ);
		double d[] = { 25, 40, 55, 70, 85 };
		total += GDamage->CalcMagicDamage (Hero, target, d[Level] + 0.2333 * Hero->BonusMagicDamage());
	}
	if (dpsW && W->IsReady())
	{
		auto Level = Hero->GetSpellLevel (kSlotW);
		double d[] = { 20, 35, 50, 65, 80 };
		total += GDamage->CalcMagicDamage (Hero, target, d[Level] + 0.15 * Hero->BonusMagicDamage());
	}
	if (dpsE && Hero->GetSpellLevel (kSlotE))
	{
		auto Level = Hero->GetSpellLevel (kSlotE);
		double d[] = { 10, 40, 70, 100, 130 };
		double flDamage = static_cast<float> ( (48 + 4 * Hero->GetLevel())) + 0.1 * Hero->BonusMagicDamage();

		//Poison Ticks
		flDamage += d[Level] + 0.35 * Hero->BonusMagicDamage();

		total += GDamage->CalcMagicDamage (Hero, target, flDamage) * eTimes;
	}
	if (dpsR && R->IsReady())
	{
		auto Level = Hero->GetSpellLevel (kSlotR);
		double d[] = { 150, 250, 350 };
		total += GDamage->CalcMagicDamage (Hero, target, d[Level] + 0.5 * Hero->TotalMagicDamage());
	}
	//Sheen
	if (GEntityList->Player()->HasItemId (3057))
	{
		total += GDamage->CalcPhysicalDamage (GEntityList->Player(), target, GEntityList->Player()->TotalPhysicalDamage() - GEntityList->Player()->BonusDamage());
	}
	//Lich Bane
	if (GEntityList->Player()->HasItemId (3100))
	{
		total += GDamage->CalcMagicDamage (GEntityList->Player(), target, 0.5 * GEntityList->Player()->TotalMagicDamage() + 0.75 * (GEntityList->Player()->TotalPhysicalDamage() - GEntityList->Player()->BonusDamage()));
	}
	// Ludens Echo
	if (GEntityList->Player()->HasBuff ("itemmagicshankcharge") && GBuffData->GetStacks ("itemmagicshankcharge") > 85)
	{
		total += GDamage->CalcMagicDamage (GEntityList->Player(), target, 100 + GEntityList->Player()->TotalMagicDamage() * 0.1);
	}

	total * Rembrandt::DamageModifierFromMasteries (Hero, target);
	return (float) total;
}

float Cassiopeia::WWidth()
{
	return 800 + W->Radius();
}

float Cassiopeia::WMaxRangeSqr()
{
	float WHalfWidth = WWidth() / 2.0f;
	return W->Range() * W->Range() + WHalfWidth*WHalfWidth;
}

float Cassiopeia::WMaxRange()
{
	return std::sqrt (WMaxRangeSqr());
}

bool Cassiopeia::IsInWRange (Vec2 position)
{
	return Extensions::GetDistanceSqr (GEntityList->Player()->ServerPosition().To2D(), position) < WMaxRangeSqr();
}

bool Cassiopeia::validRPos (IUnit* target, ISpell2* spell, Vec3& cast)
{
	if (Extensions::Validate (target) && !target->IsDead())
	{
		Vec3 futurePos;
		GPrediction->GetFutureUnitPosition (target, 0.4, true, futurePos);
		if (Extensions::GetDistanceSqr (futurePos,Hero->ServerPosition()) <R->Range() *R->Range())
		{
			cast = futurePos;
			return true;
		}

	}
	auto timeImmobile = Extensions::timeImmobile (target);
	if (timeImmobile >= spell->GetDelay())
	{
		cast = target->ServerPosition();
		return true;
	}
	return false;
}

std::vector<Vec2> Cassiopeia::rClose (std::vector< Vec3> pos)
{
	auto largeVec2s = std::vector<Vec2>();
	auto vec2s = std::vector<Vec2>();
	auto heroVec2 = Hero->GetPosition().To2D();

	for (auto i = 0; i < pos.size(); i++)
	{
		vec2s = std::vector<Vec2>();
		auto intial = pos[i].To2D();
		auto finalPos = intial - heroVec2;
		vec2s.push_back (intial);

		for (auto j = 0; j < pos.size(); j++)
		{
			if (i == j) { continue; }

			auto castAngle = Extensions::AngleBetweenEx (finalPos, pos[j].To2D() - heroVec2);
			if (castAngle > 0 && castAngle < 71)
			{
				vec2s.push_back (pos[j].To2D());
			}
		}

		if (vec2s.size() > largeVec2s.size())
		{
			largeVec2s = vec2s;
		}
	}

	return largeVec2s;
}
//shout out to lizzarin
bool Cassiopeia::GetBestPosition (std::vector<IUnit*> targets, std::vector<std::pair<Vec3, int>>& result, ISpell2* spell, Vec3 disPos = GEntityList->Player()->ServerPosition())
{
	std::vector<std::pair<Vec3, int>> Final;
	std::vector<IUnit*> closeHero = targets;
	std::vector<Vec3> castPos;

	for (auto x : closeHero)
	{
		Vec3 possiblePos;
		if (validRPos (x, spell, possiblePos))
		{
			if (Extensions::GetDistanceSqr (disPos, possiblePos) < R->Range() * R->Range())
			{
				castPos.push_back (possiblePos);
			}
		}
	}

	auto preds = rClose (castPos);

	if (preds.size() > 0)
	{
		auto final = Vec3();
		for (auto i = 0; i < preds.size(); i++)

		{
			final.x += preds[i].x;
			final.z += preds[i].y;
		}

		final.x /= preds.size() +1;
		final.z /= preds.size() +1;

		Final.push_back (std::make_pair (final, preds.size()));
		result = Final;
		return true;
	}
	return false;
}

bool Cassiopeia::OnPreCast (int Slot, IUnit* Target, Vec3* StartPosition, Vec3* EndPosition)
{
	if (Slot == kSlotR && assistedUlt->GetInteger() == 82 && !scriptR)
	{
		return false;
	}

	if (Slot == kSlotR && !scriptR && blockUlt->Enabled())
	{
		std::vector<IUnit*> all;
		for (auto targets : GEntityList->GetAllHeros (false, true))
		{
			if (Extensions::Validate (targets) && targets->IsVisible() && !targets->IsDead() && targets->IsHero() && !targets->IsInvulnerable() && !Extensions::HasSpellShield (targets))
			{
				all.push_back (targets);
			}
		}
		std::vector<std::pair<Vec3, int>> correctedPos;
		if (GetBestPosition (all, correctedPos,R))
		{
			for (auto x : correctedPos)
			{
				if (x.second)
				{
					scriptR = true;
					R->CastOnPosition (x.first);
					GPluginSDK->DelayFunctionCall (500, [=]()
					{
						scriptR = false;

					});
					return false;
				}
			}

		}
		else
		{
			return false;
		}
	}

	if (Slot == kSlotE && lhPriority->Enabled() && !Target->IsCreep() && lhAble)
	{
		return false;
	}
	return true;
}

void Cassiopeia::TryMassUlt()
{
	std::vector<IUnit*> facing;
	std::vector<IUnit*> all;
	for (auto targets : GEntityList->GetAllHeros (false, true))
	{
		if (Extensions::Validate (targets) && targets->IsVisible() && !targets->IsDead() && targets->IsHero() && !targets->IsInvulnerable() && !Extensions::HasSpellShield (targets))
		{

			all.push_back (targets);

			if (targets->IsFacing (Hero))
			{
				facing.push_back (targets);
			}
		}
	}
	std::vector<std::pair<Vec3, int>> bestPosFacing;
	if (GetBestPosition (facing, bestPosFacing, R))
	{
		for (auto x : bestPosFacing)
		{
			if (minFacing->GetInteger() <= x.second)
			{
				scriptR = true;
				GRender->DrawCircle (x.first, 50, Vec4 (255, 0, 0, 255));
				//	R->CastOnPosition (x.first);
				GPluginSDK->DelayFunctionCall (500, [=]()
				{
					scriptR = false;

				});

				return;
			}
		}
	}

	std::vector<std::pair<Vec3, int>> bestPos;

	if (GetBestPosition (all, bestPos, R))
	{
		for (auto y : bestPos)
		{

			if (minNotfacing->GetInteger() <= y.second)
			{
				scriptR = true;
				GRender->DrawCircle (y.first, 50, Vec4 (255, 0, 0, 255));
				//R->CastOnPosition (y.first);
				GPluginSDK->DelayFunctionCall (500, [=]()
				{
					scriptR = false;

				});

				return;

			}
		}
	}
}
void Cassiopeia::PerformFlashUlt()
{
	GGame->IssueOrder (Hero, kMoveTo, GGame->CursorPosition());
	if (E->IsReady() && Flash->IsReady())
	{
		auto target = GTargetSelector->GetFocusedTarget() != nullptr
		              ? GTargetSelector->GetFocusedTarget()
		              : GTargetSelector->FindTarget (QuickestKill, SpellDamage, RFlash->Range());
		if (target == nullptr || !target->IsHero() || target->IsDead() || !target->IsValidTarget())
		{
			return;
		}
		auto flashPosition = Hero->GetPosition().Extend (GGame->CursorPosition(), Flash->Range());
		std::vector<IUnit*> all;
		for (auto targets : GEntityList->GetAllHeros (false, true))
		{
			if (Extensions::Validate (targets) && targets->IsVisible() && !targets->IsDead() && targets->IsHero() && !targets->IsInvulnerable() && !Extensions::HasSpellShield (targets))
			{
				all.push_back (targets);
			}
		}
		std::vector<std::pair<Vec3, int>> correctedPos;
		if (GetBestPosition (all, correctedPos, RFlash, flashPosition))
		{
			for (auto x : correctedPos)
			{
				if (x.second)
				{
					scriptR = true;
					R->CastOnPosition (x.first);
					GPluginSDK->DelayFunctionCall (500 - (GGame->Latency()), [=]() { CastFlash(); });
					GPluginSDK->DelayFunctionCall (500, [=]()
					{
						scriptR = false;

					});

					return;
				}
			}
		}

	}
}
void Cassiopeia::CastAssistedUlt()
{
	std::vector<IUnit*> all;
	for (auto targets : GEntityList->GetAllHeros (false, true))
	{
		if (Extensions::Validate (targets) && targets->IsVisible() && !targets->IsDead() && targets->IsHero() && !targets->IsInvulnerable() && !Extensions::HasSpellShield (targets))
		{
			all.push_back (targets);
		}
	}
	std::vector<std::pair<Vec3, int>> correctedPos;
	if (GetBestPosition (all, correctedPos,R))
	{
		for (auto x : correctedPos)
		{
			if (x.second)
			{
				scriptR = true;
				R->CastOnPosition (x.first);
				GPluginSDK->DelayFunctionCall (500, [=]()
				{
					scriptR = false;

				});

				return;
			}
		}
	}

}
void Cassiopeia::OnDash (UnitDash* Args)
{
	if (Args->Source->IsHero() && Args->Source->IsEnemy (Hero))
	{
		if (Q->IsReady() && Args->Source->IsValidTarget() && !GEntityList->Player()->IsDead() && Args->Source->IsEnemy (GEntityList->Player()))
		{
			if (Extensions::GetDistance (GEntityList->Player(), Args->EndPosition) <= Q->Range())
			{
				auto delay = Args->EndTick - GGame->TickCount() - Q->GetDelay() * 1000;
				if (delay > 0)
				{
					GPluginSDK->DelayFunctionCall (delay, [=]()
					{
						Q->CastOnPosition (Args->EndPosition);

					});
				}
				else
				{
					Q->CastOnPosition (Args->EndPosition);
				}
			}
		}
	}

}
void Cassiopeia::CastWInterrupt (IUnit* target)
{

	if (target == nullptr || !target->IsValidTarget() || target->IsDead())
	{
		return;
	}
	Vec3 futurePos;
	GPrediction->GetFutureUnitPosition (target, W->GetDelay(), true, futurePos);
	if (IsInWRange (futurePos.To2D()))
		if (Hero->IsValidTarget (target, W->Range()))
		{
			W->CastOnTarget (target);
			return;
		}
		else
		{
			auto myPos = Hero->ServerPosition();
			auto myPos2D = myPos.To2D();
			auto targetPos = futurePos;
			auto targetPos2D = targetPos.To2D();
			float x = W->Range();
			float y = std::sqrt (Extensions::GetDistanceSqr (myPos, targetPos) - x*x);
			float z = Extensions::GetDistance (myPos, targetPos);
			float angle = (std::acos ( (y*y + z*z - x*x) / (2.0f * y * z))); // *PI / 180; //convert to radian
			auto direction = (myPos2D - targetPos2D).VectorNormalize().Rotated (angle);
			auto castPosition = Extensions::To3D (targetPos2D + y * direction);
			W->CastOnPosition (castPosition);
			return;
		}
}
void Cassiopeia::OnSpellCast (CastedSpell const& args)
{
	std::vector<std::string> SpellNames =
	{
		"Crowstorm",
		"SummonerTeleport",
		"ZacE",
		"ViQ",
		"TahmKenchNewR",
		"ShenR",
		"YasuoDashWrapper",
		"GalioE",
		"Destiny",
		"UrgotSwap2"
	};
	for (auto spellName : SpellNames)
	{
		if (strstr (std::string (args.Name_).c_str(), spellName.c_str()) && W->IsReady())
		{
			if (args.Caster_ != nullptr || args.Caster_->IsHero() && args.Caster_->IsEnemy (Hero) && Extensions::GetDistanceSqr (args.Caster_->ServerPosition(), Hero->ServerPosition()) <= WMaxRangeSqr() && !Hero->IsValidTarget (args.Caster_, 500))
			{
				CastWInterrupt (args.Caster_);
				break;
			}
		}
	}
}
bool Cassiopeia::GetImpactTime (ISpell2* spell, IUnit* source, IUnit* unit, float& impact)
{
	auto unitSpeed = unit->MovementSpeed();
	auto unitHitboxRadius = unit->BoundingRadius();
	//auto unitDirection = PathManager : GetDirection(unit, unit.path.curPath)
	auto unitPath = unit->GetWaypointList();
	//PathManager.paths[nID][unit.path.curPath]
	auto unitDirection = (unitPath[unitPath.size()] - (unit->ServerPosition())).VectorNormalize();
	//Calculations //
	auto ping = GGame->Latency() / 1000;
	auto delays = ping + spell->GetDelay();
	auto unitPosition = unit->ServerPosition();
	auto sourcePosition = source->ServerPosition();
	unitPosition = unitPosition + unitDirection * (unitSpeed * delays);
	unitPosition = unitPosition - unitDirection * unitHitboxRadius;
	auto toUnitDirection = (unitPosition - sourcePosition).VectorNormalize();
	sourcePosition = sourcePosition - toUnitDirection * source->BoundingRadius();
	auto theta = unitDirection * toUnitDirection;
	auto castDirection = unitDirection + toUnitDirection;
	unitPosition = unitPosition - castDirection * (theta * spell->Radius());
	auto unitDistance = Extensions::GetDistance (sourcePosition, unitPosition);
	//sourcePosition : dist(unitPosition)
	//Calculations //
	auto a = (unitSpeed * unitSpeed) - (spell->Speed() * spell->Speed());
	auto b = 2 * unitSpeed * unitDistance * theta;
	auto c = unitDistance * unitDistance;
	auto discriminant = b * b - 4 * a * c;
	if (discriminant < 0)
	{
		return false;
	}
	impact = 2 * c / (sqrt (discriminant) - b);
	if (impact < 0)
	{
		return false;
	}
	return true;
}
bool Cassiopeia::eCheck (IUnit* target)
{
	if (!Q->IsReady() || target->HasBuffOfType (BUFF_Poison) || GDamage->GetSpellDamage (Hero, target, kSlotE) * (min (4, (int) (Hero->GetMana() / E->ManaCost()))) > target->GetHealth())
	{
		return true;
	}
	else
	{
		return false;
	}
}
bool Cassiopeia::GetCastPosition (ISpell2* spell, IUnit* source, IUnit* unit, Vec3& cast)
{
	Vec3 castPosition;
	auto index = 0;
	auto path = unit->GetWaypointList();
	if (unit->IsMoving() && path.size() > 1)
	{
		for (auto pathIndex : path)
		{
			auto unitPosition = unit->ServerPosition();
			auto unitPath = pathIndex;

			auto unitDirection = (pathIndex - unitPosition).VectorNormalize();
			float impactTime;

			if (GetImpactTime (spell, source, unit, impactTime))
			{

				if (!impactTime)
				{
					return false;
				}
				castPosition = unitPosition + unitDirection * (unit->MovementSpeed() * impactTime);

				index++;
			}
			if (Extensions::GetDistance (source->ServerPosition(), castPosition) < spell->Range())
			{
				cast = castPosition;
				return true;
			}
		}

	}
	return false;
}
void Cassiopeia::CastW (IUnit* target)
{
	auto minWr = 450 - W->Radius();

	Vec3 futurePos;
	GPrediction->GetFutureUnitPosition (target, W->GetDelay(), true, futurePos);
	if (IsInWRange (futurePos.To2D()))
		if (Hero->IsValidTarget (target, WMaxRange()))
		{
			Vec3 castPos;
			if (GetCastPosition (W, Hero, target, castPos) && Extensions::GetDistanceSqr (Hero->ServerPosition(), castPos) > minWr*minWr)
			{
				W->CastOnPosition (castPos);
				return;
			}

		}
}
void Cassiopeia::qLogic (IUnit* target)
{
	if (target != nullptr)
	{
		//_lastQPoisonDelay = Game.Time + Q.Delay;
		//_lastQPoisonT = ts;
		Vec3 castPos;
		if (GetCastPosition (Q, Hero, target, castPos))
		{
			Q->CastOnPosition (castPos);
		}/*
		 if (Hero->IsChasing (target) && target->GetWaypointList().size() >= 1) // target is running
		 {
		 auto targetBehind = target->ServerPosition() + ( (target->ServerPosition() - Hero->ServerPosition()).VectorNormalize() * target->MovementSpeed() / 2) * (Extensions::GetDistance (Hero, target->ServerPosition()) / 1000);
		 Q->CastOnPosition (targetBehind);
		 return;
		 }*/
	}
}
void Cassiopeia::CastE (IUnit* target)
{
	auto lastE = 0;
	if (lastE < GGame->Time())
	{
		AdvPredictionOutput collision;
		E->RunPrediction (target, false, kCollidesWithYasuoWall, &collision);
		if (collision.HitChance != kHitChanceCollision)
		{
			if (E->CastOnTarget (target))
			{
				lastE = GGame->Time() + rand() % eDelay->GetInteger() + 1;
			}

		}
	}
}
void Cassiopeia::eLogic (IUnit* target)
{
	if (poisonTS->Enabled())
	{
		std::vector<IUnit*> all;
		std::vector<IUnit*> notPoisoned;
		IUnit* trueTarget;
		for (auto targets : GEntityList->GetAllHeros (false, true))
		{
			if (Extensions::Validate (targets) && targets->IsVisible() && !targets->IsDead() && targets->IsHero() && Extensions::GetDistanceSqr (Hero->ServerPosition(), targets->ServerPosition()) <= E->Range() *E->Range())
			{
				all.push_back (targets);
				if (!targets->HasBuffOfType (BUFF_Poison))
				{
					notPoisoned.push_back (targets);
				}
			}
		}

		if (notPoisoned.size() != all.size())
		{
			trueTarget = GTargetSelector->FindTargetEx (QuickestKill, SpellDamage, E->Range(), nullptr, true, &notPoisoned, nullptr);
		}
		else { trueTarget = nullptr; }

		if (poisonTS->Enabled() && Extensions::Validate (trueTarget) && trueTarget->IsVisible() && !trueTarget->IsDead() && trueTarget->IsHero() && !trueTarget->IsInvulnerable())
		{
			CastE (trueTarget);
		}
	}

	else if (target != nullptr && target->IsValidTarget() && target->IsVisible() && !target->IsDead() && target->IsHero() && !target->IsInvulnerable() && eCheck (target))
	{
		CastE (target);
	}
}
void Cassiopeia::OnInterrupt (InterruptibleSpell const& Args)
{

	if (Args.Source == nullptr || Args.Source->IsDead() || strstr (Args.Source->ChampionName(), "Zed") || strstr (Args.Source->ChampionName(), "Master Yi") || !Args.Source->IsEnemy (Hero) || Args.DangerLevel != kHighDanger)
	{
		return;
	}


	if (R->IsReady() && InterruptR->Enabled() && Hero->IsValidTarget (Args.Source, R->Range()) && Args.Source->IsFacing (Hero))
	{
		std::vector<IUnit*> all;
		for (auto targets : GEntityList->GetAllHeros (false, true))
		{
			if (Extensions::Validate (targets) && targets->IsVisible() && !targets->IsDead() && targets->IsHero() && !targets->IsInvulnerable() && !Extensions::HasSpellShield (targets))
			{
				all.push_back (targets);
			}
		}
		std::vector<std::pair<Vec3, int>> correctedPos;
		if (GetBestPosition (all, correctedPos,R))
		{
			for (auto x : correctedPos)
			{
				if (x.second)
				{
					scriptR = true;
					R->CastOnPosition (x.first);
					GPluginSDK->DelayFunctionCall (500, [=]()
					{
						scriptR = false;

					});
					break;
				}
			}

		}
	}
}
void Cassiopeia::Combo()
{
	auto q = ComboQ->Enabled() && Q->IsReady();
	auto w = ComboW->Enabled() && W->IsReady();
	auto e = ComboE->Enabled() && E->IsReady();
	auto r = ComboR->Enabled() && R->IsReady();
	auto qTarget = GTargetSelector->FindTarget (QuickestKill, SpellDamage, Q->Range() + 200);
	auto eTarget = GTargetSelector->FindTarget (QuickestKill, SpellDamage, E->Range());
	auto wTarget = GTargetSelector->FindTarget (QuickestKill, SpellDamage, W->Range());

	if (r && eTarget != nullptr  && eTarget->IsValidTarget() && eTarget->IsHero() && !eTarget->IsDead() && eTarget->IsFacing (Hero) && DPS (eTarget, true, true, true, true, min (3, (int) (Hero->GetMana() / E->ManaCost()))) > eTarget->GetHealth() && DPS (eTarget, true, true, true, false, min (2, (int) (Hero->GetMana() / E->ManaCost()))) < eTarget->GetHealth() && (minHealth->GetInteger() < eTarget->HealthPercent() || Hero->HealthPercent() < autoBurst->GetInteger() || burstMode))
	{
		scriptR = true;
		R->CastOnPosition (eTarget->ServerPosition());
		GPluginSDK->DelayFunctionCall (500, [=]()
		{
			scriptR = false;

		});
		return;
	}
	if (q)
	{
		if (qTarget != nullptr && qTarget->IsValidTarget() && qTarget->IsHero() && !qTarget->IsDead() && !qTarget->IsInvulnerable() && !Extensions::HasSpellShield (qTarget))
		{
			qLogic (qTarget);
		}

	}

	if (w)
	{

		GPluginSDK->DelayFunctionCall (1000, [=]()
		{
			if (wTarget != nullptr && wTarget->IsValidTarget() && wTarget->IsHero() && !wTarget->IsDead() && !wTarget->IsInvulnerable())
			{
				auto distance = (Hero->GetPosition() - wTarget->GetPosition()).Length();
				if (distance >= 400 && distance <= 800)
				{
					W->CastOnTarget (wTarget, kHitChanceHigh);
					return;
				}
			}
		});

	}


	if (e)
	{
		if (E->IsReady())
		{
			if (eTarget != nullptr && eTarget->IsValidTarget() && !eTarget->IsInvulnerable())
			{
				eLogic (eTarget);
				return;
			}
		}
	}

}
void Cassiopeia::Harass()
{

	auto q = harassQ->Enabled() && Q->IsReady() && Hero->ManaPercent() > HarassManaManager->GetInteger();
	auto w = harassW->Enabled() && W->IsReady() && Hero->ManaPercent() > HarassManaManager->GetInteger();
	auto e = harassE->Enabled() && E->IsReady() && Hero->ManaPercent() > HarassManaManager->GetInteger();
	auto qTarget = GTargetSelector->FindTarget (QuickestKill, SpellDamage, Q->Range());
	auto eTarget = GTargetSelector->FindTarget (QuickestKill, SpellDamage, E->Range());
	auto wTarget = GTargetSelector->FindTarget (QuickestKill, SpellDamage, W->Range());

	if (q)
	{
		if (qTarget != nullptr && qTarget->IsValidTarget() && !qTarget->IsInvulnerable() && !Extensions::HasSpellShield (qTarget))
		{
			Vec3 castPos;
			if (GetCastPosition (Q, GEntityList->Player(), qTarget, castPos) && Extensions::GetDistanceSqr (Hero->ServerPosition(), castPos) > 50 * 50)
			{
				Q->CastOnPosition (castPos);
			}
		}

	}

	if (w)
	{
		if (wTarget != nullptr && wTarget->IsValidTarget() && !wTarget->IsInvulnerable() && !Extensions::HasSpellShield (wTarget))
		{
			auto distance = (Hero->GetPosition() - wTarget->GetPosition()).Length();
			if (distance >= 400 && distance <= 800)
			{
				W->CastOnTarget (wTarget, kHitChanceHigh);
				return;
			}
		}
	}

	if (e)
	{
		if (E->IsReady())
		{
			if (eTarget != nullptr && eTarget->IsValidTarget() && !eTarget->IsInvulnerable())
			{
				eLogic (eTarget);
				return;
			}
		}
	}
}
bool Cassiopeia::FarmQ (Vec3 pos, ISpell2* spell, std::vector<std::pair<int, Vec2>>& result) //back line
{
	auto posChecked = 0;
	auto maxRange = R->Radius() + spell->Range();
	auto posRadius = spell->Radius() / 2;
	auto maxPosChecked = (int) (maxRange / posRadius);
	auto radiusIndex = 0;
	bool menu;
	std::vector<std::pair<int, Vec2>> HighestMinionCount;
	std::vector<std::pair<int, Vec2>> possibleQPositions;
	while (posChecked < maxPosChecked)
	{
		radiusIndex++;
		auto curRadius = radiusIndex * (0x00002 * posRadius);
		auto curCurcleChecks = static_cast<int> (ceil (0x2 * M_PI * curRadius / (0x2 * static_cast<double> (posRadius))));
		for (auto i = 0x1; i < curCurcleChecks; i++)
		{
			posChecked++;
			auto cRadians = 0x2 * M_PI / (curCurcleChecks - 0x1) * i;
			auto xPos = static_cast<float> (floor (pos.x + curRadius * cos (cRadians)));
			auto zPos = static_cast<float> (floor (pos.z + curRadius * sin (cRadians)));
			auto posFor2D = Vec2 (xPos, zPos);
			auto count = Extensions::CountMinionsInTargetRange (Extensions::To3D (posFor2D), W->Radius());
			//GGame->PrintChat(std::to_string(count).c_str());
			if (GNavMesh->IsPointWall (Extensions::To3D (posFor2D)))
			{
				// dont push is wall
				continue;
			}
			if (Extensions::Dist2D (posFor2D, GEntityList->Player()->ServerPosition()) > spell->Range())
			{
				// dont push to far away to cast;
				continue;
			}
			if (Extensions::Dist2D (posFor2D, pos) <= spell->Radius() * 4)
			{
				//	GGame->ShowPing(kPingAssistMe, To3D(posFor2D), false);
				possibleQPositions.push_back (std::make_pair (count, posFor2D));
			}
		}
	}
	std::sort (possibleQPositions.begin(), possibleQPositions.end(), [] (auto &left, auto &right)
	{
		return left.first > right.first;
	});
	//	for (auto entry : CloseQPositions) {
	//		Q->CastOnPosition(To3D(entry.second));
	//	}
	if (possibleQPositions.size() > 0)
	{
		HighestMinionCount.push_back (std::make_pair (possibleQPositions[0].first, possibleQPositions[0].second));
		result = HighestMinionCount;
		return true;
	}
	return false;

}
bool myCassfunctionHP (IUnit* i, IUnit* j) { return (i->GetHealth() >= j->GetHealth()); }
void Cassiopeia::LaneClear()
{
	if (Laneclear->Enabled())
	{
		{
			std::vector<std::pair<int, Vec2>> BestPos;
			std::vector<std::pair<int, Vec2>> BestPosRanged;
			std::vector<std::pair<int, Vec2>> FinalPos;
			std::vector<Vec3> allMinions;
			std::vector<IUnit*> allMinionsUnit;
			std::vector<Vec3> rangedMinions;
			for (auto minion : GEntityList->GetAllMinions (false, true, false))
			{
				if (minion != nullptr && !minion->IsWard() && minion->IsCreep() && Extensions::GetDistance (GEntityList->Player(), minion->ServerPosition()) <= Q->Range())
				{
					if (!minion->IsDead() && minion->PhysicalDamage() > 1);
					{
						allMinions.push_back (minion->GetPosition());
						allMinionsUnit.push_back (minion);
						if (Extensions::GetMinionType (minion) == kMinionNormal || Extensions::GetMinionType (minion) == kMinionSiege)
						{
							rangedMinions.push_back (minion->GetPosition());
						}
					}
				}
			}

			if (W->IsReady() && LaneClearW->Enabled() && Hero->ManaPercent() > LaneClearManaManager->GetInteger())
			{
				if (allMinions.size() >= LaneClearWMin->GetInteger())
				{
					for (auto x : allMinions)
					{
						std::vector<std::pair<int, Vec2>> result;
						if (FarmQ (x, W, result))
						{
							BestPos.push_back (result[0]);
						}
					}
					std::sort (BestPos.begin(), BestPos.end(), [] (auto &left, auto &right)
					{
						return left.first > right.first;
					});
					if (rangedMinions.size() > 0)
					{
						for (auto x : rangedMinions)
						{
							std::vector<std::pair<int, Vec2>> result2;
							if (FarmQ (x, W, result2))
							{
								BestPosRanged.push_back (result2[0]);
							}

						}
						std::sort (BestPosRanged.begin(), BestPosRanged.end(), [] (auto &left, auto &right)
						{
							return left.first > right.first;
						});
						FinalPos = BestPos[0].first > BestPosRanged[0].first + 1 ? BestPos : BestPosRanged;
					}
					else
					{
						FinalPos = BestPos;
					}
					//	auto finalPos = BestPos[0].first > BestPosRanged[0].first + 1 ? BestPos : BestPosRanged;
					if (FinalPos[0].first >= LaneClearWMin->GetInteger())
					{
						W->CastOnPosition (Extensions::To3D (FinalPos[0].second));
						return;
					}
				}
			}

			if (Q->IsReady() && LaneClearQ->Enabled() && Hero->ManaPercent() > LaneClearManaManager->GetInteger())
			{
				if (allMinions.size())
				{
					for (auto x : allMinions)
					{
						std::vector<std::pair<int, Vec2>> result;
						if (FarmQ (x, Q, result))
						{
							BestPos.push_back (result[0]);
						}

					}
					std::sort (BestPos.begin(), BestPos.end(), [] (auto &left, auto &right)
					{
						return left.first > right.first;
					});
					if (rangedMinions.size() > 0)
					{
						for (auto x : rangedMinions)
						{
							std::vector<std::pair<int, Vec2>> result2;
							if (FarmQ (x, Q, result2))
							{
								BestPosRanged.push_back (result2[0]);
							}
						}
						std::sort (BestPosRanged.begin(), BestPosRanged.end(), [] (auto &left, auto &right)
						{
							return left.first > right.first;
						});
						FinalPos = BestPos[0].first > BestPosRanged[0].first + 1 ? BestPos : BestPosRanged;
					}
					else
					{
						FinalPos = BestPos;
					}
					//	auto finalPos = BestPos[0].first > BestPosRanged[0].first + 1 ? BestPos : BestPosRanged;
					if (FinalPos[0].first >= LaneClearQMin->GetInteger())
					{
						Q->CastOnPosition (Extensions::To3D (FinalPos[0].second));
						return;
					}
				}
			}
			if (E->IsReady() && Hero->ManaPercent() > LaneClearManaManager->GetInteger())
			{
				for (auto pCreep : GEntityList->GetAllMinions (false, true, false))
				{
					if (GEntityList->Player()->IsValidTarget (pCreep, E->Range()))
					{
						if (pCreep->HasBuffOfType (BUFF_Poison))
						{
							if (E->CastOnTarget (pCreep))
							{
								return;
							}
						}
						else
						{
							//Stolen from null dad
							for (auto pCreep : GEntityList->GetAllMinions (false, true, true))
							{
								if (!GEntityList->Player()->IsValidTarget (pCreep, E->Range()))
								{
									continue;
								}

								float hp = pCreep->GetHealth();

								if (E->Speed() != 0.f)
								{
									int t =
									    static_cast<int> (E->GetDelay() * 1000) +
									    static_cast<int> ( (pCreep->ServerPosition() - GEntityList->Player()->GetPosition()).Length2D() * 1000) /
									    static_cast<int> (E->Speed()) - 125;

									hp = GHealthPrediction->GetPredictedHealth (pCreep, kLastHitPrediction, t, 0);
								}

								if (hp > 0 && hp < GDamage->GetSpellDamage (GEntityList->Player(), pCreep, kSlotE) - 20.f)
								{
									if (E->CastOnUnit (pCreep))
									{
										return;
									}
								}
							}
						}
					}
				}
			}
		}
		std::vector<IUnit*> allMobs;
		for (auto mob : GEntityList->GetAllMinions (false, false, true))
		{
			if (mob != nullptr && !mob->IsWard() && mob->IsJungleCreep() && mob->IsVisible() && Extensions::GetDistance (Hero, mob->ServerPosition()) <= Q->Range())
			{
				if (!mob->IsDead() && mob->PhysicalDamage() > 1)
				{
					allMobs.push_back (mob);

				}
			}
		}
		if (allMobs.size() >= 1 && Hero->ManaPercent() > LaneClearManaManager->GetInteger())
		{
			std::sort (allMobs.begin(), allMobs.end(), myCassfunctionHP);
			if (Q->IsReady() && LaneClearQ->Enabled())
			{
				Q->CastOnUnit (allMobs[0]);
			}

			if (E->IsReady())
			{
				E->CastOnTarget (allMobs[0]);
			}

		}
	}
}
void Cassiopeia::LastHit()
{

	if (E->IsReady())
	{
		//Stolen from null dad
		for (auto pCreep : GEntityList->GetAllMinions (false, true, true))
		{
			if (!GEntityList->Player()->IsValidTarget (pCreep, E->Range()))
			{
				continue;
			}

			float hp = pCreep->GetHealth();

			if (E->Speed() != 0.f)
			{
				int t =
				    static_cast<int> (E->GetDelay() * 1000) +
				    static_cast<int> ( (pCreep->ServerPosition() - GEntityList->Player()->GetPosition()).Length2D() * 1000) /
				    static_cast<int> (E->Speed()) - 125;

				hp = GHealthPrediction->GetPredictedHealth (pCreep, kLastHitPrediction, t, 0);
			}

			if (hp > 0 && hp < GDamage->GetSpellDamage (GEntityList->Player(), pCreep, kSlotE) - 20.f)
			{
				if (E->CastOnUnit (pCreep))
				{
					return;
				}
			}
		}
	}
	std::vector<std::pair<int, Vec2>> BestPos;
	std::vector<std::pair<int, Vec2>> BestPosRanged;
	std::vector<std::pair<int, Vec2>> FinalPos;
	std::vector<Vec3> allMinions;
	std::vector<IUnit*> allMinionsUnit;
	std::vector<Vec3> rangedMinions;
	for (auto minion : GEntityList->GetAllMinions (false, true, false))
	{
		if (minion != nullptr && !minion->IsWard() && minion->IsCreep() && Extensions::GetDistance (GEntityList->Player(), minion->ServerPosition()) <= Q->Range())
		{
			if (!minion->IsDead() && minion->PhysicalDamage() > 1);
			{
				allMinions.push_back (minion->GetPosition());
				allMinionsUnit.push_back (minion);
				if (Extensions::GetMinionType (minion) == kMinionNormal || Extensions::GetMinionType (minion) == kMinionSiege)
				{
					rangedMinions.push_back (minion->GetPosition());
				}
			}
		}
	}
	if (Q->IsReady() && lhQ->Enabled() && Hero->ManaPercent() > lhqMana->GetInteger())
	{
		if (allMinions.size())
		{
			for (auto x : allMinions)
			{
				std::vector<std::pair<int, Vec2>> result;
				if (FarmQ (x, Q, result))
				{
					BestPos.push_back (result[0]);
				}

			}
			std::sort (BestPos.begin(), BestPos.end(), [] (auto &left, auto &right)
			{
				return left.first > right.first;
			});
			if (rangedMinions.size() > 0)
			{
				for (auto x : rangedMinions)
				{
					std::vector<std::pair<int, Vec2>> result2;
					if (FarmQ (x, Q, result2))
					{
						BestPosRanged.push_back (result2[0]);
					}
				}
				std::sort (BestPosRanged.begin(), BestPosRanged.end(), [] (auto &left, auto &right)
				{
					return left.first > right.first;
				});
				FinalPos = BestPos[0].first > BestPosRanged[0].first + 1 ? BestPos : BestPosRanged;
			}
			else
			{
				FinalPos = BestPos;
			}
			//	auto finalPos = BestPos[0].first > BestPosRanged[0].first + 1 ? BestPos : BestPosRanged;
			if (FinalPos[0].first >= lhqMin->GetInteger())
			{
				Q->CastOnPosition (Extensions::To3D (FinalPos[0].second));
				return;
			}
		}
	}


}
void Cassiopeia::dmgDraw()
{
	if (draw_Dmg)
	{
		for (auto hero : GEntityList->GetAllHeros (false, true))
		{
			if (hero != nullptr && hero->IsValidTarget() && !hero->IsDead() && hero->IsOnScreen())
			{
				Vec4 BarColor;
				HPBarColor->GetColor (&BarColor);
				float totalDamage = DPS (hero, true, true, true, true, min (3, (int) (Hero->GetMana() / E->ManaCost())));
				Rembrandt::DrawDamageOnChampionHPBar (hero, totalDamage, BarColor);
				//GGame->PrintChat (std::to_string ( (int) totalDamage).c_str());
			}
		}
	}
}
void Cassiopeia::Automated()
{
	auto target = GTargetSelector->FindTarget (QuickestKill, SpellDamage, Q->Range() + 200);
	if (autoHarass->Enabled() && Q->IsReady() && Extensions::Validate (target))
	{
		qLogic (target);
	}

	for (auto targets : GEntityList->GetAllHeros (false, true))
	{
		if (targets != nullptr && Hero->IsValidTarget (targets, W->Range()) && !Hero->IsValidTarget (targets, 500) && !targets->IsDead())
		{
			AdvPredictionOutput prediction_output;
			W->RunPrediction (targets, true, kCollidesWithNothing, &prediction_output);
			if (prediction_output.HitChance == kHitChanceImmobile)
			{
				W->CastOnPosition (prediction_output.CastPosition);
			}
		}
		if (targets != nullptr && Hero->IsValidTarget (targets, Q->Range()) && !targets->IsDead() && targets == GOrbwalking->GetLastTarget())
		{
			AdvPredictionOutput prediction_output;
			Q->RunPrediction (targets, true, kCollidesWithNothing, &prediction_output);
			if (prediction_output.HitChance == kHitChanceImmobile)
			{
				Q->CastOnPosition (prediction_output.CastPosition);
			}
		}
	}


	if (GOrbwalking->GetOrbwalkingMode() == kModeNone && Q->IsReady() && Extensions::EnemiesInRange (Hero->ServerPosition(), 2000) == 0 && 95 < Hero->ManaPercent() && !Hero->IsRecalling() && stackT->Enabled())
	{
		bool close2Turret = false;
		auto turrets = GEntityList->GetAllTurrets (true, false);
		for (auto x : turrets)
		{
			if (Extensions::GetDistanceSqr (x->ServerPosition(), Hero->ServerPosition()) < 1000 * 1000)
			{
				close2Turret = true;

			}
		}

		if (close2Turret || Extensions::GetDistanceSqr (Hero->ServerPosition(), GEntityList->GetTeamNexus()->GetPosition()) < 3000 * 3000)
		{
			std::vector<int> slots;
			auto tear = false;
			int itemslot;

			for (auto i = 1; i <= 6; i++)
			{
				if ( (Hero->GetSpellBook()->IsValidSpell (i + 5)))
				{
					slots.push_back (i + 5);
				}
			}

			for (auto y : slots)
			{
				if (strstr (Hero->GetSpellBook()->GetName (y), "DummySpell") || strstr (Hero->GetSpellBook()->GetName (y), "ArchAngelsDummySpell"))
				{
					itemslot = (y);
					tear = true;

				}
			}

			if (tear && Hero->GetSpellRemainingCooldown (itemslot) <= 0)
			{
				Q->CastOnPosition (Hero->ServerPosition().Extend (GGame->CursorPosition(), 500));
			}

		}
	}

}
void Cassiopeia::Drawing()
{
	for (auto pCreep : GEntityList->GetAllMinions (false, true, true))
	{
		if (!GEntityList->Player()->IsValidTarget (pCreep, E->Range()))
		{
			continue;
		}

		float hp = pCreep->GetHealth();

		if (E->Speed() != 0.f)
		{
			int t =
			    static_cast<int> (E->GetDelay() * 1000) +
			    static_cast<int> ( (pCreep->ServerPosition() - GEntityList->Player()->GetPosition()).Length2D() * 1000) /
			    static_cast<int> (E->Speed()) - 125;

			hp = GHealthPrediction->GetPredictedHealth (pCreep, kLastHitPrediction, t, 0);
		}

		if (hp > 0 && hp < GDamage->GetSpellDamage (GEntityList->Player(), pCreep, kSlotE) - 20.f)
		{
			{
				lhAble = true;
				GRender->DrawCircle (pCreep->GetPosition(), 75, Vec4 (40, 240, 255, 200));
			}
		}
		else
		{
			lhAble = false;
		}
	}
	auto player = GEntityList->Player();
	if (DrawReady->Enabled())
	{
		if (Q->IsReady() && DrawQ->Enabled())
		{
			GRender->DrawOutlinedCircle (player->GetPosition(), Vec4 (0, 225, 0, 225), Q->Range());
		}
		if (W->IsReady() && DrawW->Enabled())
		{
			GRender->DrawCircle (GEntityList->Player()->GetPosition(), (550 + 800) / 2.f, Vec4 (53, 32, 59, 115), 450);
		}
	}
	else
	{
		if (DrawQ->Enabled())
		{
			GRender->DrawOutlinedCircle (player->GetPosition(), Vec4 (0, 225, 0, 225), Q->Range());
		}
		if (DrawW->Enabled())
		{
			GRender->DrawCircle (GEntityList->Player()->GetPosition(), (550 + 800) / 2.f, Vec4 (53, 32, 59, 115), 450);
		}
	}
}