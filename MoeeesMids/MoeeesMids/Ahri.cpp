#include "Ahri.h"
#include "Extensions.h"
#include "Rembrandt.h"


Ahri::~Ahri()
{
	AhriMenu->Remove();
}


Ahri::Ahri (IMenu* Parent, IUnit* Hero) :Champion (Parent, Hero)
{
	Q = GPluginSDK->CreateSpell2 (kSlotQ, kLineCast, false, false, kCollidesWithNothing);
	Q->SetSkillshot (0.25f, 80, 1450, 840);
	W = GPluginSDK->CreateSpell2 (kSlotW, kTargetCast, false, false, kCollidesWithNothing);
	W->SetOverrideRange (580);
	E = GPluginSDK->CreateSpell2 (kSlotE, kLineCast, false, false, static_cast<eCollisionFlags> (kCollidesWithMinions | kCollidesWithYasuoWall));
	E->SetSkillshot (0.25f, 60, 1550, 950);
	EFlash = GPluginSDK->CreateSpell2 (kSlotE, kLineCast, false, false, static_cast<eCollisionFlags> (kCollidesWithMinions | kCollidesWithYasuoWall));
	EFlash->SetSkillshot (0.25f, 60, 3100, 1350);
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
	AhriMenu = Parent->AddMenu ("Ahri Menu");
	ComboMenu = Parent->AddMenu ("Combo");
	HarassMenu = Parent->AddMenu ("Harass");
	LaneClearMenu = Parent->AddMenu ("LaneClear");
	ksSettings = Parent->AddMenu ("Kill Steal");
	Prediction = Parent->AddMenu ("Prediction");
	Drawings = Parent->AddMenu ("Spell Drawings");
	MiscMenu = Parent->AddMenu ("Miscs");
	ComboQ = ComboMenu->CheckBox ("Use Q", true);
	ComboW = ComboMenu->CheckBox ("Use W", true);
	ComboE = ComboMenu->CheckBox ("Use E", true);
	HarassQ = HarassMenu->CheckBox ("Use Q", true);
	autoQ = HarassMenu->CheckBox ("Use Q Automatically", false);
	HarassW = HarassMenu->CheckBox ("Use W", true);
	HarassE = HarassMenu->CheckBox ("Use E", true);
	LaneClearQ = LaneClearMenu->CheckBox ("Use Q", true);
	LaneClearManaManager = LaneClearMenu->AddFloat ("ManaManager for Q", 0, 100, 65);
	LaneClearMin = LaneClearMenu->AddInteger ("Minimum Minions to use Q", 0, 10, 4);
	killstealQ = ksSettings->CheckBox ("Use Q", false);
	killstealW = ksSettings->CheckBox ("Use W", false);
	killstealE = ksSettings->CheckBox ("Use E", false);
	Fleemode = MiscMenu->AddKey ("Flee Mode Key", 75);
	gapcloseE = MiscMenu->CheckBox ("Use E on Gap Closers", true);
	interruptE = MiscMenu->CheckBox ("Use E to Interrupt Spells", true);
	ComboAALevel = MiscMenu->AddInteger ("At what level disable AA", 1, 18, 6);
	ComboAA = MiscMenu->CheckBox ("Disable AA", false);
	ComboAAkey = MiscMenu->AddKey ("Disable key", 32);
	FlashCondemn = MiscMenu->AddKey ("Flash Charm key", 84);
	DrawDmg = Drawings->CheckBox ("Draw Damage Calaclations", true);
	HPBar = Drawings->AddColor ("Change Health Bar", 69, 64, 185, 100);
	DrawReady = Drawings->CheckBox ("Draw Ready Spells", true);
	DrawQ = Drawings->CheckBox ("Draw Q", true);
	DrawW = Drawings->CheckBox ("Draw W", true);
	DrawE = Drawings->CheckBox ("Draw E", true);
	DrawR = Drawings->CheckBox ("Draw R", true);
	PredType = { "Oracle", "Core" };
	PredictionType = Prediction->AddSelection ("Choose Prediction Type", 1, PredType);
}

