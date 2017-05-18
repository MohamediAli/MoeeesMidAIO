#include "Viktor.h"
#include "Rembrandt.h"
#include "Ahri.h"
#define M_PI 3.14159265358979323846
bool WaitForMove = true;

Viktor::~Viktor()
{
	ViktorMenu->Remove();
}

Viktor::Viktor (IMenu* Parent, IUnit* Hero) :Champion (Parent, Hero)
{
	Q = GPluginSDK->CreateSpell2 (kSlotQ, kTargetCast, false, false, kCollidesWithNothing);
	Q->SetSkillshot (0.15f, 0.f, 2050.f, 600.f + GEntityList->Player()->BoundingRadius() + 15); //+15 for bounding radius of target, find a better way to do this xD
	W = GPluginSDK->CreateSpell2 (kSlotW, kCircleCast, false, true, kCollidesWithNothing);
	W->SetSkillshot (1.0f, 200.f, FLT_MAX, 700.f);
	E = GPluginSDK->CreateSpell2 (kSlotE, kLineCast, false, true, kCollidesWithYasuoWall);
	E->SetSkillshot (0.30f, 80.f, 1050, 525.f);
	R = GPluginSDK->CreateSpell2 (kSlotR, kCircleCast, false, true, kCollidesWithNothing);
	R->SetSkillshot (0.25f, 300.f, FLT_MAX, 700.f);
	ViktorMenu = Parent->AddMenu ("Viktor Menu");
	qMenu = Parent->AddMenu ("Q Settings");
	wMenu = Parent->AddMenu ("W Settings");
	eMenu = Parent->AddMenu ("E Settings");
	rMenu = Parent->AddMenu ("R Settings");
	Prediction = Parent->AddMenu ("Prediction");
	MiscMenu = Parent->AddMenu ("Miscs");
	Drawings = Parent->AddMenu ("All Drawings");
	ComboQ = qMenu->CheckBox ("Use Q in Combo", true);
	harassQ = qMenu->CheckBox ("Harass with Q", true);
	harassQMana = qMenu->AddFloat (":: Only Harras Q if Mana >", 0, 100, 50);
	laneClearQ = qMenu->CheckBox ("Wave Clear with Q", true);
	laneClearQMana = qMenu->AddFloat (":: Only Wave Clear Q if Mana >", 0, 100, 30);
	ComboW = wMenu->CheckBox ("Use W in Combo", true);
	gapCloserW = wMenu->CheckBox ("W on Gap Closer", true);
	interrupterW = wMenu->CheckBox ("W on Interruptable Spells", true);
	ComboE = eMenu->CheckBox ("Use E in Combo", true);
	killstealing_e_ = eMenu->CheckBox ("Kill Steal with E", true);
	hunderAuto = eMenu->CheckBox ("Automatic Harass with E", false);
	harassmentE = eMenu->CheckBox ("Harass with E", true);
	harassEMana = eMenu->AddFloat (":: Only Harras E if Mana >", 0, 100, 40);
	laneClearE = eMenu->CheckBox ("Wave Clear with E", true);
	eMin = eMenu->AddInteger (":: Only E Minions Hit >", 1, 10, 2);
	laneClearEMana = eMenu->AddFloat (":: Only Wave Clear E if Mana >", 0, 100, 30);
	ComboR = rMenu->CheckBox ("Use R", true);
	ultMin = rMenu->AddInteger ("Ult if can hit >=: ", 1, 5, 3);
	RInterveral = rMenu->AddInteger ("Interval to Follow R in 100ms", 1, 10, 5);
	killStealR = rMenu->CheckBox ("Only ult if killable", true);
	rInterrupt = rMenu->CheckBox ("Interrupt Spells with R", true);
	PredType = { "Core", "Moeee's Pred", };
	PredictionType = Prediction->AddSelection ("Choose Prediction Type", 1, PredType);
	Laneclear = MiscMenu->CheckBox ("Use Spells in Lane Clear", true);
	mouseClear = MiscMenu->CheckBox ("Mouse Scroll to Toggle Wave Clear", true);
	Fleemode = MiscMenu->AddKey ("Flee Mode Key", 75);
	ComboAALevel = MiscMenu->AddInteger ("At what level disable AA", 1, 18, 6);
	ComboAA = MiscMenu->CheckBox ("Disable AA", false);
	ComboAAkey = MiscMenu->AddKey ("Disable key", 32);
	drawDmg = Drawings->CheckBox ("Draw Damage", true);
	HPBarColor = Drawings->AddColor ("Change Health Bar Color", 125, 0, 250, 200);
	drawLC = Drawings->CheckBox ("Draw Lance Clear Status", true);
	DrawReady = Drawings->CheckBox ("Draw Ready Spells", true);
	DrawQ = Drawings->CheckBox ("Draw Q", true);
	DrawW = Drawings->CheckBox ("Draw W", true);
	DrawE = Drawings->CheckBox ("Draw E", true);
	DrawR = Drawings->CheckBox ("Draw R", true);
}

