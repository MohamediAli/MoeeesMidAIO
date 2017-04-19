#include "Ahri.h"
#include "Extensions.h"
#include "Rembrandt.h"


Ahri::~Ahri()
{
	AhriMenu->Remove();
}

IUnit* myHero;



Ahri::Ahri(IMenu* Parent, IUnit* Hero) :Champion(Parent, Hero)
{
	Q = GPluginSDK->CreateSpell2(kSlotQ, kLineCast, false, false, kCollidesWithNothing);
	Q->SetSkillshot(0.25f, 90, 1550, 840);
	W = GPluginSDK->CreateSpell2(kSlotW, kTargetCast, false, false, kCollidesWithNothing);
	W->SetOverrideRange(580);
	E = GPluginSDK->CreateSpell2(kSlotE, kLineCast, false, false, static_cast<eCollisionFlags>(kCollidesWithMinions | kCollidesWithYasuoWall));
	E->SetSkillshot(0.25f, 60, 1550, 890);
	EFlash = GPluginSDK->CreateSpell2(kSlotE, kLineCast, false, false, static_cast<eCollisionFlags>(kCollidesWithMinions | kCollidesWithYasuoWall));
	EFlash->SetSkillshot(0.25f, 60, 3100, 1350);

	if (strcmp(GEntityList->Player()->GetSpellName(kSummonerSlot1), "SummonerFlash") == 0)
	{
		Flash = GPluginSDK->CreateSpell2(kSummonerSlot1, kCircleCast, false, false, kCollidesWithNothing);
		Flash->SetOverrideRange(425.f);
	}
	if (strcmp(GEntityList->Player()->GetSpellName(kSummonerSlot2), "SummonerFlash") == 0)
	{
		Flash = GPluginSDK->CreateSpell2(kSummonerSlot2, kCircleCast, false, false, kCollidesWithNothing);
		Flash->SetOverrideRange(425.f);
	}

	AhriMenu = Parent->AddMenu("Ahri Menu");
	ComboMenu = Parent->AddMenu("Combo");
	HarassMenu = Parent->AddMenu("Harass");
	LaneClearMenu = Parent->AddMenu("LaneClear");
	ksSettings = Parent->AddMenu("Kill Steal");
	Drawings = Parent->AddMenu("Spell Drawings");
	MiscMenu = Parent->AddMenu("Miscs");

	ComboQ = ComboMenu->CheckBox("Use Q", true);
	ComboW = ComboMenu->CheckBox("Use W", true);
	ComboE = ComboMenu->CheckBox("Use E", true);


	HarassQ = HarassMenu->CheckBox("Use Q", true);
	HarassW = HarassMenu->CheckBox("Use W", true);
	HarassE = HarassMenu->CheckBox("Use E", true);

	LaneClearQ = LaneClearMenu->CheckBox("Use Q", true);
	LaneClearManaManager = LaneClearMenu->AddFloat("ManaManager for Q", 0, 100, 65);
	LaneClearMin = LaneClearMenu->AddInteger("Minimum Minions to use Q", 0, 10, 4);

	killstealQ = ksSettings->CheckBox("Use Q", true);
	killstealW = ksSettings->CheckBox("Use W", true);
	killstealE = ksSettings->CheckBox("Use E", true);

	Fleemode = MiscMenu->AddKey("Flee Mode Key", 75);
	autoQ = MiscMenu->CheckBox("Use Q Automatically", true);
	gapcloseE = MiscMenu->CheckBox("Use E on Gap Closers", true);
	interruptE = MiscMenu->CheckBox("Use E to Interrupt Spells", true);
	ComboAALevel = MiscMenu->AddInteger("At what level disable AA", 1, 18, 6);
	ComboAA = MiscMenu->CheckBox("Disable AA", false);
	ComboAAkey = MiscMenu->AddKey("Disable key", 32);
	FlashCondemn = MiscMenu->AddKey("Flash Charm key", 84);


	DrawDmg = Drawings->CheckBox("Draw Damage Calaclations", true);
	HPBar = Drawings->AddColor("Change Health Bar", 69, 64, 185, 100);
	DrawReady = Drawings->CheckBox("Draw Ready Spells", true);
	DrawQ = Drawings->CheckBox("Draw Q", true);
	DrawW = Drawings->CheckBox("Draw W", true);
	DrawE = Drawings->CheckBox("Draw E", true);
	DrawR = Drawings->CheckBox("Draw R", true);
}