void Ahri::OnGameUpdate()
{
	Automated();
	killSteal();
	if (GGame->IsChatOpen() || !GUtility->IsLeagueWindowFocused())
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
	}
	if (GetAsyncKeyState (Fleemode->GetInteger()))
	{
		FleeMode();
	}
	if (GetAsyncKeyState (ComboAAkey->GetInteger()))
	{
		auto level = Hero->GetLevel();
		if (ComboAA->Enabled() && level >= ComboAALevel->GetInteger() && Hero->GetMana() > 100)
		{
			GOrbwalking->SetAttacksAllowed (false);
		}
	}
	if (!GetAsyncKeyState (ComboAAkey->GetInteger()) || Hero->GetMana() < 100)
	{
		{
			GOrbwalking->SetAttacksAllowed (true);
		}
	}
}

void Ahri::OnRender()
{
	GRender->DrawCircle (Hero->GetPosition(), 30, Vec4 (255, 255, 0, 255), 2);
	Drawing();
	dmgdraw();
}

Vec2 Ahri::vect2d (Vec2 p1, Vec2 p2)
{
	Vec2 temp;
	temp.x = (p2.x - p1.x);
	temp.y = -1 * (p2.y - p1.y);
	return temp;
}
//CREDITS AMBER
bool Ahri::PointInRectangle (Vec2 A, Vec2 B, Vec2 C, Vec2 D, Vec2 m)
{
	Vec2 AB = vect2d (A, B);
	float C1 = -1 * (AB.y*A.x + AB.x*A.y);
	float  D1 = (AB.y*m.x + AB.x*m.y) + C1;
	Vec2 AD = vect2d (A, D);
	float C2 = -1 * (AD.y*A.x + AD.x*A.y);
	float D2 = (AD.y*m.x + AD.x*m.y) + C2;
	Vec2 BC = vect2d (B, C);
	float C3 = -1 * (BC.y*B.x + BC.x*B.y);
	float D3 = (BC.y*m.x + BC.x*m.y) + C3;
	Vec2 CD = vect2d (C, D);
	float C4 = -1 * (CD.y*C.x + CD.x*C.y);
	float D4 = (CD.y*m.x + CD.x*m.y) + C4;
	return     0 >= D1 && 0 >= D4 && 0 <= D2 && 0 >= D3;
}
//CREDITS AMBER
bool Ahri::IsInRectangle (Vec3 Start, Vec3 End, Vec3 pointTest, int radius)
{
	int mediumRadius = radius;
	Vec2 pointTest2 = pointTest.To2D();
	Vec2 From = Start.To2D();
	Vec2 To = End.To2D();
	Vec2 A = From + (From - To).VectorNormalize().Perpendicular() * (-mediumRadius); //Vec2 A1 = A.To2D();
	Vec2 B = From + (From - To).VectorNormalize().Perpendicular() * (mediumRadius); //Vec2 B1 = B.To2D();
	Vec2 C = To + (To - From).VectorNormalize().Perpendicular() * (-mediumRadius); //Vec2 C1 = C.To2D();
	Vec2 D = To + (To - From).VectorNormalize().Perpendicular() * (mediumRadius); //Vec2 D1 = D.To2D();
	return PointInRectangle (A, B, C, D, pointTest2);
}