void Viktor::OnGameUpdate()
{
	position = GEntityList->Player()->ServerPosition();
//	IUnit*  test = GTargetSelector->FindTarget (QuickestKill, SpellDamage, Q->Range());
	srand (time (NULL));
	rFollow = GTargetSelector->FindTarget (QuickestKill, SpellDamage, 1300);
	if (Extensions::Validate (rFollow) && rFollow->IsHero() && !rFollow->IsDead() && rFollow->IsVisible())
	{
		RFollowLogic (rFollow);
	}
	KillSteal();
	Automatic();
	autoE();
	if (GGame->IsChatOpen() || !GUtility->IsLeagueWindowFocused() || Hero->IsDead())
	{
		return;
	}
	if (GOrbwalking->GetOrbwalkingMode() == kModeCombo)
	{
		Combo();
	}
	if (GOrbwalking->GetOrbwalkingMode() == kModeMixed)
	{
		Harass();
	}
	if (GOrbwalking->GetOrbwalkingMode() == kModeLaneClear)
	{
		LaneClear();
		JungleClear();
	}
	if (GetAsyncKeyState (ComboAAkey->GetInteger()))
	{
		auto level = Hero->GetLevel();
		if (ComboAA->Enabled() && level >= ComboAALevel->GetInteger() && Hero->GetMana() > 100)
		{
			//		GGame->PrintChat (std::to_string (DPS (test, true, false, true, false)).c_str());
			GOrbwalking->SetAttacksAllowed (false);
		}
	}
	if (!GetAsyncKeyState (ComboAAkey->GetInteger()) || Hero->GetMana() < 100)
	{
		{
			GOrbwalking->SetAttacksAllowed (true);
		}
	}
	if (GetAsyncKeyState (Fleemode->GetInteger()))
	{
//		GGame->PrintChat (std::to_string (DPS (test, true, false, true, true)).c_str());
		FleeMode();
	}
}

void Viktor::OnPlayAnimation (IUnit* Source, std::string const AnimationId)
{
	if (Source == Hero)
	{
		if (strstr (AnimationId.c_str(), "b2f63868"))
		{
			GOrbwalking->SetAttacksAllowed (false);
			GPluginSDK->DelayFunctionCall (150 - GGame->Latency(), [=]()
			{
				CancelAnimation();
			});
		}

		if (strstr (AnimationId.c_str(), "2acd4eca") || strstr (AnimationId.c_str(), "9dd9dc06"))
		{
			WaitForMove = false;
		}
	}
}

void Viktor::CancelAnimation() //yol0 riven credits
{
	if (WaitForMove)
	{
		return;
	}
	WaitForMove = true;
	GGame->IssueOrder (Hero, kMoveTo, GGame->CursorPosition());

	GOrbwalking->SetAttacksAllowed (true);
	GGame->IssueOrder (Hero, kAttackUnit, GOrbwalking->GetLastTarget());

}



void Viktor::FleeMode()
{
	GGame->IssueOrder (Hero, kMoveTo, GGame->CursorPosition());
	if (Q->IsReady())
	{
		for (auto minion : GEntityList->GetAllMinions (false, true, true))
		{
			if (minion != nullptr && !minion->IsWard() && minion->IsCreep() && Extensions::GetDistance (GEntityList->Player(), minion->ServerPosition()) <= Q->Range())
			{
				if (!minion->IsDead() && minion->PhysicalDamage() > 1)
				{
					Q->CastOnTarget (minion);
					break;
				}
			}
		}
	}
}
void Viktor::OnRender()
{
	Drawing();
	dmgdraw();
	if (Laneclear->Enabled() && drawLC->Enabled())
	{
		Vec2 pos;
		if (GGame->Projection (Hero->GetPosition(), &pos))
		{
			GRender->DrawTextW (Vec2 (pos.x + 72, pos.y + 10), Vec4 (0, 255, 0, 255), "LANE CLEAR ON");
		}
	}
}

bool Viktor::onMouseWheel (HWND wnd, UINT message, WPARAM wparam, LPARAM lparam)
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

void Viktor::AntiGapclose (GapCloserSpell const& args)
{
	if (W->IsReady() && gapCloserW->Enabled() && args.Source->IsValidTarget() && !GEntityList->Player()->IsDead() && args.Source->IsEnemy (GEntityList->Player()))
	{
		if (Extensions::GetDistance (GEntityList->Player(), args.EndPosition) <= W->Range())
		{
			auto delay = (GBuffData->GetEndTime (args.Data) - GGame->Time() - W->GetDelay());
			GPluginSDK->DelayFunctionCall (delay * 1000, [=]()
			{
				W->CastOnPosition (args.EndPosition);
			});
		}
	}
}

float Viktor::LaserDistance (Vec2 point, Vec2 segmentStart, Vec2 segmentEnd, bool onlyIfOnSegment, bool squared)
{
	auto objects = Extensions::ProjectOn (point, segmentStart, segmentEnd);
	if (objects.IsOnSegment || onlyIfOnSegment == false)
	{
		return squared
		       ? Extensions::GetDistanceSqr (objects.SegmentPoint, point)
		       : Extensions::GetDistance (objects.SegmentPoint, point);
	}
	return FLT_MAX;
}