void Ahri::OnGameUpdate()
{
	Automated();
	killSteal();

	if (GGame->IsChatOpen() || !GUtility->IsLeagueWindowFocused()) {
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

	if (GetAsyncKeyState(Fleemode->GetInteger()))
	{
		FleeMode();
	}

	if (GetAsyncKeyState(ComboAAkey->GetInteger()))
	{
		auto level = GEntityList->Player()->GetLevel();
		if (ComboAA->Enabled() && level >= ComboAALevel->GetInteger() && GEntityList->Player()->GetMana() > 100)
		{
			GOrbwalking->SetAttacksAllowed(false);
		}
	}
	if (!GetAsyncKeyState(ComboAAkey->GetInteger()) || GEntityList->Player()->GetMana() < 100)
	{
		{
			GOrbwalking->SetAttacksAllowed(true);
		}
	}



}

void Ahri::OnRender()
{
	GRender->DrawCircle(Hero->GetPosition(), 30, Vec4(255, 255, 0, 255), 2);
	Drawing();
	dmgdraw();
}


void Ahri::CastE(IUnit* target)
{
	AdvPredictionOutput prediction_output;
	E->RunPrediction(target, false, kCollidesWithYasuoWall | kCollidesWithMinions, &prediction_output);
	if (prediction_output.HitChance >= kHitChanceVeryHigh)
	{
		E->CastOnTarget(target, kHitChanceCollision);
	}
}

void Ahri::AntiGapclose(GapCloserSpell const& args)
{
	if (E->IsReady() && gapcloseE->Enabled() && args.Source->IsValidTarget() && args.Source->IsEnemy(GEntityList->Player()))
	{
		if (args.Source->IsValidTarget(GEntityList->Player(), E->Range()))
		{
			E->CastOnTarget(args.Source, kHitChanceHigh);
		}
	}

}

void Ahri::AntiInterrupt(InterruptibleSpell const& args)
{
	auto player = GEntityList->Player();
	if (interruptE->Enabled() && E->IsReady() && player->IsValidTarget(args.Source, E->Range()) && args.Source != nullptr && args.Source != GEntityList->Player() && args.Source->IsEnemy(GEntityList->Player()))
	{
		E->CastOnTarget(args.Source, kHitChanceHigh);
	}
}

Vec3 Ahri::getPosToEflash(Vec3 target)
{

	return  GEntityList->Player()->ServerPosition().Extend(GGame->CursorPosition(), Flash->Range());
}

void Ahri::CastFlash() {
	auto target = GTargetSelector->GetFocusedTarget() != nullptr
		? GTargetSelector->GetFocusedTarget()
		: GTargetSelector->FindTarget(QuickestKill, SpellDamage, EFlash->Range());
	Flash->CastOnPosition(getPosToEflash(target->GetPosition()));
}

void Ahri::PerformFlashCharm()
{
	GGame->IssueOrder(GEntityList->Player(), kMoveTo, GGame->CursorPosition());
	if (E->IsReady() && Flash->IsReady())
	{
		auto target = GTargetSelector->GetFocusedTarget() != nullptr
			? GTargetSelector->GetFocusedTarget()
			: GTargetSelector->FindTarget(QuickestKill, SpellDamage, EFlash->Range());
		if (target == nullptr || !target->IsHero() || target->IsDead() || !target->IsValidTarget() )
			return;


		auto flashPosition = GEntityList->Player()->ServerPosition().Extend(GGame->CursorPosition(), Flash->Range());
		AdvPredictionOutput result;
		EFlash->RunPrediction(target, false, kCollidesWithMinions, &result);

		if (target != nullptr && target->IsValidTarget() && !target->IsDead() && !target->IsInvulnerable() && result.HitChance >= kHitChanceVeryHigh)
		{
			EFlash->CastOnTarget(target, kHitChanceVeryHigh);
			GPluginSDK->DelayFunctionCall(200 + (GGame->Latency()) / 2, [=]() { CastFlash(); });

		}
	}
}

void Ahri::dmgdraw()
{
	if (DrawDmg->Enabled()) {
		for (auto hero : GEntityList->GetAllHeros(false, true))
		{
			Vec2 barPos = Vec2();
			if (hero->GetHPBarPosition(barPos) && !hero->IsDead())
			{
				float QDamage = 0;
				float WDamage = 0;
				float EDamage = 0;
				float RDamage = 0;
				if (W->IsReady()) {
					WDamage = GDamage->GetSpellDamage(GEntityList->Player(), hero, kSlotW);
				}
				if (Q->IsReady()) {
					QDamage = GDamage->GetSpellDamage(GEntityList->Player(), hero, kSlotQ) + GDamage->GetAutoAttackDamage(GEntityList->Player(), hero, true);
				}
				if (E->IsReady()) {
					EDamage = GDamage->GetSpellDamage(GEntityList->Player(), hero, kSlotE);
				}
				Vec4 BarColor;
				HPBar->GetColor(&BarColor);

				float totalDamage = QDamage + WDamage + EDamage + RDamage;
				float percentHealthAfterDamage = max(0, hero->GetHealth() - float(totalDamage)) / hero->GetMaxHealth();
				Rembrandt::DrawDamageOnChampionHPBar(hero, totalDamage, BarColor);
			}
		}
	}
}



void Ahri::Combo()
{
	auto player = GEntityList->Player();
	auto target = GTargetSelector->FindTarget(QuickestKill, SpellDamage, E->Range());

	if (target == nullptr || !target->IsHero())
		return;


	if (ComboE->Enabled() && E->IsReady() && player->IsValidTarget(target, E->Range()) && GEntityList->Player()->GetMana() > 100 + E->ManaCost())
	{
		E->CastOnTarget(target, kHitChanceVeryHigh);
	}


	if (ComboQ->Enabled() && Q->IsReady() && player->IsValidTarget(target, Q->Range()))
	{
		Q->CastOnTarget(target, kHitChanceVeryHigh);

	}

	if (ComboW->Enabled() && W->IsReady() && player->IsValidTarget(target, W->Range()) && GEntityList->Player()->GetMana() > 100 + W->ManaCost())
	{
		W->CastOnPlayer();

	}

}


void Ahri::Harass()
{
	auto player = GEntityList->Player();
	auto target = GTargetSelector->FindTarget(QuickestKill, SpellDamage, E->Range());

	if (target == nullptr || !target->IsHero())
		return;


	if (E->IsReady() && HarassE->Enabled() && player->IsValidTarget(target, E->Range()) && GEntityList->Player()->GetMana() > 100 + E->ManaCost())
	{
		CastE(target);

	}

	if (Q->IsReady() && HarassQ->Enabled() && player->IsValidTarget(target, Q->Range()) && GEntityList->Player()->GetMana() > 100 + Q->ManaCost())
	{
		Q->CastOnTarget(target, kHitChanceVeryHigh);

	}

	if (HarassW->Enabled() && HarassW->Enabled() && W->IsReady() && player->IsValidTarget(target, W->Range()) && GEntityList->Player()->GetMana() > 100 + W->ManaCost())
	{
		W->CastOnPlayer();

	}


}


void Ahri::LaneClear()
{
	auto player = GEntityList->Player();
	std::vector<Vec3> CastPos;
	CastPos.push_back(Hero->GetPosition());
	//	FarmLocation Farmlocation;
	//	Rembrandt::FindBestLineCastPosition(CastPos, Q->Range(), Q->Range(), Q->Radius(), true, true, Farmlocation);
	//	if (LaneClearQ->Enabled() && player->ManaPercent() >= LaneClearManaManager->GetFloat() && Farmlocation.HitCount >= LaneClearMin->GetInteger()) {
	//		Q->CastOnPosition(Farmlocation.CastPosition);

	//	}
}


void Ahri::FleeMode()
{
	GGame->IssueOrder(GEntityList->Player(), kMoveTo, GGame->CursorPosition());
	if (Q->IsReady())
		Q->CastOnPosition(GEntityList->Player()->GetPosition().Extend(GGame->CursorPosition(), -400));

}


void Ahri::killSteal() {

	auto player = GEntityList->Player();
	auto target = GTargetSelector->FindTarget(QuickestKill, SpellDamage, E->Range());


	if (target == nullptr || !target->IsHero())
		return;


	if (killstealE->Enabled() && E->IsReady() && player->IsValidTarget(target, E->Range()) && GDamage->GetSpellDamage(GEntityList->Player(), target, kSlotE) > target->GetHealth())
	{
		CastE(target);

	}


	if (killstealQ->Enabled() && Q->IsReady() && player->IsValidTarget(target, Q->Range()) && GDamage->GetSpellDamage(GEntityList->Player(), target, kSlotQ) > target->GetHealth())
	{
		Q->CastOnTarget(target, kHitChanceVeryHigh);

	}

	if (killstealW->Enabled() && W->IsReady() && player->IsValidTarget(target, W->Range()) && GDamage->GetSpellDamage(GEntityList->Player(), target, kSlotW) > target->GetHealth())
	{
		W->CastOnPlayer();

	}

}

float GetDistance(IUnit* Player, IUnit* target)
{
	return (Player->GetPosition() - target->GetPosition()).Length2D();
}

void Ahri::Automated()
{
	auto player = GEntityList->Player();
	auto target = GTargetSelector->FindTarget(QuickestKill, SpellDamage, E->Range());


	if (autoQ->Enabled() && Q->IsReady() && player->IsValidTarget(target, Q->Range()) && GEntityList->Player()->GetMana() > (100 + W->ManaCost() + Q->ManaCost() * 2)) //magic number for r mana cost
	{
		if (GetDistance(player, target) > 730) {
			IUnit *target = GTargetSelector->FindTarget(QuickestKill, SpellDamage, Q->Range());

			AdvPredictionOutput result;
			Q->RunPrediction(target, false, kCollidesWithNothing, &result);
			if (result.HitChance >= kHitChanceVeryHigh)
			{
				Q->CastOnPosition(result.CastPosition);
			}

		}
	}

	if (GetAsyncKeyState(FlashCondemn->GetInteger()) && !GGame->IsChatOpen())
	{
		PerformFlashCharm();
	}

	for (auto target : GEntityList->GetAllHeros(false, true))
	{

		if (player->IsValidTarget(target, E->Range()) && target != nullptr && !target->IsDead()) {
			AdvPredictionOutput prediction_output;
			E->RunPrediction(target, true, kCollidesWithMinions || kCollidesWithYasuoWall, &prediction_output);


			if (prediction_output.HitChance == kHitChanceImmobile)
			{
				E->CastOnTarget(target, kHitChanceImmobile);
			}
		}


	}
}

void Ahri::Drawing()
{
	auto player = GEntityList->Player();
	if (DrawReady->Enabled())
	{
		if (Q->IsReady() && DrawQ->Enabled())
		{
			GRender->DrawOutlinedCircle(player->GetPosition(), Vec4(225, 225, 0, 225), Q->Range());
		}

		if (W->IsReady() && DrawW->Enabled())
		{
			GRender->DrawOutlinedCircle(player->GetPosition(), Vec4(225, 225, 0, 225), W->Range());
		}

		if (E->IsReady() && DrawE->Enabled())
		{
			GRender->DrawOutlinedCircle(player->GetPosition(), Vec4(225, 225, 0, 225), E->Range());
		}


	}

	else
	{
		if (DrawQ->Enabled())
		{
			GRender->DrawOutlinedCircle(player->GetPosition(), Vec4(225, 225, 0, 225), Q->Range());
		}

		if (DrawW->Enabled())
		{
			GRender->DrawOutlinedCircle(player->GetPosition(), Vec4(225, 225, 0, 225), W->Range());
		}

		if (DrawE->Enabled())
		{
			GRender->DrawOutlinedCircle(player->GetPosition(), Vec4(225, 225, 0, 225), E->Range());
		}


	}
}
