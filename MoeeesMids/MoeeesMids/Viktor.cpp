#include "Viktor.h"
#include "Rembrandt.h"
#define M_PI 3.14159265358979323846

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
	Drawings = Parent->AddMenu ("All Drawings");
	MiscMenu = Parent->AddMenu ("Miscs");
	ComboQ = qMenu->CheckBox ("Use Q in Combo", true);
	harassQ = qMenu->CheckBox ("Harass with Q", true);
	harassQMana = qMenu->AddFloat (":: Only Harras Q if Mana >", 0, 100, 50);
	laneClearQ = qMenu->CheckBox ("Wave Clear with Q", true);
	laneClearQMana = qMenu->AddFloat (":: Only Wave Clear Q if Mana >", 0, 100, 30);
	ComboW = wMenu->CheckBox ("Use W in Combo", true);
	killStealW = wMenu->CheckBox ("Kill Steal with W", true);
	gapCloserW = wMenu->CheckBox ("W on Gap Closer", true);
	interrupterW = wMenu->CheckBox ("W on Interruptable Spells", true);
	ComboE = eMenu->CheckBox ("Use E in Combo", true);
	killStealE = eMenu->CheckBox ("Kill Steal with E", true);
	automaticE = eMenu->CheckBox ("Automatic Harass with E", true);
	harassE = eMenu->CheckBox ("Harass with E", true);
	harassEMana = eMenu->AddFloat (":: Only Harras E if Mana >", 0, 100, 40);
	laneClearE = eMenu->CheckBox ("Wave Clear with E", true);
	eMin = eMenu->AddInteger (":: Minimum Minions Hit", 1, 10, 3);
	laneClearEMana = eMenu->AddFloat (":: Only Wave Clear E if Mana >", 0, 100, 30);
	ComboR = rMenu->CheckBox ("Use R", true);
	ultMin = rMenu->AddInteger ("Ult if can hit more than: ", 1, 5, 3);
	RInterveral = rMenu->AddInteger ("Interval to Follow R in 100ms", 1, 10, 5);
	killStealR = rMenu->CheckBox ("KillSteal with R", true);
	rInterrupt = rMenu->CheckBox ("Interrupt Spells with R", true);
	Laneclear = MiscMenu->CheckBox ("Use Spells in Lane Clear", true);
	mouseClear = MiscMenu->CheckBox ("Mouse Scroll to Toggle Wave Clear", true);
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
	position = GEntityList->Player()->GetPosition();
	srand (time (NULL));
	rFollow = GTargetSelector->FindTarget (QuickestKill, SpellDamage, 1300);
	if (Extensions::Validate (rFollow) && rFollow->IsHero() && !rFollow->IsDead() && rFollow->IsVisible())
	{
		RFollowLogic (rFollow);
	}
	KillSteal();
	Automatic();
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
		for (auto minion : GEntityList->GetAllMinions (false, true, true))
		{
			if (minion != nullptr && !minion->IsWard() && minion->IsCreep() && Extensions::GetDistance (GEntityList->Player(), minion->GetPosition()) <= 1225)
			{
				if (!minion->IsDead())
				{
					allMinions.push_back (minion);
					if ( (minion->GetPosition().To2D() - GEntityList->Player()->GetPosition().To2D()).LengthSqr() <= E->Range() * E->Range())
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
			if (minion != nullptr && !minion->IsWard() && minion->IsJungleCreep() && Extensions::GetDistance (GEntityList->Player(), minion->GetPosition()) <= 1225)
			{
				if (!minion->IsDead())
				{
					allMinions.push_back (minion);
					if ( (minion->GetPosition().To2D() - GEntityList->Player()->GetPosition().To2D()).LengthSqr() <= E->Range() * E->Range())
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
		minionSources.push_back (minionsOne->GetPosition().To2D());
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
		auto startSource = minionsTwo->GetPosition().To2D();
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
				if (count >= Hits)
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
		R->CastOnPosition (args.Source->GetPosition());
	}
	if (W->IsReady() && interrupterW->Enabled() && Hero->IsValidTarget (args.Source, W->Range()) && args.Source != GEntityList->Player() && args.Source->IsEnemy (GEntityList->Player()) && args.DangerLevel >= kMediumDanger)
	{
		W->CastOnPosition (args.Source->GetPosition());
	}
}

void Viktor::OnOrbwalkPreAttack (IUnit* Target)
{
	if (Target->IsHero() && Q->IsReady() && !GEntityList->Player()->HasBuff ("ViktorPowerTransferReturn") && (GOrbwalking->GetOrbwalkingMode() == kModeCombo || GOrbwalking->GetOrbwalkingMode() == kModeMixed))
	{
		GOrbwalking->SetAttacksAllowed (false);
	}
	else
	{
		GOrbwalking->SetAttacksAllowed (true);
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
		if (Extensions::Validate (enemies) && !GEntityList->Player()->HasBuff ("ViktorPowerTransferReturn") && enemies->IsVisible() && !enemies->IsDead() && enemies->IsHero() && Extensions::GetDistance (Hero, enemies->GetPosition()) <= W->Radius() * 1.1f)
		{
			W->CastOnPosition (enemies->GetPosition());
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
		auto in_range = Extensions::GetDistanceSqr2D (target->GetPosition(), Hero->GetPosition()) <= E->Range() * E->Range();
		auto casted = false;
		Vec3 source1;
		std::vector<IUnit*> closeHero;
		std::vector<IUnit*> closerHero;
		std::vector<IUnit*> furtherHero;
		for (auto targets : GEntityList->GetAllHeros (false, true))
		{
			if (Extensions::Validate (targets) && targets->IsVisible() && !targets->IsDead() && targets->IsHero() && Extensions::GetDistanceSqr2D (Hero->GetPosition(), targets->GetPosition()) <= 1225*1225)
			{
				closeHero.push_back (targets);
				if (Extensions::GetDistanceSqr2D (targets->GetPosition(), Hero->GetPosition()) <= E->Range() * E->Range())
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
			if (minion != nullptr && !minion->IsWard() && minion->IsCreep() && Extensions::GetDistanceSqr2D (Hero->GetPosition(), minion->GetPosition()) <= 1225)
			{
				if (!minion->IsDead())
				{
					closeMinions.push_back (minion);
					if (Extensions::GetDistanceSqr2D (minion->GetPosition(), Hero->GetPosition()) <= E->Range() * E->Range())
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
			E->SetFrom (target->GetPosition() + (Hero->GetPosition() - target->GetPosition()).VectorNormalize() * (700 * 0.1f));
			AdvPredictionOutput outputfam;
			E->RunPrediction (target, true, kCollidesWithYasuoWall, &outputfam);
			E->SetFrom (Hero->GetPosition());
			if (Extensions::GetDistance (outputfam.CastPosition, Hero->GetPosition()) <= E->Range())
			{
				source1 = outputfam.CastPosition;
			}
			else
			{
				source1 = target->GetPosition();
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
					E->SetFrom (Hero->GetPosition());
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
			auto startPos = Hero->GetPosition() + (target->GetPosition() - Hero->GetPosition()).VectorNormalize() * 525;
			std::vector<IUnit*> targets1;
			for (auto enemies1 : closeHero)
			{
				if (Extensions::GetDistanceSqr2D (enemies1->GetPosition(), startPos) < startPosRad * startPosRad && Extensions::GetDistanceSqr2D (Hero->GetPosition(), enemies1->GetPosition()) < 525 * 525)
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
				source1 = targets1[0]->GetPosition();
			}
			else
			{
				std::vector<IUnit*> minionTargets;
				for (auto minions1 : closeMinions)
				{
					if (Extensions::GetDistanceSqr2D (minions1->GetPosition(), startPos) < startPosRad * startPosRad && Extensions::GetDistanceSqr2D (Hero->GetPosition(), minions1->GetPosition()) < 525 * 525)
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
					source1 = minionTargets[0]->GetPosition();
				}
				else
				{
					source1 = startPos;
				}
			}
			E->SetFrom (source1);
			E->SetOverrideRange (700);
			E->SetRangeCheckFrom (source1);
			AdvPredictionOutput output3;
			E->RunPrediction (target, true, kCollidesWithNothing, &output3);
			if (output3.HitChance >= kHitChanceHigh)
			{
				E->CastFrom (source1, output3.CastPosition);
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
	auto e = harassE->Enabled() && E->IsReady() && Hero->ManaPercent() >= harassEMana->GetFloat();
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
	if (automaticE->Enabled() && Extensions::Validate (eTarget) && eTarget->IsHero() && !eTarget->IsDead() && harassEMana->GetFloat() <= GEntityList->Player()->ManaPercent())
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
				W->CastFrom (prediction_output.CastPosition, Hero->GetPosition());
			}
		}
	}
}

void Viktor::OnDoCast (CastedSpell const& args)
{
	if (strcmp (args.Name_, "ViktorPowerTransfer") == 0)
	{
		GOrbwalking->SetAttacksAllowed (false);
		GPluginSDK->DelayFunctionCall (90 - GGame->Latency(), [=]()
		{
			GGame->IssueOrder (GEntityList->Player(), kMoveTo, Vec3 (position.x + rand() % 5 + 1, position.y, position.z + rand() % 5 + 1));
			GOrbwalking->SetAttacksAllowed (true);
			GGame->IssueOrder (GEntityList->Player(), kAttackTo, GOrbwalking->GetLastTarget());
			//	GGame->PrintChat (std::to_string (GGame->Latency()).c_str());
		});
	}
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

float Viktor::DPS (IUnit* target, bool dpsQ, bool checkQ, bool dpsE, bool dpsR)
{
	auto total = 0.f;
	auto ticks = 2; //Max ticks is 3 anyway zzz
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
		if (GEntityList->Player()->HasBuff ("viktorchaosstormtimer") && GBuffData->GetEndTime ("viktorchaosstormtimer") - GBuffData->GetStartTime ("viktorchaosstormtimer") > 3.5f)
		{
			total += rDmg (target, ticks);
		}
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
			auto xPos = static_cast<float> (floor (pos->GetPosition().x + curRadius * cos (cRadians)));
			auto zPos = static_cast<float> (floor (pos->GetPosition().z + curRadius * sin (cRadians)));
			auto posFor2D = Vec2 (xPos, zPos);
			auto count = Extensions::EnemiesInRange (Extensions::To3D (posFor2D), R->Radius());
			//GGame->PrintChat (std::to_string (count).c_str());
			if (GNavMesh->IsPointWall (Extensions::To3D (posFor2D)))
			{
				// dont push is wall
				continue;
			}
			if (Extensions::Dist2D (posFor2D, GEntityList->Player()->GetPosition()) >1225)
			{
				// dont push to far away to cast;
				continue;
			}
			if (Extensions::Dist2D (posFor2D, pos->GetPosition()) <= R->Radius() - 20)
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
			R->CastOnPosition (target->GetPosition());
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
	auto r = ComboR->Enabled() && R->IsReady();
	auto qTarget = GTargetSelector->FindTarget (QuickestKill, SpellDamage, Q->Range());
	auto eTarget = GTargetSelector->FindTarget (QuickestKill, SpellDamage, 1225);
	auto rTarget = GTargetSelector->FindTarget (QuickestKill, SpellDamage, R->Range());
	if (Extensions::Validate (qTarget) && qTarget->IsHero() && !qTarget->IsDead() && Extensions::Validate (eTarget) && eTarget->IsHero() && !eTarget->IsDead() && eTarget != qTarget && ( (eTarget->GetHealth() > DPS (eTarget, false, false, true, false)) || (eTarget->GetHealth() > DPS (eTarget, false, false, true, true) && eTarget == rTarget)) && qTarget->GetHealth() < DPS (qTarget, true, false, true, false))
	{
		eTarget = qTarget;
	}
	if (Extensions::Validate (rTarget) && rTarget->IsHero() && !rTarget->IsDead() && killStealR->Enabled() && r)
	{
		if (DPS (rTarget, true, false, true, false) < rTarget->GetHealth() && DPS (rTarget, true, false, true, true) > rTarget->GetHealth())
		{
			R->CastOnPosition (rTarget->GetPosition());
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
	auto rCast = false;
	if (r)
	{
		if (Extensions::Validate (rObject))
		{
			return;
		}
		if (Extensions::Validate (rTarget) && rTarget->IsHero() && !rTarget->IsDead())
		{
			if (GEntityList->Player()->HasBuff ("ViktorPowerTransferReturn") || qUsed || !q || !Q->IsReady() || DPS (rTarget, true, false, false, true) >= rTarget->GetHealth())
			{
				R->CastOnPosition (rTarget->GetPosition());
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
			Vec4 BarColor;
			HPBarColor->GetColor (&BarColor);
			float totalDamage = DPS (hero,true,false,true,true);
			Rembrandt::DrawDamageOnChampionHPBar (hero, totalDamage, BarColor);
		}
	}
}

void Viktor::KillSteal()
{
	if (killStealE->Enabled())
	{
		for (auto targets : GEntityList->GetAllHeros (false, true))
		{
			if (Extensions::Validate (targets) && targets->IsVisible() && !targets->IsDead() && targets->IsHero() && Extensions::GetDistance (Hero, targets->GetPosition()) <= 1225)
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
			if (mob != nullptr && !mob->IsWard() && mob->IsJungleCreep() && Extensions::GetDistance (Hero, mob->GetPosition()) <= 525)
			{
				if (!mob->IsDead())
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
		for (auto minion : GEntityList->GetAllMinions (false, true, false))
		{
			if (minion != nullptr && !minion->IsWard() && minion->IsCreep() && Extensions::GetDistance (Hero, minion->GetPosition()) <= 525)
			{
				if (!minion->IsDead())
				{
					if (Extensions::GetMinionType (minion) == kMinionSiege)
					{
						if (GDamage->GetSpellDamage (GEntityList->Player(), minion, kSlotQ) >= GHealthPrediction->GetPredictedHealth (minion, kLastHitPrediction, 0.2, 0))
						{
							Q->CastOnTarget (minion);
							break;
						}
					}
				}
			}
		}
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