FarmLocationVik Viktor::FindBestLaserLineFarm (bool jg)
{
	Vec2 sourceOne;
	Vec2 sourceTwo;
	std::vector<IUnit*> allMinions;
	std::vector<IUnit*> sourceMinion;
	auto Hits = 0;
	auto minHits = eMin->GetInteger();
	if (!jg)
	{
		for (auto minion : GEntityList->GetAllMinions (false, true, false))
		{
			if (minion != nullptr && !minion->IsWard() && minion->IsCreep() && Extensions::GetDistance (GEntityList->Player(), minion->ServerPosition()) <= 1225)
			{
				if (!minion->IsDead() && minion->PhysicalDamage() > 1)
				{
					allMinions.push_back (minion);
					if ( (minion->ServerPosition().To2D() - GEntityList->Player()->ServerPosition().To2D()).LengthSqr() <= E->Range() * E->Range())
					{
						sourceMinion.push_back (minion);
					}
				}
			}
		}
	}
	else if (jg)
	{
		for (auto minion : GEntityList->GetAllMinions (false, false, true))
		{
			if (minion != nullptr && !minion->IsWard() && minion->IsJungleCreep() && Extensions::GetDistance (GEntityList->Player(), minion->ServerPosition()) <= 1225)
			{
				if (!minion->IsDead() && minion->PhysicalDamage() > 1)
				{
					allMinions.push_back (minion);
					if ( (minion->ServerPosition().To2D() - GEntityList->Player()->ServerPosition().To2D()).LengthSqr() <= E->Range() * E->Range())
					{
						sourceMinion.push_back (minion);
					}
				}
			}
		}
	}
	std::vector<Vec2>  minionSources;
	std::vector<Vec2>  posibleSources;
	for (auto minionsOne : allMinions)
	{
		minionSources.push_back (minionsOne->ServerPosition().To2D());
	}
	posibleSources.insert (posibleSources.end(), minionSources.begin(), minionSources.end());
	int maximum = posibleSources.size();
	for (auto i = 0; i < maximum; i++)
	{
		for (auto j = 0; j < maximum; j++)
		{
			if (posibleSources[j] != posibleSources[i])
			{
				posibleSources.push_back ( (posibleSources[j] + posibleSources[i]) / 2);
			}
		}
	}
	for (auto minionsTwo : sourceMinion)
	{
		auto startSource = minionsTwo->ServerPosition().To2D();
		for (auto source : posibleSources)
		{
			if (Extensions::GetDistanceSqr (source, startSource) <= 700 * 700)
			{
				auto endPos = startSource + 700 * (source - startSource).VectorNormalize();
				auto count = 0;
				for (auto minionVec2 : minionSources)
				{
					if (LaserDistance (minionVec2, startSource, endPos, false, true) <= 140 * 140)
					{
						count++;
					}
				}
				if (count >= Hits-1)
				{
					sourceTwo = endPos;
					Hits = count;
					sourceOne = startSource;
				}
			}
		}
	}
	if ( (!jg && minHits < Hits) || (jg && Hits > 0))
	{
		return  FarmLocationVik (sourceOne, sourceTwo, Hits);
	}
	else
	{
		return FarmLocationVik (sourceOne, sourceTwo, 0);
	}
}

void Viktor::OnInterrupt (InterruptibleSpell const& args)
{
	if (args.Source == nullptr || args.Source->IsDead() || !args.Source->IsHero() || !args.Source->IsEnemy (GEntityList->Player())) { return; }
	if (R->IsReady() && rInterrupt->Enabled() && Hero->IsValidTarget (args.Source, R->Range()) && args.Source != GEntityList->Player() && args.Source->IsEnemy (GEntityList->Player()) && args.DangerLevel > kMediumDanger)
	{
		R->CastOnPosition (args.Source->ServerPosition());
	}
	if (W->IsReady() && interrupterW->Enabled() && Hero->IsValidTarget (args.Source, W->Range()) && args.Source != GEntityList->Player() && args.Source->IsEnemy (GEntityList->Player()) && args.DangerLevel >= kMediumDanger)
	{
		W->CastOnPosition (args.Source->ServerPosition());
	}
}

void Viktor::OnOrbwalkPreAttack (IUnit* Target)
{
	if (Extensions::Validate (Target) && Target->IsHero() && Q->IsReady() && !GEntityList->Player()->HasBuff ("ViktorPowerTransferReturn") && (GOrbwalking->GetOrbwalkingMode() == kModeCombo || GOrbwalking->GetOrbwalkingMode() == kModeMixed))
	{
		GOrbwalking->DisableNextAttack();
	}
	if (Extensions::Validate (Target) && Target->IsCreep() && GEntityList->Player()->HasBuff ("ViktorPowerTransferReturn") && (GOrbwalking->GetOrbwalkingMode() == kModeLaneClear || GOrbwalking->GetOrbwalkingMode() == kModeMixed))
	{
		float hp = Target->GetHealth();

		if (Q->Speed() != 0.f)
		{
			int t =
			    static_cast<int> (Q->GetDelay() * 1000) +
			    static_cast<int> ( (Target->ServerPosition() - GEntityList->Player()->GetPosition()).Length2D() * 1000) /
			    static_cast<int> (Q->Speed()) - 125;

			hp = GHealthPrediction->GetPredictedHealth (Target, kLastHitPrediction, t, 0);
		}

		if (hp > 0 && hp > qDmg (Target, false) - 15.f)
		{
			GOrbwalking->DisableNextAttack();
		}
	}
}