Vec3 Ahri::GetPredictedUnitPosition (IUnit* Unit, ISpell2* Skillshot, float& TravelTime)
{
	if (Unit->GetWaypointList().size() < 2)
	{
		return Unit->GetPosition();
	}
	TravelTime = ( (Unit->GetPosition() - Hero->GetPosition()).Length2D() - Unit->BoundingRadius()) / Skillshot->Speed() + Skillshot->GetDelay() + (GGame->Latency() / 2) / 1000;
	auto Path = Unit->GetWaypointList();
	Vec3 EstimatedMaxPosition;
	EstimatedMaxPosition = (Unit->GetPosition()).Extend (Path.at (1), Unit->MovementSpeed() * TravelTime);
	for (int i = 0; i < 10; i++)
	{
		TravelTime = ( (EstimatedMaxPosition - Hero->GetPosition()).Length2D() - Unit->BoundingRadius()) / Skillshot->Speed() + Skillshot->GetDelay() + (GGame->Latency() / 2) / 1000;
		EstimatedMaxPosition = (Unit->GetPosition()).Extend (Path.at (1), Unit->MovementSpeed() * TravelTime);
	}
	return EstimatedMaxPosition;
}

bool Ahri::CheckForCollision (ISpell2* Skillshot, Vec3 CheckAtPosition)
{
	for (auto Minion : GEntityList->GetAllMinions (false, true, true))
	{
		if (Minion->IsDead() || (Hero->GetPosition() - Minion->GetPosition()).Length2D() - Minion->BoundingRadius() > Skillshot->Range())
		{
			continue;
		}
		float TravelTime;
		Vec3 MinionPos = GetPredictedUnitPosition (Minion, Skillshot, TravelTime);
		if (GHealthPrediction->GetPredictedHealth (Minion, kLastHitPrediction, TravelTime / 1000, 0) <= 0)
		{
			continue;
		}
		if (IsInRectangle (Hero->GetPosition(), CheckAtPosition, MinionPos, Skillshot->Radius()))
		{
			return false;
		}
		auto CheckFrom = Hero->GetPosition().Extend (CheckAtPosition, (Hero->GetPosition() - MinionPos).Length2D());
		auto D = (CheckFrom - MinionPos).Length2D();
		auto Radius1 = Skillshot->Radius();
		auto Radius2 = Minion->BoundingRadius();
		//The Circles dont intersect:
		if (D > Radius1 + Radius2 || (D <= std::abs (Radius1 - Radius2)))
		{
			continue;
		}
		return false;
	}
	return true;
}

bool Ahri::BestCastPosition (IUnit* Unit, ISpell2* Skillshot, Vec3& CastPosition, bool CheckCollision)
{
	float TravelTime = ( (Unit->GetPosition() - Hero->GetPosition()).Length2D() - Unit->BoundingRadius()) / Skillshot->Speed() + Skillshot->GetDelay() + (GGame->Latency() / 2) / 1000;
	auto Path = Unit->GetWaypointList();
	if (Path.size() > 1) //target is moving
	{
		Vec3 EstimatedMaxPosition;
		EstimatedMaxPosition = (Unit->GetPosition()).Extend (Path.at (1), Unit->MovementSpeed() * TravelTime);
		for (int i = 0; i < 10; i++)
		{
			TravelTime = ( (EstimatedMaxPosition - Hero->GetPosition()).Length2D() - Unit->BoundingRadius()) / Skillshot->Speed() + Skillshot->GetDelay() + (GGame->Latency() / 2) / 1000;
			EstimatedMaxPosition = (Unit->GetPosition()).Extend (Path.at (1), Unit->MovementSpeed() * TravelTime);
		}
		CastPosition = EstimatedMaxPosition.Extend (Unit->GetPosition(), Unit->BoundingRadius() + Skillshot->Radius() * 0.8);
	}
	else
	{
		CastPosition = Unit->GetPosition();
	}
	if (CheckCollision)
	{
		return CheckForCollision (Skillshot, CastPosition);
	}
	else
	{
		return true;
	}
}

