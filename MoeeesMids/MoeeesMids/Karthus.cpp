#define NOMINMAX
#include "Karthus.h"
#include <algorithm>
#include "Extensions.h"
#define M_PI 3.14159265358979323846


Karthus::~Karthus()
{
	KarthusMenu->Remove();
}


Karthus::Karthus (IMenu* Parent, IUnit* Hero) :Champion (Parent, Hero)
{
	Q = GPluginSDK->CreateSpell2 (kSlotQ, kLineCast, true, true, kCollidesWithNothing);
	Q->SetSkillshot (1.f, 140.f, FLT_MAX, 890.f);
	W = GPluginSDK->CreateSpell2 (kSlotW, kCircleCast, false, true, kCollidesWithNothing);
	W->SetSkillshot (1.0f, 200.f, FLT_MAX, 1000.f);
	E = GPluginSDK->CreateSpell2 (kSlotE, kConeCast, false, true, kCollidesWithWalls);
	E->SetSkillshot (0.30f, 550.f, FLT_MAX, 550.f);
	R = GPluginSDK->CreateSpell2 (kSlotR, kTargetCast, false, false, kCollidesWithNothing);
	KarthusMenu = Parent->AddMenu ("Karthus Menu");
	ComboMenu = Parent->AddMenu ("Combo");
	qMenu = Parent->AddMenu ("Q Settings");
	wMenu = Parent->AddMenu ("W Settings");
	eMenu = Parent->AddMenu ("E Settings");
	Drawings = Parent->AddMenu ("All Drawings");
	MiscMenu = Parent->AddMenu ("Miscs");
	ComboQ = qMenu->CheckBox ("Use Q in Combo", true);
	autoQ = qMenu->CheckBox ("Automatic Harass Q", true);
	harassQ = qMenu->CheckBox ("Harass with Q", true);
	harassQMana = qMenu->AddFloat ("^-> Only Harras Q if Mana >", 0, 100, 50);
	laneClearQ = qMenu->CheckBox ("Wave Clear with Q", true);
	laneClearQMana = qMenu->AddFloat ("^-> Only Wave Clear Q if Mana >", 0, 100, 70);
	JungleClearQ = qMenu->CheckBox ("Jungle Clear with Q", true);
	JungleClearQMana = qMenu->AddFloat ("^-> Only Jungle Clear Q if Mana >", 0, 100, 50);
	ComboW = wMenu->CheckBox ("Use W in Combo", true);
	harassW = qMenu->CheckBox ("Harass with W", false);
	gapCloserW = wMenu->CheckBox ("W on Gap Closer", true);
	ComboE = eMenu->CheckBox ("Use E in Combo", true);
	harassE = eMenu->CheckBox ("Harass with E", true);
	harassEMana = eMenu->AddFloat ("^-> Only Harras E if Mana >", 0, 100, 50);
	laneClearE = eMenu->CheckBox ("Wave Clear with E", true);
	laneClearEMana = eMenu->AddFloat ("^-> Only Wave Clear E if Mana >", 0, 100, 70);
	JungleClearE = eMenu->CheckBox ("Jungle Clear with E", true);
	JungleClearEMana = eMenu->AddFloat ("^-> Only Jungle Clear E if Mana >", 0, 100, 50);
	DrawReady = Drawings->CheckBox ("Draw Ready Spells", true);
	drawDmg = Drawings->CheckBox ("Draw R killable", true);
	DrawQ = Drawings->CheckBox ("Draw Q", true);
	DrawW = Drawings->CheckBox ("Draw W", true);
	DrawE = Drawings->CheckBox ("Draw E", false);
}

void Karthus::OnGameUpdate()
{
	automatic();
	zigzag();
	QTarget = GTargetSelector->FindTarget (QuickestKill, SpellDamage, Q->Range());
	if (GOrbwalking->GetOrbwalkingMode() == kModeCombo)
		{
		Combo();
		}
	if (GOrbwalking->GetOrbwalkingMode() == kModeMixed)
		{
		LastHit();
		Harass();
		}
	if (GOrbwalking->GetOrbwalkingMode() == kModeLastHit)
		{
		LastHit();
		}
	if (GOrbwalking->GetOrbwalkingMode() == kModeLaneClear)
		{
		LaneClear();
		}
}
void Karthus::OnRender()
{
	Drawing();
	dmgdraw();
}

void Karthus::AntiGapclose (GapCloserSpell const& args)
{
	if (W->IsReady() && gapCloserW->Enabled() && Hero->IsValidTarget (args.Source, W->Range()) && args.Source->IsEnemy (Hero))
		{
		AdvPredictionOutput outputfam;
		W->RunPrediction (args.Source, false, kCollidesWithMinions, &outputfam);
		if (outputfam.HitChance == kHitChanceDashing)
			{
			W->CastOnTarget (args.Source, kHitChanceDashing);
			}
		}
}