void Viktor::OnCreate (IUnit* object)
{
	auto objectName = object->GetObjectName();
	if (strcmp (objectName, "Storm") == 0)
	{
		rObject = object;
	}
}

void Viktor::WLogic (IUnit* target)
{
	for (auto enemies : GEntityList->GetAllHeros (false, true))
	{
		if (Extensions::Validate (enemies) && !GEntityList->Player()->HasBuff ("ViktorPowerTransferReturn") && enemies->IsVisible() && !enemies->IsDead() && enemies->IsHero() && Extensions::GetDistance (Hero, enemies->ServerPosition()) <= W->Radius() * 1.1f)
		{
			W->CastOnPosition (enemies->ServerPosition());
		}
	}
	AdvPredictionOutput prediction_output;
	W->RunPrediction (target, false, kCollidesWithNothing, &prediction_output);
	if (!Q->IsReady() && !GEntityList->Player()->HasBuff ("ViktorPowerTransferReturn") && !E->IsReady() && (!R->IsReady() || Extensions::Validate (rObject)) && (target->HasBuffOfType (BUFF_Slow) || prediction_output.HitChance >= kHitChanceVeryHigh))
	{
		if (prediction_output.HitChance >= kHitChanceHigh)
		{
			W->CastOnPosition (prediction_output.CastPosition);
		}
	}
}

bool myfunctionHP (IUnit* i, IUnit* j) { return (i->GetHealth() >= j->GetHealth()); } //in dec order