void Ahri::CastE (IUnit* target)
{
	if (PredictionType->GetInteger() == 0)
	{
		Vec3 CastOn;
		BestCastPosition (target, E, CastOn, false);
		AdvPredictionOutput prediction_output1;
		E->RunPrediction (target, false, kCollidesWithMinions, &prediction_output1);
		if (prediction_output1.HitChance > kHitChanceCollision)
		{
			E->CastOnPosition (CastOn);
		}
	}
	if (PredictionType->GetInteger() == 1)
	{
		E->CastOnTarget (target);
		/*AdvPredictionOutput prediction_output;
		E->RunPrediction (target, false, kCollidesWithYasuoWall | kCollidesWithMinions, &prediction_output);
		if (prediction_output.HitChance >= kHitChanceVeryHigh)
		{/
			E->CastOnPosition (prediction_output.CastPosition);
		}*/
	}
}

void Ahri::CastQ (IUnit* target)
{
	if (PredictionType->GetInteger() == 0)
	{
		Vec3 CastOn;
		BestCastPosition (target, Q, CastOn, false);
		Q->CastOnPosition (CastOn);
	}
	if (PredictionType->GetInteger() == 1)
	{
		AdvPredictionOutput prediction_output;
		Q->RunPrediction (target, false, kCollidesWithYasuoWall, &prediction_output);
		if (prediction_output.HitChance >= kHitChanceHigh)
		{
			Q->CastOnPosition (prediction_output.CastPosition);
		}
	}
}




void Ahri::AntiGapclose (GapCloserSpell const& args)
{
	if (E->IsReady() && gapcloseE->Enabled() && args.Source->IsValidTarget() && args.Source->IsEnemy (Hero))
	{
		if (args.Source->IsValidTarget (Hero, E->Range()))
		{
			CastE (args.Source);
		}
	}
}

void Ahri::AntiInterrupt (InterruptibleSpell const& args)
{
	auto player = Hero;
	if (interruptE->Enabled() && E->IsReady() && player->IsValidTarget (args.Source, E->Range()) && args.Source != nullptr && args.Source != Hero && args.Source->IsEnemy (Hero))
	{
		CastE (args.Source);
	}
}

Vec3 Ahri::getPosToEflash (Vec3 target)
{
	return  Hero->ServerPosition().Extend (GGame->CursorPosition(), Flash->Range());
}

void Ahri::CastFlash()
{
	auto target = GTargetSelector->GetFocusedTarget() != nullptr
	              ? GTargetSelector->GetFocusedTarget()
	              : GTargetSelector->FindTarget (QuickestKill, SpellDamage, EFlash->Range());
	Flash->CastOnPosition (getPosToEflash (target->GetPosition()));
}

void Ahri::PerformFlashCharm()
{
	GGame->IssueOrder (Hero, kMoveTo, GGame->CursorPosition());
	if (E->IsReady() && Flash->IsReady())
	{
		auto target = GTargetSelector->GetFocusedTarget() != nullptr
		              ? GTargetSelector->GetFocusedTarget()
		              : GTargetSelector->FindTarget (QuickestKill, SpellDamage, EFlash->Range());
		if (target == nullptr || !target->IsHero() || target->IsDead() || !target->IsValidTarget())
		{
			return;
		}
		auto flashPosition = Hero->ServerPosition().Extend (GGame->CursorPosition(), Flash->Range());
		AdvPredictionOutput result;
		EFlash->RunPrediction (target, false, kCollidesWithMinions, &result);
		if (target != nullptr && target->IsValidTarget() && !target->IsDead() && !target->IsInvulnerable() && result.HitChance >= kHitChanceVeryHigh && E->IsReady())
		{
			EFlash->CastOnTarget (target, kHitChanceVeryHigh);
			GPluginSDK->DelayFunctionCall (200 + (GGame->Latency()) / 2, [=]() { CastFlash(); });
		}
	}
}