void Karthus::automatic()
{
	if (E->IsReady() && Hero->HasBuff ("KarthusDefile"))
		{
		if (Extensions::CountMinionsInTargetRange (Hero->GetPosition(), E->Range()) > 2 && (GOrbwalking->GetOrbwalkingMode() == kModeLaneClear))
			{
			return;
			}
		if (Extensions::EnemiesInRange (Hero->GetPosition(), E->Range()) && (GOrbwalking->GetOrbwalkingMode() == kModeCombo || GOrbwalking->GetOrbwalkingMode() == kModeMixed))
			{
			return;
			}
		E->CastOnPlayer();
		}
	if (autoQ->Enabled() && Q->IsReady())
		{
		if (QTarget == nullptr || !QTarget->IsHero() || !QTarget->IsValidTarget())
			{
			return;
			}
		if (!cz)
			{
			Q->CastOnPosition (PredPos (QTarget, 0.75f + (GGame->Latency() / 1000) / 2));
			}
		}
}


void Karthus::eToggle()
{
	if (E->IsReady() && !Hero->HasBuff ("KarthusDefile"))
		{
		E->CastOnPlayer();
		}
}

float Karthus::qWidthChange (IUnit* target)
{
	return 	std::max (30.f, (1.f - (Extensions::GetDistance (Hero, target->GetPosition())) / Q->Range()) * 160.f);
}

float Karthus::wWidthChange (IUnit* target)
{
	return 	std::max (70.f, (1.f - (Extensions::GetDistance (Hero, target->GetPosition())) / W->Range()) * 160.f);
}

void Karthus::zigzag()   //credits sn karthus
{
	if (QTarget == nullptr || !QTarget->IsHero() || !QTarget->IsValidTarget())
		{
		return;
		}
	if (czx < czx2)
		{
		if (czx2 >= QTarget->GetPosition().x)
			{
			cz = true;
			}
		else
			{
			cz = false;
			}
		}
	else if (czx == czx2)
		{
		cz = false;
		czx = czx2;
		czx2 = (QTarget->GetPosition().x);
		return;
		}
	else
		{
		if (czx2 <= QTarget->GetPosition().x)
			{
			cz = true;
			}
		else
			{
			cz = false;
			}
		}
	czx = czx2;
	czx2 = QTarget->GetPosition().x;
	if (czy < czy2)
		{
		if (czy2 >= QTarget->GetPosition().z)
			{
			cz = true;
			}
		else
			{
			cz = false;
			}
		}
	else if (czy == czy2)
		{
		cz = false;
		}
	else
		{
		if (czy2 <= QTarget->GetPosition().z)
			{
			cz = true;
			}
		else
			{
			cz = false;
			}
		}
	czy = czy2;
	czy2 = QTarget->GetPosition().z;
}

Vec3 Karthus::PredPos (IUnit* Hero, float Delay)    //credits sn karthus
{
	float value = 0.f;
	if (Hero->IsFacing (Hero))
		{
		value = (50.f - Hero->BoundingRadius());
		}
	else
		{
		value = - (100.f - Hero->BoundingRadius());
		}
	auto distance = Delay * Hero->MovementSpeed() + value;
	auto path = Hero->GetWaypointList();
	for (auto i = 0; i < path.size() - 1; i++)
		{
		auto a = path[i];
		auto b = path[i + 1];
		auto d = Extensions::GetDistance (a, b);
		if (d < distance)
			{
			distance -= d;
			}
		else
			{
			return (a + distance * (b - a).VectorNormalize());
			}
		}
	return (path[path.size() - 1]).To3D();
}

float Karthus::WWidth()
{
	return 700 + 100 * GEntityList->Player()->GetSpellLevel (kSlotW);
}

float Karthus::WMaxRangeSqr()
{
	float WHalfWidth = WWidth() / 2.0f;
	return W->Range() * W->Range() + WHalfWidth*WHalfWidth;
}

float Karthus::WMaxRange()
{
	return std::sqrt (WMaxRangeSqr());
}

bool Karthus::IsInWRange (Vec2 position)
{
	return Extensions::GetDistanceSqr (GEntityList->Player()->GetPosition().To2D(), position) < WMaxRangeSqr();
}