void Viktor::eCast (IUnit* target)
{
	{
		auto in_range = Extensions::GetDistanceSqr2D (target->ServerPosition(), Hero->ServerPosition()) <= E->Range() * E->Range();
		auto casted = false;
		Vec3 source1;
		std::vector<IUnit*> closeHero;
		std::vector<IUnit*> closerHero;
		std::vector<IUnit*> furtherHero;
		for (auto targets : GEntityList->GetAllHeros (false, true))
		{
			if (Extensions::Validate (targets) && targets->IsVisible() && !targets->IsDead() && targets->IsHero() && Extensions::GetDistanceSqr2D (Hero->ServerPosition(), targets->ServerPosition()) <= 1225 * 1225)
			{
				closeHero.push_back (targets);
				if (Extensions::GetDistanceSqr2D (targets->ServerPosition(), Hero->ServerPosition()) <= E->Range() * E->Range())
				{
					closerHero.push_back (targets);
				}
				else
				{
					furtherHero.push_back (targets);
				}
			}
		}
		std::vector<IUnit*> closeMinions;
		std::vector<IUnit*> closerMinions;
		std::vector<IUnit*> furtherMinions;
		for (auto minion : GEntityList->GetAllMinions (false, true, true))
		{
			if (minion != nullptr && !minion->IsWard() && minion->IsCreep() && Extensions::GetDistanceSqr2D (Hero->ServerPosition(), minion->ServerPosition()) <= 1225)
			{
				if (!minion->IsDead() && minion->PhysicalDamage() > 1)
				{
					closeMinions.push_back (minion);
					if (Extensions::GetDistanceSqr2D (minion->ServerPosition(), Hero->ServerPosition()) <= E->Range() * E->Range())
					{
						closerMinions.push_back (minion);
					}
					else
					{
						furtherMinions.push_back (minion);
					}
				}
			}
		}
		if (in_range)
		{
			E->SetOverrideSpeed (1050 * 0.9f);
			E->SetFrom (target->ServerPosition() + (Hero->ServerPosition() - target->ServerPosition()).VectorNormalize() * (700 * 0.1f));
			AdvPredictionOutput outputfam;
			E->RunPrediction (target, true, kCollidesWithYasuoWall, &outputfam);
			E->SetFrom (Hero->ServerPosition());
			if (Extensions::GetDistance (outputfam.CastPosition, Hero->ServerPosition()) <= E->Range())
			{
				source1 = outputfam.CastPosition;
			}
			else
			{
				source1 = target->ServerPosition();
				E->SetOverrideSpeed (1050);
			}
			E->SetFrom (source1);
			E->SetRangeCheckFrom (source1);
			E->SetOverrideRange (700);
			if (closeHero.size() > 0)
			{
				std::vector<IUnit*> closeToPrediction;
				for (auto enemies : closeHero)
				{
					AdvPredictionOutput output;
					E->RunPrediction (enemies, true, kCollidesWithYasuoWall, &output);
					E->SetFrom (Hero->ServerPosition());
					if (output.HitChance >= kHitChanceHigh && Extensions::GetDistanceSqr2D (source1, output.CastPosition) < (E->Range() * E->Range()) * 0.8)
					{
						closeToPrediction.push_back (enemies);
					}
				}
				if (closeToPrediction.size() > 0)
				{
					if (closeToPrediction.size() > 1)
					{
						std::sort (closeToPrediction.begin(), closeToPrediction.end(), myfunctionHP);
					}
					AdvPredictionOutput output2;
					E->RunPrediction (closeToPrediction[0], true, kCollidesWithYasuoWall, &output2);
					auto source2 = output2.CastPosition;
					E->CastFrom (source1, source2);
					casted = true;
				}
			}
			if (!casted)
			{
				AdvPredictionOutput outputs;
				E->RunPrediction (target, true, kCollidesWithYasuoWall, &outputs);
				E->CastFrom (source1, outputs.CastPosition);
			}
			E->SetOverrideSpeed (1050);
			E->SetOverrideRange (525);
			E->SetRangeCheckFrom (Vec3 (0, 0, 0));
		}
		else
		{
			float startPosRad = 150;
			auto startPos = Hero->ServerPosition() + (target->ServerPosition() - Hero->ServerPosition()).VectorNormalize() * 525;
			std::vector<IUnit*> targets1;
			for (auto enemies1 : closeHero)
			{
				if (Extensions::GetDistanceSqr2D (enemies1->ServerPosition(), startPos) < startPosRad * startPosRad && Extensions::GetDistanceSqr2D (Hero->ServerPosition(), enemies1->ServerPosition()) < 525 * 525)
				{
					targets1.push_back (enemies1);
				}
			}
			if (targets1.size() > 0)
			{
				if (targets1.size() > 1)
				{
					std::sort (targets1.begin(), targets1.end(), myfunctionHP);
				}
				source1 = targets1[0]->ServerPosition();
			}
			else
			{
				std::vector<IUnit*> minionTargets;
				for (auto minions1 : closeMinions)
				{
					if (Extensions::GetDistanceSqr2D (minions1->ServerPosition(), startPos) < startPosRad * startPosRad && Extensions::GetDistanceSqr2D (Hero->ServerPosition(), minions1->ServerPosition()) < 525 * 525)
					{
						minionTargets.push_back (minions1);
					}
				}
				if (minionTargets.size() > 0)
				{
					if (minionTargets.size() > 1)
					{
						std::sort (minionTargets.begin(), minionTargets.end(), myfunctionHP);
					}
					source1 = minionTargets[0]->ServerPosition();
				}
				else
				{
					source1 = startPos;
				}
			}
			E->SetFrom (source1);
			E->SetOverrideRange (700);
			E->SetRangeCheckFrom (source1);
			if (PredictionType->GetInteger() == 0)
			{
				AdvPredictionOutput output3;
				E->RunPrediction (target, true, kCollidesWithYasuoWall, &output3);
				if (output3.HitChance >= kHitChanceHigh)
				{
					E->CastFrom (source1, output3.CastPosition);
				}
			}
			else if (PredictionType->GetInteger() == 1)
			{
				auto delay = Extensions::GetDistance (Hero, target) / 1225;
				Vec3 outputFuture;
				GPrediction->GetFutureUnitPosition (target, E->GetDelay() *delay, true, outputFuture);
				E->CastFrom (source1, outputFuture);
			}
			E->SetOverrideRange (525);
			E->SetFrom (Vec3 (0, 0, 0));
			E->SetRangeCheckFrom (Vec3 (0, 0, 0));
		}
	}
}

void Viktor::Harass()
{
	auto q = harassQ->Enabled() && Q->IsReady() && Hero->ManaPercent() >= harassQMana->GetFloat();
	auto e = harassmentE->Enabled() && E->IsReady() && Hero->ManaPercent() >= harassEMana->GetFloat();
	auto qTarget = GTargetSelector->FindTarget (QuickestKill, SpellDamage, Q->Range());
	auto eTarget = GTargetSelector->FindTarget (QuickestKill, SpellDamage, 1225);
	if (e)
	{
		if (Extensions::Validate (eTarget) && eTarget->IsHero() && !eTarget->IsDead())
		{
			eCast (eTarget);
			return;
		}
	}
	if (q)
	{
		if (Extensions::Validate (qTarget) && qTarget->IsHero() && !qTarget->IsDead() && qTarget->IsVisible())
		{
			Q->CastOnTarget (qTarget);
		}
	}
}

void Viktor::autoE()
{
	auto eTarget = GTargetSelector->FindTarget (QuickestKill, SpellDamage, 1225);
	if (hunderAuto->Enabled() && Extensions::Validate (eTarget) && eTarget->IsHero() && !eTarget->IsDead() && harassEMana->GetFloat() <= GEntityList->Player()->ManaPercent())
	{
		eCast (eTarget);
	}
}

void Viktor::Automatic()
{
	for (auto target : GEntityList->GetAllHeros (false, true))
	{
		if (Hero->IsValidTarget (target, W->Range()) && target != nullptr && !target->IsDead())
		{
			AdvPredictionOutput prediction_output;
			W->RunPrediction (target, true, kCollidesWithNothing, &prediction_output);
			if (prediction_output.HitChance == kHitChanceImmobile)
			{
				W->CastFrom (prediction_output.CastPosition, Hero->ServerPosition());
			}
		}
	}
}