void Ahri::dmgdraw()
{
	if (DrawDmg->Enabled())
	{
		for (auto hero : GEntityList->GetAllHeros (false, true))
		{
			Vec2 barPos = Vec2();
			if (hero->GetHPBarPosition (barPos) && !hero->IsDead())
			{
				float QDamage = 0;
				float WDamage = 0;
				float EDamage = 0;
				float RDamage = 0;
				if (W->IsReady())
				{
					WDamage = GDamage->GetSpellDamage (Hero, hero, kSlotW);
				}
				if (Q->IsReady())
				{
					QDamage = GDamage->GetSpellDamage (Hero, hero, kSlotQ) + GDamage->GetAutoAttackDamage (Hero, hero, true);
				}
				if (E->IsReady())
				{
					EDamage = GDamage->GetSpellDamage (Hero, hero, kSlotE);
				}
				Vec4 BarColor;
				HPBar->GetColor (&BarColor);
				float totalDamage = QDamage + WDamage + EDamage + RDamage;
				float percentHealthAfterDamage = max (0, hero->GetHealth() - float (totalDamage)) / hero->GetMaxHealth();
				Rembrandt::DrawDamageOnChampionHPBar (hero, totalDamage, BarColor);
			}
		}
	}
}

void Ahri::Combo()
{
	auto player = Hero;
	auto target = GTargetSelector->FindTarget (QuickestKill, SpellDamage, E->Range());
	if (target == nullptr || !target->IsHero())
	{
		return;
	}
	if (ComboW->Enabled() && W->IsReady() && player->IsValidTarget (target, W->Range()) && Hero->GetMana() > 100 + W->ManaCost())
	{
		W->CastOnPlayer();
	}
}
void Ahri::OnNewPath (IUnit* Source, const std::vector<Vec3>& path_)
{
	auto player = Hero;
	auto target = GTargetSelector->FindTarget (QuickestKill, SpellDamage, E->Range());
	if (GOrbwalking->GetOrbwalkingMode() == kModeCombo && target == Source)
	{
		if (target == nullptr || !target->IsHero())
		{
			return;
		}
		if (ComboE->Enabled() && E->IsReady() && player->IsValidTarget (target, E->Range()) && Hero->GetMana() > 100 + E->ManaCost())
		{
			CastE (target);
		}
		if (ComboQ->Enabled() && Q->IsReady() && player->IsValidTarget (target, Q->Range()))
		{
			CastQ (target);
		}
	}
	if (GOrbwalking->GetOrbwalkingMode() == kModeMixed && target == Source)
	{
		auto player = Hero;
		auto target = GTargetSelector->FindTarget (QuickestKill, SpellDamage, E->Range());
		if (target == nullptr || !target->IsHero())
		{
			return;
		}
		if (E->IsReady() && HarassE->Enabled() && player->IsValidTarget (target, E->Range()) && Hero->GetMana() > 100 + E->ManaCost())
		{
			CastE (target);
		}
		if (Q->IsReady() && HarassQ->Enabled() && player->IsValidTarget (target, Q->Range()) && Hero->GetMana() > 100 + Q->ManaCost())
		{
			CastQ (target);
		}
	}
}

void Ahri::Harass()
{
	auto player = Hero;
	auto target = GTargetSelector->FindTarget (QuickestKill, SpellDamage, E->Range());
	if (target == nullptr || !target->IsHero())
	{
		return;
	}
	if (HarassW->Enabled() && HarassW->Enabled() && W->IsReady() && player->IsValidTarget (target, W->Range()) && Hero->GetMana() > 100 + W->ManaCost())
	{
		W->CastOnPlayer();
	}
}


void Ahri::LaneClear()
{
	std::vector<Vec3> CastPos;
	CastPos.push_back (Hero->GetPosition());
	FarmLocation Farmlocation;
	Rembrandt::FindBestLineCastPosition (CastPos, Q->Range(), Q->Range(), Q->Radius(), true, true, Farmlocation);
	if (LaneClearQ->Enabled() && Hero->ManaPercent() >= LaneClearManaManager->GetFloat() && Farmlocation.HitCount >= LaneClearMin->GetInteger())
	{
		Q->CastOnPosition (Farmlocation.CastPosition);
	}
}


void Ahri::FleeMode()
{
	GGame->IssueOrder (Hero, kMoveTo, GGame->CursorPosition());
	if (Q->IsReady())
	{
		Q->CastOnPosition (Hero->GetPosition().Extend (GGame->CursorPosition(), -400));
	}
}