//Divine Nader[Sl]
void  Karthus::CastW()
{
	auto target = GTargetSelector->FindTarget (QuickestKill, SpellDamage, WMaxRange());
	if (target == nullptr || !target->IsValidTarget() || target->IsDead())
		{
		return;
		}
	Vec3 futurePos;
	GPrediction->GetFutureUnitPosition (target, W->GetDelay(), true, futurePos);
	if (IsInWRange (futurePos.To2D()))
		if (Hero->IsValidTarget (target, W->Range()))
			{
			W->SetOverrideRadius (wWidthChange (target));
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



Vec3 Karthus::FarmQ (Vec3 pos)
{
	auto posChecked = 0;
	auto maxPosChecked = 52;
	auto posRadius = 65;
	auto radiusIndex = 0;
	std::vector<std::pair<int, Vec2>> CloseQPositions;
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
			auto xPos = static_cast<float> (floor (pos.x + curRadius * cos (cRadians)));
			auto zPos = static_cast<float> (floor (pos.z + curRadius * sin (cRadians)));
			auto posFor2D = Vec2 (xPos, zPos);
			auto count = Extensions::CountMinionsInTargetRange (Extensions::To3D (posFor2D), 160.f);
			//GGame->PrintChat(std::to_string(count).c_str());
			if (GNavMesh->IsPointWall (Extensions::To3D (posFor2D)))
				{
				// dont push is wall
				continue;
				}
			if (Extensions::Dist2D (posFor2D, Hero->ServerPosition()) > Q->Range())
				{
				// dont push to far away to cast;
				continue;
				}
			if (Extensions::Dist2D (posFor2D, pos) <= Q->Radius())
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
	for (auto entry : possibleQPositions)
		{
		return (Extensions::To3D (entry.second));
		}
}

void Karthus::Combo()
{
	if (Extensions::EnemiesInRange (Hero->GetPosition(), E->Range()) && ComboE->Enabled())
		{
		eToggle();
		}
	auto player = Hero;
	auto target = GTargetSelector->FindTarget (QuickestKill, SpellDamage, Q->Range());
	if (target == nullptr || !target->IsHero() || !target->IsValidTarget())
		{
		return;
		}
	if (ComboQ->Enabled() && Q->IsReady() && player->IsValidTarget (target, Q->Range()))
		{
		//	Q->SetOverrideRadius (qWidthChange (target));
		if (!cz)
			{
			Q->CastOnPosition (PredPos (QTarget, 0.75f + (GGame->Latency() / 1000) / 2));
			}
		}
	if (ComboW->Enabled() && W->IsReady())
		{
		CastW();
		}
}

void Karthus::Harass()
{
	auto player = Hero;
	if (Extensions::EnemiesInRange (Hero->GetPosition(), E->Range()) && harassE->Enabled() && Hero->ManaPercent() >= harassEMana->GetFloat())
		{
		eToggle();
		}
	auto target = GTargetSelector->FindTarget (QuickestKill, SpellDamage, Q->Range());
	if (!Extensions::Validate (target) && !target->IsHero() || !target->IsValidTarget())
		{
		return;
		}
	if (harassQ->Enabled() && Q->IsReady() && player->IsValidTarget (target, Q->Range()) && Hero->ManaPercent() >= harassQMana->GetFloat())
		{
		//Q->SetOverrideRadius(qWidthChange(target));
		if (!cz)
			{
			Q->CastOnPosition (PredPos (QTarget, 0.75f + (GGame->Latency() / 1000) / 2));
			}
		}
	if (harassW->Enabled() && W->IsReady() && player->IsValidTarget (target, W->Range()))
		{
		W->SetOverrideRadius (wWidthChange (target));
		W->CastOnTarget (target);
		}
}


float Karthus::qDmg (IUnit* Target)
{
	float InitDamage = 0;
	float BonusStackDamage = ( (0.3 * Hero->TotalMagicDamage()));
	if (Hero->GetSpellLevel (kSlotQ) == 1)
		{
		InitDamage += (40 + BonusStackDamage);
		}
	else if (Hero->GetSpellLevel (kSlotQ) == 2)
		{
		InitDamage += (60 + BonusStackDamage);
		}
	else if (Hero->GetSpellLevel (kSlotQ) == 3)
		{
		InitDamage += (80 + BonusStackDamage);
		}
	else if (Hero->GetSpellLevel (kSlotQ) == 4)
		{
		InitDamage += (100 + BonusStackDamage);
		}
	else if (Hero->GetSpellLevel (kSlotQ) == 5)
		{
		InitDamage += (120 + BonusStackDamage);
		}
	return GDamage->CalcMagicDamage (Hero, Target, InitDamage);
}

float Karthus::rDmg (IUnit* Target)
{
	float InitDamage = 0;
	float BonusStackDamage = (0.6 * Hero->TotalMagicDamage());
	if (Hero->GetSpellLevel (kSlotR) == 1)
		{
		InitDamage += 250 + BonusStackDamage;
		}
	else if (Hero->GetSpellLevel (kSlotR) == 2)
		{
		InitDamage += 400 + BonusStackDamage;
		}
	else if (Hero->GetSpellLevel (kSlotR) == 3)
		{
		InitDamage += 550 + BonusStackDamage;
		}
	return GDamage->CalcMagicDamage (Hero, Target, InitDamage);
}

void Karthus::dmgdraw()
{
	std::string killable = "Killable : ";
	if (R->IsReady())
		{
		for (auto enemy : GEntityList->GetAllHeros (false, true))
			{
			if (!enemy->IsDead())
				{
				auto health = enemy->GetHealth() + enemy->HPRegenRate() * R->GetDelay();
				if (enemy->IsVisible())
					{
					LastSeen = GGame->Time();
					}
				if (health < rDmg (enemy) && GGame->Time()-LastSeen<15)
					{
					killable += enemy->ChampionName();
					killable.append (" ");
					if (GGame->Time() - LastPing > 1 && enemy->IsVisible())
						{
						LastPing = GGame->Time();
						GGame->ShowPing (kPingDanger, enemy, true);
						}
					}
				}
			}
		}
	if (killable != "Killable : ")
		{
		Vec2 pos;
		(GGame->Projection (Hero->GetPosition(), &pos));
		static auto message = GRender->CreateFontW ("Impact", 30.f, kFontWeightNormal);
		message->SetColor (Vec4 (255, 0, 0, 255));
		message->SetOutline (true);
		message->Render (pos.x + 10 + 32, pos.y + 10, killable.c_str());
		}
}


void Karthus::LaneClear()
{
	auto player = Hero;
	if (Extensions::CountMinionsInTargetRange (player->GetPosition(), E->Range()) > 2 && E->IsReady() && laneClearE->Enabled() && Hero->ManaPercent() >= laneClearEMana->GetFloat())
		{
		eToggle();
		}
	Q->SetOverrideRadius (160.f);
	if (Q->IsReady() && laneClearQ->Enabled() && Hero->ManaPercent() >= laneClearQMana->GetFloat())
		{
		for (auto minion : GEntityList->GetAllMinions (false, true, true))
			{
			if (Extensions::Validate (minion) && !minion->IsWard() && minion->IsCreep() && Extensions::GetDistance (player, minion->GetPosition()) <= 1000)
				{
				if (!minion->IsDead())
					{
					auto health = minion->GetHealth();
					auto hp = GHealthPrediction->GetPredictedHealth (minion, kLastHitPrediction, 1000, 950+GGame->Latency());
					if (hp < qDmg (minion) && hp > health - hp * 2)
						{
						Q->CastOnPosition (minion->GetPosition());
						return;
						}
					Q->CastOnPosition (FarmQ (minion->GetPosition()));
					return;
					}
				}
			}
		}
}


void Karthus::LastHit()
{
	Q->SetOverrideRadius (160.f);
	auto player = Hero;
	if (Q->IsReady())
		{
		for (auto minion : GEntityList->GetAllMinions (false, true, true))
			{
			if (Extensions::Validate (minion) && !minion->IsWard() && minion->IsCreep() && Extensions::GetDistance (player, minion->GetPosition()) <= 1000)
				{
				if (!minion->IsDead())
					{
					auto health = minion->GetHealth();
					auto hp = GHealthPrediction->GetPredictedHealth (minion, kLastHitPrediction, 1000, 950 + GGame->Latency());
					if (hp < qDmg (minion) && hp > health - hp * 2)
						{
						Q->CastOnPosition (minion->GetPosition());
						return;
						}
					}
				}
			}
		}
}


void Karthus::Drawing()
{
	auto player = Hero;
	if (DrawReady->Enabled())
		{
		if (Q->IsReady() && DrawQ->Enabled())
			{
			GRender->DrawOutlinedCircle (player->GetPosition(), Vec4 (0, 255, 0, 225), Q->Range());
			}
		if (W->IsReady() && DrawW->Enabled())
			{
			GRender->DrawOutlinedCircle (player->GetPosition(), Vec4 (0, 255, 0, 225), W->Range());
			}
		if (E->IsReady() && DrawE->Enabled())
			{
			GRender->DrawOutlinedCircle (player->GetPosition(), Vec4 (0, 255, 0, 225), E->Range());
			}
		}
	else
		{
		if (DrawQ->Enabled())
			{
			GRender->DrawOutlinedCircle (player->GetPosition(), Vec4 (0, 255, 0, 225), Q->Range());
			}
		if (DrawW->Enabled())
			{
			GRender->DrawOutlinedCircle (player->GetPosition(), Vec4 (0, 255, 0, 225), W->Range());
			}
		if (DrawE->Enabled())
			{
			GRender->DrawOutlinedCircle (player->GetPosition(), Vec4 (0, 255, 0, 225), E->Range());
			}
		}
}