void Viktor::OnDoCast (CastedSpell const& args)
{

}

float Viktor::qDmg (IUnit* Target, bool stage)
{
	auto Level = (GEntityList->Player()->GetSpellLevel (kSlotQ));
	if (stage)
	{
		double d[] = { 60, 80, 100, 120, 140 };
		return GDamage->CalcMagicDamage (GEntityList->Player(), Target, d[Level] + 0.4 * GEntityList->Player()->BonusMagicDamage());
	}
	else
	{
		double d[] = { 20, 40, 60, 80, 100 };
		return GDamage->CalcMagicDamage (GEntityList->Player(), Target, d[Level] + 0.5 * GEntityList->Player()->BonusMagicDamage() + (1 * (GEntityList->Player()->PhysicalDamage() + GEntityList->Player()->BonusDamage())));
	}
}

float Viktor::eDmg (IUnit* Target, bool augment)
{
	double d[] = { 70, 110, 150, 190, 230 };
	double e[] = { 20, 60, 100, 140, 180 };
	auto Level = (GEntityList->Player()->GetSpellLevel (kSlotE));
	auto init = d[Level] + 0.5 * GEntityList->Player()->BonusMagicDamage();
	auto second = e[Level] + 0.70 * GEntityList->Player()->BonusMagicDamage();
	if (augment)
	{
		return GDamage->CalcMagicDamage (GEntityList->Player(), Target, init + second);
	}
	else
	{
		return GDamage->CalcMagicDamage (GEntityList->Player(), Target, init);
	}
}

float Viktor::rDmg (IUnit* Target, int ticks)
{
	double d[] = { 100, 175, 250 };
	double e[] = { 150, 250, 350 };
	auto Level = (GEntityList->Player()->GetSpellLevel (kSlotR));
	auto init = d[Level] + 0.55 * GEntityList->Player()->BonusMagicDamage();
	auto tick = e[Level] + 0.6 *  GEntityList->Player()->BonusMagicDamage();
	return GDamage->CalcMagicDamage (GEntityList->Player(), Target, init + tick*ticks);
}