void Ahri::killSteal()
{
	auto target = GTargetSelector->FindTarget (QuickestKill, SpellDamage, E->Range());
	if (target == nullptr || !target->IsHero())
	{
		return;
	}
	if (killstealE->Enabled() && E->IsReady() && Hero->IsValidTarget (target, E->Range()) && GDamage->GetSpellDamage (Hero, target, kSlotE) > target->GetHealth())
	{
		CastE (target);
	}
	if (killstealQ->Enabled() && Q->IsReady() && Hero->IsValidTarget (target, Q->Range()) && GDamage->GetSpellDamage (Hero, target, kSlotQ) > target->GetHealth())
	{
		CastQ (target);
	}
	if (killstealW->Enabled() && W->IsReady() && Hero->IsValidTarget (target, W->Range()) && GDamage->GetSpellDamage (Hero, target, kSlotW) > target->GetHealth())
	{
		W->CastOnPlayer();
	}
}

float GetDistance (IUnit* Player, IUnit* target)
{
	return (Player->GetPosition() - target->GetPosition()).Length2D();
}

void Ahri::Automated()
{
	auto target = GTargetSelector->FindTarget (QuickestKill, SpellDamage, E->Range());
	if (autoQ->Enabled() && Q->IsReady() && Hero->IsValidTarget (target, Q->Range()) && Hero->GetMana() > (100 + W->ManaCost() + Q->ManaCost() * 2)) //magic number for r mana cost
	{
		if (GetDistance (Hero, target) > 730)
		{
			IUnit *target = GTargetSelector->FindTarget (QuickestKill, SpellDamage, Q->Range());
			AdvPredictionOutput result;
			Q->RunPrediction (target, false, kCollidesWithNothing, &result);
			if (result.HitChance >= kHitChanceVeryHigh)
			{
				Q->CastOnPosition (result.CastPosition);
			}
		}
	}
	if (GetAsyncKeyState (FlashCondemn->GetInteger()) && !GGame->IsChatOpen())
	{
		PerformFlashCharm();
	}
	for (auto target : GEntityList->GetAllHeros (false, true))
	{
		if (Hero->IsValidTarget (target, E->Range()) && target != nullptr && !target->IsDead())
		{
			AdvPredictionOutput prediction_output;
			E->RunPrediction (target, true, kCollidesWithMinions || kCollidesWithYasuoWall, &prediction_output);
			if (prediction_output.HitChance == kHitChanceImmobile)
			{
				E->CastOnTarget (target, kHitChanceImmobile);
			}
		}
	}
}

void Ahri::Drawing()
{
	if (DrawReady->Enabled())
	{
		if (Q->IsReady() && DrawQ->Enabled())
		{
			GRender->DrawOutlinedCircle (Hero->GetPosition(), Vec4 (225, 225, 0, 225), Q->Range());
		}
		if (W->IsReady() && DrawW->Enabled())
		{
			GRender->DrawOutlinedCircle (Hero->GetPosition(), Vec4 (225, 225, 0, 225), W->Range());
		}
		if (E->IsReady() && DrawE->Enabled())
		{
			GRender->DrawOutlinedCircle (Hero->GetPosition(), Vec4 (225, 225, 0, 225), E->Range());
		}
	}
	else
	{
		if (DrawQ->Enabled())
		{
			GRender->DrawOutlinedCircle (Hero->GetPosition(), Vec4 (225, 225, 0, 225), Q->Range());
		}
		if (DrawW->Enabled())
		{
			GRender->DrawOutlinedCircle (Hero->GetPosition(), Vec4 (225, 225, 0, 225), W->Range());
		}
		if (DrawE->Enabled())
		{
			GRender->DrawOutlinedCircle (Hero->GetPosition(), Vec4 (225, 225, 0, 225), E->Range());
		}
	}
}