float Viktor::DPS (IUnit* target, bool dpsQ, bool checkQ, bool dpsE, bool dpsR, int rTicks)
{
	auto total = 0.f;
	auto inQRange = checkQ && Extensions::GetDistance (GEntityList->Player(), target) <= GOrbwalking->GetAutoAttackRange (GEntityList->Player()) || checkQ == false;
	if (dpsQ && Q->IsReady() && inQRange)
	{
		total += qDmg (target, true);
	}
	if (dpsQ && !Q->IsReady() && GEntityList->Player()->HasBuff ("ViktorPowerTransferReturn") && inQRange)
	{
		total += qDmg (target, false);
	}
	if (dpsE && E->IsReady())
	{
		if (GEntityList->Player()->HasBuff ("viktoreaug") || GEntityList->Player()->HasBuff ("viktorqeaug") || GEntityList->Player()->HasBuff ("viktorqweaug") || GEntityList->Player()->HasBuff ("viktorweaug"))
		{
			total += eDmg (target, true);
		}
		else
		{
			total += eDmg (target, false);
		}
	}
	if (dpsR && R->IsReady())
	{
		total += rDmg (target, rTicks);
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
	return (float) total;
	//credit lizzarin for most of logic here
}

Vec3 Viktor::TeamFightR (IUnit* pos)
{
	auto posChecked = 0;
	auto maxRange = R->Radius() + R->Range();
	auto posRadius = 45;
	auto maxPosChecked = (int) (maxRange / posRadius);
	auto radiusIndex = 0;
	std::vector<std::pair<int, Vec2>> possibleQPositions;
	while (posChecked < maxPosChecked)
	{
		radiusIndex++;
		auto curRadius = radiusIndex * (0x2 * posRadius);
		auto curCurcleChecks = static_cast<int> (ceil ( (0x2 * M_PI * curRadius) / (0x2 * static_cast<double> (posRadius))));
		for (auto i = 1; i < curCurcleChecks; i++)
		{
			posChecked++;
			auto cRadians = (0x2 * M_PI / (curCurcleChecks - 1)) * i;
			auto xPos = static_cast<float> (floor (pos->ServerPosition().x + curRadius * cos (cRadians)));
			auto zPos = static_cast<float> (floor (pos->ServerPosition().z + curRadius * sin (cRadians)));
			auto posFor2D = Vec2 (xPos, zPos);
			auto count = Extensions::EnemiesInRange (Extensions::To3D (posFor2D), R->Radius());
			//GGame->PrintChat (std::to_string (count).c_str());
			if (GNavMesh->IsPointWall (Extensions::To3D (posFor2D)))
			{
				// dont push is wall
				continue;
			}
			if (Extensions::Dist2D (posFor2D, GEntityList->Player()->ServerPosition()) >1225)
			{
				// dont push to far away to cast;
				continue;
			}
			if (Extensions::Dist2D (posFor2D, pos->ServerPosition()) <= R->Radius() - 20)
			{
				//	GGame->ShowPing (kPingAssistMe, Extensions::To3D (posFor2D), false);
				possibleQPositions.push_back (std::make_pair (count, posFor2D));
			}
		}
	}
	std::sort (possibleQPositions.begin(), possibleQPositions.end(), [] (auto &left, auto &right)
	{
		return left.first > right.first;
	});
	for (auto entry : possibleQPositions)
	{
		if (entry.first >= ultMin->GetInteger())
		{
			//R->CastOnPosition (Extensions::To3D (entry.second));
			return (Extensions::To3D (entry.second));
		}
	}
	return Vec3 (0, 0, 0);
}

void Viktor::RFollowLogic (IUnit* target)

{
	if (lastRMoveTick + 5 * 100 > GGame->TickCount() || GEntityList->Player()->IsWindingUp())
	{
		return;
	}
	if (Extensions::Validate (rObject))
	{
		lastRMoveTick = GGame->TickCount();
		if (TeamFightR (target) != Vec3 (0, 0, 0))
		{
			R->CastOnPosition (TeamFightR (target));
		}
		else
		{
			R->CastOnPosition (target->ServerPosition());
		}
	}
}

void Viktor::Combo()
{
	//lizarin combo logic-ish
	auto qUsed = false;
	auto q = ComboQ->Enabled() && Q->IsReady();
	auto w = ComboW->Enabled() && W->IsReady();
	auto e = ComboE->Enabled() && E->IsReady();
	auto rKS = ComboR->Enabled() && R->IsReady() && killStealR->Enabled();
	auto r = ComboR->Enabled() && R->IsReady() && !killStealR->Enabled();
	auto qTarget = GTargetSelector->FindTarget (QuickestKill, SpellDamage, Q->Range());
	auto eTarget = GTargetSelector->FindTarget (QuickestKill, SpellDamage, 1225);
	auto rTarget = GTargetSelector->FindTarget (QuickestKill, SpellDamage, R->Range());
	if (Extensions::Validate (qTarget) && qTarget->IsHero() && !qTarget->IsDead() && Extensions::Validate (eTarget) && eTarget->IsHero() &&
	    !eTarget->IsDead() && eTarget != qTarget && ( (eTarget->GetHealth() > DPS (eTarget, false, false, true, false)) ||
	            (eTarget->GetHealth() > DPS (eTarget, false, false, true, true) && eTarget == rTarget)
	            && qTarget->GetHealth() < DPS (qTarget, true, false, true, false)))
	{
		eTarget = qTarget;
	}
	if (Extensions::Validate (rTarget) && rTarget->IsHero() && !rTarget->IsDead() && rKS)
	{
		if (Extensions::Validate (rObject))
		{
			return;
		}
		if (DPS (rTarget, true, false, true, true, 2) > rTarget->GetHealth() && DPS (rTarget, true, false, true, false) < rTarget->GetHealth())
		{
			R->CastOnPosition (rTarget->ServerPosition());
			return;
		}
	}
	if (e)
	{
		if (Extensions::Validate (eTarget) && eTarget->IsHero() && !eTarget->IsDead())
		{
			eCast (eTarget);
			return;
		}
	}
	if (q)
	{
		if (Extensions::Validate (qTarget) && qTarget->IsHero() && !qTarget->IsDead() && qTarget->IsVisible())
		{
			if (Q->CastOnTarget (qTarget))
			{
				qUsed = true;
			}
		}
	}
	if (w)
	{
		if (Extensions::Validate (rTarget) && rTarget->IsHero() && !rTarget->IsDead())
		{
			WLogic (rTarget);
		}
	}
	if (r)
	{
		if (Extensions::Validate (rObject))
		{
			return;
		}
		if (Extensions::Validate (rTarget) && rTarget->IsHero() && !rTarget->IsDead())
		{
			if (GEntityList->Player()->HasBuff ("ViktorPowerTransferReturn") || qUsed || !q || !Q->IsReady())
			{
				R->CastOnPosition (rTarget->ServerPosition());
			}
			else if (Extensions::GetDistance (GEntityList->Player(), TeamFightR (rTarget)) <= R->Range() && TeamFightR (rTarget) != Vec3 (0, 0, 0))
			{
				R->CastOnPosition (TeamFightR (rTarget));
			}
		}
	}
}

bool Viktor::jgLaser()
{
	auto farmLoc = FindBestLaserLineFarm (true);
	if (farmLoc.MinionsHit > 0)
	{
		E->CastFrom (Extensions::To3D (farmLoc.Position1), Extensions::To3D (farmLoc.Position2));
		return true;
	}
	return false;
}

bool Viktor::minionLaser()
{
	auto farmLoc = FindBestLaserLineFarm (false);
	if (farmLoc.MinionsHit > 0)
	{
		E->CastFrom (Extensions::To3D (farmLoc.Position1), Extensions::To3D (farmLoc.Position2));
		return true;
	}
	return false;
}

void Viktor::dmgdraw()
{
	if (drawDmg->Enabled())
	{
		for (auto hero : GEntityList->GetAllHeros (false, true))
		{
			if (Extensions::Validate (hero) && !hero->IsDead() && hero->IsOnScreen())
			{
				Vec4 BarColor;
				HPBarColor->GetColor (&BarColor);
				float totalDamage = DPS (hero, true, false, true, true);
				Rembrandt::DrawDamageOnChampionHPBar (hero, totalDamage, BarColor);
			}
		}
	}
}

void Viktor::KillSteal()
{
	if (killstealing_e_->Enabled())
	{
		for (auto targets : GEntityList->GetAllHeros (false, true))
		{
			if (Extensions::Validate (targets) && targets->IsVisible() && !targets->IsDead() && targets->IsHero() && Extensions::GetDistance (Hero, targets->ServerPosition()) <= 1225)
			{
				if (DPS (targets, false, false, true, false) >= targets->GetHealth())
				{
					eCast (targets);
					break;
				}
			}
		}
	}
}

void Viktor::JungleClear()
{
	if (!Laneclear->Enabled())
	{
		return;
	}
	std::vector<IUnit*> allMobs;
	if (laneClearE->Enabled() && E->IsReady() && Hero->ManaPercent() >= laneClearEMana->GetFloat())
	{
		jgLaser();
	}
	if (laneClearQ->Enabled() && Q->IsReady() && Hero->ManaPercent() >= laneClearQMana->GetFloat())
	{
		for (auto mob : GEntityList->GetAllMinions (false, false, true))
		{
			if (mob != nullptr && !mob->IsWard() && mob->IsJungleCreep() && mob->IsVisible() && Extensions::GetDistance (Hero, mob->ServerPosition()) <= 525)
			{
				if (!mob->IsDead() && mob->PhysicalDamage() > 1)
				{
					allMobs.push_back (mob);
				}
			}
		}
		if (allMobs.size() >= 1)
		{
			std::sort (allMobs.begin(), allMobs.end(), myfunctionHP);
			Q->CastOnUnit (allMobs[0]);
		}
	}
}

void Viktor::LaneClear()
{
	if (!Laneclear->Enabled())
	{
		return;
	}
	if (laneClearE->Enabled() && E->IsReady() && Hero->ManaPercent() >= laneClearEMana->GetFloat())
	{
		minionLaser();
	}
	if (laneClearQ->Enabled() && Q->IsReady() && Hero->ManaPercent() >= laneClearQMana->GetFloat())
	{
		for (auto pCreep : GEntityList->GetAllMinions (false, true, true))
		{
			if (!GEntityList->Player()->IsValidTarget (pCreep, Q->Range()))
			{
				continue;
			}

			float hp = pCreep->GetHealth();

			if (Q->Speed() != 0.f)
			{
				int t =
				    static_cast<int> (Q->GetDelay() * 1000) +
				    static_cast<int> ( (pCreep->ServerPosition() - GEntityList->Player()->GetPosition()).Length2D() * 1000) /
				    static_cast<int> (Q->Speed()) - 125;

				hp = GHealthPrediction->GetPredictedHealth (pCreep, kLastHitPrediction, t, 0);
			}

			if (hp > 0 && hp < (qDmg (pCreep, true) - 15.f))
			{
				if (Q->CastOnUnit (pCreep))
				{
					return;
				}
			}
		}
		/*
		for (auto minion : GEntityList->GetAllMinions (false, true, false))
		{
			if (minion != nullptr && !minion->IsWard() && minion->IsCreep() && Extensions::GetDistance (Hero, minion->ServerPosition()) <= 525)
			{
				if (!minion->IsDead() && minion->PhysicalDamage() > 1)
				{
					if (qDmg (minion,true) + qDmg (minion,false) >= minion->GetHealth())
					{
						GOrbwalking->SetAttacksAllowed (false);
						Q->CastOnTarget (minion);
						GOrbwalking->SetAttacksAllowed (true);
						GOrbwalking->SetOverrideTarget (minion);

						break;
					}
				}
			}
		}*/
	}
}



void Viktor::Drawing()
{
	if (DrawReady->Enabled())
	{
		if (Q->IsReady() && DrawQ->Enabled())
		{
			GRender->DrawCircle (Hero->GetPosition(), Q->Range(), Vec4 (0, 225, 0, 225));
		}
		if (W->IsReady() && DrawW->Enabled())
		{
			GRender->DrawCircle (Hero->GetPosition(), W->Range(), Vec4 (0, 225, 0, 225));
		}
		if (E->IsReady() && DrawE->Enabled())
		{
			GRender->DrawCircle (Hero->GetPosition(), 1225, Vec4 (0, 225, 0, 225));
		}
	}
	else
	{
		if (DrawQ->Enabled())
		{
			GRender->DrawCircle (Hero->GetPosition(), Q->Range(), Vec4 (0, 225, 0, 225));
		}
		if (DrawW->Enabled())
		{
			GRender->DrawCircle (Hero->GetPosition(), W->Range(), Vec4 (0, 225, 0, 225));
		}
		if (DrawE->Enabled())
		{
			GRender->DrawCircle (Hero->GetPosition(), 1225, Vec4 (0, 225, 0, 225));
		}
	}
}
