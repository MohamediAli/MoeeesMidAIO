#include "Orianna.h"
#include <time.h>
#include "Lords.h"
#include "Rembrandt.h"
#include "MEC.h"
#include <tuple>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */



#define M_PI 3.14159265358979323846
bool FlashUlting = false;
IUnit *StationaryBall, *MovingBall;
Vec3 BallPosition;
int Ticks;
auto VecG = std::vector<Vec3>();
float BallRad = 75;
Vec4 BallIndicatorColor (52, 152, 219, 255);
Vec4 BallIndicatorMovingColor (255, 0, 255, 255);
Vec4 GagongColorsMove[9];
Vec3 UpVec (0, 1, 0);
float lastTimeStamp;
float gagongAngle;

Orianna::~Orianna()
{
	OriannaMenu->Remove();
}

struct MovingBallData
{
	Vec3 StartPos;
	Vec3 EndPos;
	float OverAllTime;
	float StartTime;

} QBallData;

struct MovingBallDataW
{
	Vec3 StartPos;
	Vec3 EndPos;
	float OverAllTime;
	float StartTime;

} WBallData;

Orianna::Orianna (IMenu* Parent, IUnit* Hero) :Champion (Parent, Hero)
{
	Q = GPluginSDK->CreateSpell2 (kSlotQ, kCircleCast, true, true, kCollidesWithYasuoWall);
	Q->SetSkillshot (0.f, 80.f, 1400.f, 825.f);
	W = GPluginSDK->CreateSpell2 (kSlotW, kCircleCast, false, true, kCollidesWithNothing);
	W->SetSkillshot (0.f, 220.f, FLT_MAX, 245.f);
	E = GPluginSDK->CreateSpell2 (kSlotE, kTargetCast, false, true, kCollidesWithYasuoWall);
	E->SetSkillshot (0.25f, 80.f, 1900.f, 1095.f);
	R = GPluginSDK->CreateSpell2 (kSlotR, kCircleCast, false, true, kCollidesWithNothing);
	R->SetSkillshot (0.6f, 365.f, FLT_MAX, 325.f);
	RFlash = GPluginSDK->CreateSpell2 (kSlotR, kLineCast, false, false, static_cast<eCollisionFlags> (kCollidesWithNothing));
	RFlash->SetSkillshot (0.25f, 60, FLT_MAX, 775);
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
	OriannaMenu = Parent->AddMenu ("Orianna Menu");
	qMenu = Parent->AddMenu ("Q Settings");
	wMenu = Parent->AddMenu ("W Settings");
	eMenu = Parent->AddMenu ("E Settings");
	rMenu = Parent->AddMenu ("R Settings");
	oneVerusOne = Parent->AddMenu ("1v1 Settings");
	LaneClearMenu = Parent->AddMenu ("Lane Clear");
	Prediction = Parent->AddMenu ("Prediction");
	MiscMenu = Parent->AddMenu ("Miscs");
	Drawings = Parent->AddMenu ("All Drawings");
	DivineBall = Parent->AddMenu ("Divine Ball Colors");
	killStealQ = qMenu->CheckBox ("Kill Steal with Q", true);
	ComboQ = qMenu->CheckBox ("Use Q in Combo", true);
	autoQ = qMenu->CheckBox ("Automatic Harass Q", false);
	harassQ = qMenu->CheckBox ("Harass with Q", true);
	harassQMana = qMenu->AddFloat ("^-> Only Harras Q if Mana >", 0, 100, 50);
	gapcloseQ = qMenu->CheckBox ("Use Q on Gap Closers", true);
	killStealW = wMenu->CheckBox ("Kill Steal with W", true);
	ComboW = wMenu->CheckBox ("Use W in Combo", true);
	autoW = wMenu->CheckBox ("Automatic Harass W", false);
	harassW = wMenu->CheckBox ("Harass with W", true);
	harassWMana = wMenu->AddFloat ("^-> Only Harras W if Mana >", 0, 100, 50);
	ComboE = eMenu->CheckBox ("Use E in Combo", true);
	killStealE = eMenu->CheckBox ("Kill Steal with E", true);
	harassE = eMenu->CheckBox ("Harass with E", false);
	HealthPercentage = eMenu->AddFloat ("Shield  Self if Health Percent Below: ", 0, 100, 30);
	ShieldTeamate = eMenu->CheckBox ("Shield Teammates", true);
	ShieldTeamatePercent = eMenu->AddFloat ("^->Shield Teammate if Health Percent Below: ", 0, 100, 30);
	autoEiniti = eMenu->CheckBox ("Automatically Shield Initiating Teammates", true);
	eHelper = eMenu->CheckBox ("E Assist", true);
	eHelperKey = eMenu->AddKey ("E Assist Key", 69);
	ComboR = rMenu->CheckBox ("Use Ult in Combo", true);
	ultMin = rMenu->AddInteger ("Only Ult if it will hit atleast: ", 0, 5, 2);
	BlockR = rMenu->CheckBox ("Block R on no hits", true);
	FlashUlt = rMenu->AddKey ("Flash Ult key", 84);
	InterruptR = rMenu->CheckBox ("Use Ult to Interrupt Spells", true);
	KillStealR = rMenu->CheckBox ("Use Ult to Kill Steal", false);
	priorityMin = rMenu->AddInteger ("Automatically R if Priority >=", 0, 5, 5);
	onev1 = oneVerusOne->CheckBox ("Enable 1v1 Mode", true);
	extraAutos = oneVerusOne->AddInteger ("Add Extra Autos in Ult Damage Calc", 0, 5, 2);
	Laneclear = LaneClearMenu->CheckBox ("Use Spells in Lane Clear", true);
	laneClearQ = LaneClearMenu->CheckBox ("Wave Clear with Q", true);
	laneClearQMana = LaneClearMenu->AddFloat ("^-> Only Wave Clear Q if Mana >", 0, 100, 50);
	laneClearW = LaneClearMenu->CheckBox ("Wave Clear with W", true);
	laneClearWMana = LaneClearMenu->AddFloat ("^-> Only Wave Clear W if Mana >", 0, 100, 50);
	mouseClear = LaneClearMenu->CheckBox ("Mouse Scroll to Toggle Wave Clear", true);
	PredType = { "Oracle", "Core", };
	PredictionType = Prediction->AddSelection ("Choose Prediction Type", 0, PredType);
	ballAnimation = { "Divine Nader [Sl]", "Gagondix" };
	DrawReady = Drawings->CheckBox ("Draw Ready Spells", true);
	drawDmg = Drawings->CheckBox ("Draw Damage", true);
	HPBarColor = Drawings->AddColor ("Change Health Bar", 69, 64, 185, 100);
	DrawQ = Drawings->CheckBox ("Draw Q", true);
	DrawW = Drawings->CheckBox ("Draw W", true);
	DrawE = Drawings->CheckBox ("Draw E", true);
	DrawR = Drawings->CheckBox ("Draw R", true);
	drawLC = Drawings->CheckBox ("Draw Lance Clear Status", true);
	drawBall = Drawings->CheckBox ("Draw Ball Animation", true);
	ballSelect = Drawings->AddSelection ("Choose Ball Style", 0, ballAnimation);
	ballColor = { "Normal", "Random", "DISCO" };
	RandomMode = DivineBall->AddSelection ("Choose Color Pattern", 0, ballColor);
	DivineColor1 = DivineBall->AddColor ("Divine Ball Color Line 1", 138, 195, 202, 100);
	DivineColor2 = DivineBall->AddColor ("Divine Ball Color Line 2", 69, 29, 195, 100);
	DivineColor3 = DivineBall->AddColor ("Divine Ball Color Line 3", 127, 140, 185, 100);
	DivineColor4 = DivineBall->AddColor ("Divine Ball Color Line 4", 60, 169, 185, 100);
	DivineColor5 = DivineBall->AddColor ("Divine Ball Color Line 5", 49, 64, 255, 100);
	DivineColor6 = DivineBall->AddColor ("Divine Ball Color Line 6", 104, 94, 240, 100);
	DivineColor7 = DivineBall->AddColor ("Divine Ball Color Line 7", 71, 131, 170, 100);
	DivineColor8 = DivineBall->AddColor ("Divine Ball Color Line 8", 122, 171, 255, 100);
}

Vec3 Orianna::getPosToRflash (Vec3 target)
{
	return  Hero->ServerPosition().Extend (GGame->CursorPosition(), Flash->Range());
}

void Orianna::CastFlash()
{
	auto target = GTargetSelector->GetFocusedTarget() != nullptr
	              ? GTargetSelector->GetFocusedTarget()
	              : GTargetSelector->FindTarget (QuickestKill, SpellDamage, RFlash->Range());
	Flash->CastOnPosition (getPosToRflash (target->GetPosition()));
}

void Orianna::PerformFlashUlt()
{
	GGame->IssueOrder (Hero, kMoveTo, GGame->CursorPosition());
	auto target1 = GTargetSelector->GetFocusedTarget();
	if (target1 == nullptr || !target1->IsHero() || target1->IsDead())
	{
		return;
	}
	if (R->IsReady() && Flash->IsReady() && StationaryBall == Hero)
	{
		auto target = GTargetSelector->GetFocusedTarget() != nullptr
		              ? GTargetSelector->GetFocusedTarget()
		              : GTargetSelector->FindTarget (QuickestKill, SpellDamage, RFlash->Range());
		//auto flashPosition = Hero->ServerPosition().Extend (GGame->CursorPosition(), Flash->Range());
		AdvPredictionOutput result;
		RFlash->RunPrediction (target, false, kCollidesWithNothing, &result);
		if (target != nullptr && target->IsValidTarget() && !target->IsDead() && !target->IsInvulnerable() && result.HitChance >= kHitChanceVeryHigh)
		{
			FlashUlting = true;
			RFlash->CastOnTarget (target, kHitChanceVeryHigh);
			GPluginSDK->DelayFunctionCall (500 + (GGame->Latency()) / 2, [=]()
			{
				CastFlash();
				FlashUlting = false;
			});
		}
	}
}


void Orianna::OnNewPath (IUnit* Source, const std::vector<Vec3>& path_)
{
	OnRunPath (Source, path_);
	auto target = GTargetSelector->FindTarget (QuickestKill, SpellDamage, Q->Range());
	if (target == nullptr || !target->IsHero() || target->IsDead() || isBallMoving() || !target->IsVisible())
	{
		return;
	}
	if (target == Source && GOrbwalking->GetOrbwalkingMode() == kModeCombo)
		if (Q->IsReady() && ComboQ->Enabled() && !isChasing (target) && R->IsReady() && Extensions::EnemiesInRange (target->ServerPosition(), R->Radius() * 2) > 1)
		{
			TeamFightQ (target->GetPosition());
			return;
		}
		else if (Q->IsReady() && target == Source && ComboQ->Enabled() && Hero->IsValidTarget (target, Q->Range()))
		{
			// do a normal cast not aoe one
			CastQ (target);
			return;
		}
	if (autoQ->Enabled() && Q->IsReady())
	{
		auto target1 = GTargetSelector->FindTarget (QuickestKill, SpellDamage, Q->Range());
		if (target1 == nullptr || !target1->IsHero() || !target1->IsValidTarget() || !target1->IsVisible())
		{
			return;
		}
		CastQ (target1);
	}
}

void Orianna::eAssist()
{
	if (eHelper->Enabled() && E->IsReady())
	{
		auto bestEqTravelTime = FLT_MAX;
		std::vector<std::pair<int, IUnit*>> bestAlly;
		for (auto ally : GEntityList->GetAllHeros (true, false))
		{
			if (!ally->IsDead() && ally != nullptr)
			{
				int distanceA = Extensions::GetDistance (Hero, ally);
				int distanceB = Extensions::GetDistance (ally, GGame->CursorPosition());
				if (distanceA < 2000 && distanceB < 420)
				{
					bestAlly.push_back (std::make_pair (distanceB, ally));
				}
			}
			std::sort (bestAlly.begin(), bestAlly.end(), [] (auto &left, auto &right)
			{
				return left.first < right.first;
			});
		}
		for (auto entry : bestAlly)
		{
			if (entry.second != nullptr)
			{
				CastE (entry.second);
				return;
			}
		}
	}
}

void Orianna::OnGameUpdate()
{
	if (RandomMode->GetInteger() == 2)
	{
		srand ( (GGame->TickCount()));
		Color1 = Vec4 (rand() % 255, rand() % 255, rand() % 255, 255);
		Color2 = Vec4 (rand() % 255, rand() % 255, rand() % 255, 255);
		Color3 = Vec4 (rand() % 255, rand() % 255, rand() % 255, 255);
		Color4 = Vec4 (rand() % 255, rand() % 255, rand() % 255, 255);
		Color5 = Vec4 (rand() % 255, rand() % 255, rand() % 255, 255);
		Color6 = Vec4 (rand() % 255, rand() % 255, rand() % 255, 255);
		Color7 = Vec4 (rand() % 255, rand() % 255, rand() % 255, 255);
		Color8 = Vec4 (rand() % 255, rand() % 255, rand() % 255, 255);
	}
	else if (RandomMode->GetInteger() == 1)
	{
		srand (time (NULL));
		Color1 = Vec4 (rand() % 255, rand() % 255, rand() % 255, 255);
		Color2 = Vec4 (rand() % 255, rand() % 255, rand() % 255, 255);
		Color3 = Vec4 (rand() % 255, rand() % 255, rand() % 255, 255);
		Color4 = Vec4 (rand() % 255, rand() % 255, rand() % 255, 255);
		Color5 = Vec4 (rand() % 255, rand() % 255, rand() % 255, 255);
		Color6 = Vec4 (rand() % 255, rand() % 255, rand() % 255, 255);
		Color7 = Vec4 (rand() % 255, rand() % 255, rand() % 255, 255);
		Color8 = Vec4 (rand() % 255, rand() % 255, rand() % 255, 255);
	}
	else if (RandomMode->GetInteger() == 0)
	{
		DivineColor1->GetColor (&Color1);
		DivineColor2->GetColor (&Color2);
		DivineColor3->GetColor (&Color3);
		DivineColor4->GetColor (&Color4);
		DivineColor5->GetColor (&Color5);
		DivineColor6->GetColor (&Color6);
		DivineColor7->GetColor (&Color7);
		DivineColor8->GetColor (&Color8);
	}
	//debug key
	if (GGame->IsChatOpen() || !GUtility->IsLeagueWindowFocused() || Hero->IsDead())
	{
		return;
	}
	Automatic();
	if (GetAsyncKeyState (eHelperKey->GetInteger()))
	{
		eAssist();
	}
	if (GOrbwalking->GetOrbwalkingMode() == kModeCombo)
	{
		Combo();
	}
	else if (GOrbwalking->GetOrbwalkingMode() == kModeMixed)
	{
		Harass();
	}
	else if (GOrbwalking->GetOrbwalkingMode() == kModeLaneClear)
	{
		LaneClear();
	}
}


void Orianna::OnRender()
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

bool Orianna::OnPreCast (int Slot, IUnit* Target, Vec3* StartPosition, Vec3* EndPosition)
{
	if (Slot == kSlotR && BlockR->Enabled() && FlashUlting == false)
	{
		if (SpellCheck (StationaryBall, R->Radius(), 0.2) == 0)
		{
			return false;
		}
	}
	if (Slot == kSlotW && R->IsReady() && ComboR->Enabled() && SpellCheck (StationaryBall, R->Radius(), R->GetDelay()) >= ultMin->GetInteger())
	{
		return false;
	}
	return true;
}

///Thanks Divine for Indentation Tips.
//Author Divine.
void Orianna::OnInterrupt (InterruptibleSpell const& Args)
{
	auto player = Hero;
	if (Args.Source == nullptr || Args.Source->IsDead() || Args.DangerLevel != kHighDanger)
	{
		return;
	}
	if (Q->IsReady() && player->IsValidTarget (Args.Source, Q->Range()) && Args.Source != nullptr && Args.Source != Hero && Args.Source->IsEnemy (Hero))
	{
		Q->CastOnPosition (Args.Source->ServerPosition());
	}
	if (R->IsReady() && InterruptR->Enabled() && SpellCheckKS (StationaryBall, R->Range(), 0.5, Args.Source))
	{
		R->CastOnPlayer();
	}
}

void Orianna::AntiGapclose (GapCloserSpell const& args)
{
	if (Q->IsReady() && args.Source->IsValidTarget() && !Hero->IsDead() && args.Source->IsEnemy (Hero) && gapcloseQ->Enabled())
	{
		if (Extensions::GetDistance (Hero, args.EndPosition) <= Q->Range())
		{
			auto delay = (GBuffData->GetEndTime (args.Data) - GGame->Time() - Q->GetDelay());
			//Thanks Divine for fixing my stupid logic <3.
			GPluginSDK->DelayFunctionCall (delay * 1000, [=]()
			{
				Q->CastOnPosition (args.EndPosition);
			});
		}
	}
}


int Orianna::GetEHits()
{
	auto player = Hero;
	auto hits = std::vector <IUnit*>();
	for (auto enemy : GEntityList->GetAllHeros (false, true))
	{
		if (player->IsValidTarget (enemy, 2000))
		{
			Vec3 position;
			GPrediction->FindBestCastPositionEx (player->GetPosition(), 0, E->Range(), E->Radius(), true, false, true, position, hits);
			if (Extensions::GetDistance (position, StationaryBall->GetPosition()) < 100)
			{
				return hits.size();
			}
		}
	}
	hits.clear();
	return 0;
}

//Author Divine ( you should pay a lot of money , when this is done, i dont want profit % since prob you wont sell shit, i need static payment, ty <3 :*
bool Orianna::PriorityHit()
{
	Vec3 futurePos;
	auto Targets = GEntityList->GetAllHeros (false, true);
	for (auto target : Targets)
	{
		if (Extensions::Validate (StationaryBall) && Extensions::Validate (target) && !target->IsDead() && target->IsHero() && target->IsVisible())
		{
			GPrediction->GetFutureUnitPosition (target, 0.5f, true, futurePos);
			auto flDistance = (futurePos - StationaryBall->GetPosition()).Length();
			if (flDistance <= R->Range())
			{
				if (GTargetSelector->GetHeroPriority (target) >= priorityMin->GetInteger())
				{
					return true;
				}
			}
		}
	}
	return false;
}


//t[0,1]


//Author= Divine[NaderSl]
BOOL Orianna::isBallMoving()
{
	return 	GGame->Time() - QBallData.StartTime < QBallData.OverAllTime;
}

float Orianna::BallDelay()
{
	if (GGame->Time() - QBallData.StartTime < QBallData.OverAllTime)
	{
		return (GGame->Time() - QBallData.StartTime);
	}
	else
	{
		return 0;
	}
}

//Divine[NaderSl]
Vec3 Orianna::GetMovingBallPos()
{
	float EndTime = QBallData.StartTime + QBallData.OverAllTime;
	Vec3 CurrPos = Extensions::lerp ( (EndTime - GGame->Time()) / QBallData.OverAllTime, QBallData.EndPos, QBallData.StartPos);
	return Vec3 (CurrPos.x, GNavMesh->GetHeightForPoint (Vec2 (CurrPos.x, CurrPos.z)) + 110, CurrPos.z);
}

Vec3 Orianna::GetMovingBallPosW()
{
	float EndTime = WBallData.StartTime + WBallData.OverAllTime;
	Vec3 CurrPos = Extensions::lerp ( (EndTime - GGame->Time()) / WBallData.OverAllTime, WBallData.EndPos, WBallData.StartPos);
	if ( (EndTime - GGame->Time()) < 0.1)
	{
		return Vec3 (CurrPos.x, GNavMesh->GetHeightForPoint (Vec2 (CurrPos.x, CurrPos.z)) + 110, CurrPos.z);
	}
	else { return Vec3 (0, 0, 0); }
}


void Orianna::OnSpellCast (CastedSpell const& args)
{
	if (!args.Caster_->IsEnemy (Hero))
	{
		if (std::string (args.Name_) == "OrianaIzunaCommand")
		{
			//Author= Divine[NaderSl]
			if (&args.Position_ && &args.EndPosition_ && args.Speed_)
			{
				// v = d/t => t = d/v
				Vec3 StartPos = (StationaryBall ? StationaryBall->GetPosition() : args.Position_);
				float dist = (args.EndPosition_ - StartPos).Length();
				QBallData = { StartPos,args.EndPosition_, (dist / args.Speed_), GGame->Time() };
				WBallData = { StartPos,args.EndPosition_, (dist / args.Speed_), GGame->Time() };
			}
		}
		if (std::string (args.Name_) == "OrianaRedactCommand")
		{
			//Author= Divine[NaderSl]
			if (&args.Position_ && &args.EndPosition_ && args.Speed_)
			{
				// v = d/t => t = d/v
				Vec3 StartPos = (StationaryBall ? StationaryBall->GetPosition() : args.Position_);
				float dist = (args.EndPosition_ - StartPos).Length();
				QBallData = { StartPos,args.EndPosition_, (dist / 1900), GGame->Time() };
			}
		}
		if (Extensions::GetDistance (Hero, args.Caster_->GetPosition()) <= E->Range() && autoEiniti->Enabled() && args.Caster_->IsValidTarget() && E->IsReady() && !args.Caster_->IsEnemy (Hero))
		{
			std::vector<std::string> SpellNames =
			{
				"ShenE",
				"JaxLeapStrike",
				"AatroxQ",
				"AkaliShadowDance",
				"HeadButt",
				"BandageToss",
				"DianaTeleport",
				"EkkoE",
				"EliseSpidereInitial",
				"CamilleE",
				"KledR",
				"KledE",
				"IvernQ",
				"IllaoiW",
				"Crowstorm",
				"FioraQ",
				"GnarE",
				"GnarBigE",
				"GragasE",
				"HecarimUlt",
				"IreliaGatotsu",
				"JarvanIVCataclysm",
				"JarvanIVDragonStrike",
				"KatarinaE",
				"KennenLightningRush",
				"KhazixE",
				"LeblancSlide",
				"LeblancSlideM",
				"LeonaZenithBlade",
				"MaokaiUnstableGrowth",
				"NocturneParanoia",
				"OlafRagnarok",
				"SionR",
				"RengarR",
				"ShyvanaTransformCast",
				"ShyvanaTransformLeap",
				"ThreshQLeap",
				"WarwickR",
				"GallioE",
				"ZacE",
				"LucianE",
				"MonkeyKingNimbus",
				"NautilusAnchorDrag",
				"Pantheon_LeapBash",
				"PoppyHeroicCharge",
				"QuinnE",
				"RenektonSliceAndDice",
				"RiftWalk",
				"RivenTriCleave",
				"RocketJump",
				"SejuaniArcticAssault",
				"TalonCutThroat",
				"UFSlash",
				//  "UdyrBearStance",
				"KatarinaE",
				"Valkyrie",
				"ViQ",
				"ViR",
				"VolibearQ",
				"XenZhaoSweep",
				"YasuoDashWrapper",
				"blindmonkqtwo",
				"khazixelong",
				"reksaieburrowed",
				"TryndamereE"
			};
			for (auto spellName : SpellNames)
			{
				if (std::string (args.Name_) == spellName)
				{
					E->CastOnTarget (args.Caster_);
				}
			}
		}
	}
}


void Orianna::OnCreate (IUnit* object)
{
	auto objectName = object->GetObjectName();
	if (object->IsMissile() && GMissileData->GetCaster (object) == GEntityList->Player())
	{
		if (strcmp (GMissileData->GetName (object), "OrianaIzuna") == 0 || strcmp (GMissileData->GetName (object), "OrianaRedact") == 0)
		{
			BallMissile = object;
		}
	}
	if (strcmp (object->GetObjectName(), "Orianna_Base_Z_Izuna_nova.troy") == 0)
	{
		BallPosition = object->GetPosition();
	}
	//auto player = Hero;
	//	GUtility->CreateDebugConsole();
	//	GUtility->LogConsole(objectName);
	if (object->IsValidObject() && object->IsVisible())
	{
		if ( (!strcmp (objectName, "Orianna_Base_Z_ball_glow_green.troy")))
		{
			StationaryBall = object;
		}
		else  if ( (!strcmp (objectName, "Orianna_Base_Q_yomu_ring_green.troy")))
		{
			MovingBall = object;
		}
	}
}

bool Orianna::pairCompare (const std::pair<int, Vec2>& firstElem, const std::pair<int, Vec2>& secondElem)
{
	return firstElem.first < secondElem.first;
}


void Orianna::TeamFightQ (Vec3 pos)
{
	auto posChecked = 0;
	auto maxRange = R->Radius() + Q->Range();
	auto posRadius = 35;
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
			auto xPos = static_cast<float> (floor (pos.x + curRadius * cos (cRadians)));
			auto zPos = static_cast<float> (floor (pos.z + curRadius * sin (cRadians)));
			auto posFor2D = Vec2 (xPos, zPos);
			auto count = Extensions::EnemiesInRange (Extensions::To3D (posFor2D), R->Radius());
			//GGame->PrintChat (std::to_string (count).c_str());
			if (GNavMesh->IsPointWall (Extensions::To3D (posFor2D)))
			{
				// dont push is wall
				continue;
			}
			if (Extensions::Dist2D (posFor2D, GEntityList->Player()->ServerPosition()) > Q->Range())
			{
				// dont push to far away to cast;
				continue;
			}
			if (Extensions::Dist2D (posFor2D, pos) <= R->Radius() - 20)
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
		Q->CastOnPosition (Extensions::To3D (entry.second));
	}
	return;
}

void Orianna::FarmQ (Vec3 pos)
{
	auto posChecked = 0;
	auto maxRange = R->Radius() + Q->Range();
	auto posRadius = 65;
	auto maxPosChecked = (int) (maxRange / posRadius);
	auto radiusIndex = 0;
	bool menu;
	std::vector<std::pair<int, Vec2>> CloseQPositions;
	std::vector<std::pair<int, Vec2>> possibleQPositions;
	while (posChecked < maxPosChecked)
	{
		radiusIndex++;
		auto curRadius = radiusIndex * (0x00002 * posRadius);
		auto curCurcleChecks = static_cast<int> (ceil ( (0x2 * M_PI * curRadius) / (0x2 * static_cast<double> (posRadius))));
		for (auto i = 0x1; i < curCurcleChecks; i++)
		{
			posChecked++;
			auto cRadians = (0x2 * M_PI / (curCurcleChecks - 0x1)) * i;
			auto xPos = static_cast<float> (floor (pos.x + curRadius * cos (cRadians)));
			auto zPos = static_cast<float> (floor (pos.z + curRadius * sin (cRadians)));
			auto posFor2D = Vec2 (xPos, zPos);
			auto count = Extensions::CountMinionsInTargetRange (Extensions::To3D (posFor2D), Q->Radius());
			//GGame->PrintChat(std::to_string(count).c_str());
			if (GNavMesh->IsPointWall (Extensions::To3D (posFor2D)))
			{
				// dont push is wall
				continue;
			}
			if (Extensions::Dist2D (posFor2D, GEntityList->Player()->ServerPosition()) > Q->Range())
			{
				// dont push to far away to cast;
				continue;
			}
			if (Extensions::Dist2D (posFor2D, pos) <= Q->Radius() * 4)
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
	for (auto entry : possibleQPositions)
	{
		Q->CastOnPosition (Extensions::To3D (entry.second));
		return;
	}
}


Vec2 Orianna::vect2d (Vec2 p1, Vec2 p2)
{
	Vec2 temp;
	temp.x = (p2.x - p1.x);
	temp.y = -1 * (p2.y - p1.y);
	return temp;
}
//CREDITS AMBER
bool Orianna::PointInRectangle (Vec2 A, Vec2 B, Vec2 C, Vec2 D, Vec2 m)
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
bool Orianna::IsInRectangle (Vec3 Start, Vec3 End, Vec3 pointTest, int radius)
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

Vec3 Orianna::GetPredictedUnitPosition (IUnit* Unit, ISpell2* Skillshot, float& TravelTime)
{
	if (Unit->GetWaypointList().size() < 2)
	{
		return Unit->GetPosition();
	}
	TravelTime = ( (Unit->GetPosition() - Hero->GetPosition()).Length2D() - Unit->BoundingRadius()) / Skillshot->Speed() + Skillshot->GetDelay() + (GGame->Latency()) / 1000;
	auto Path = Unit->GetWaypointList();
	Vec3 EstimatedMaxPosition;
	EstimatedMaxPosition = (Unit->GetPosition()).Extend (Path.at (1), Unit->MovementSpeed() * TravelTime);
	for (int i = 0; i < 10; i++)
	{
		TravelTime = ( (EstimatedMaxPosition - Hero->GetPosition()).Length2D() - Unit->BoundingRadius()) / Skillshot->Speed() + Skillshot->GetDelay() + (GGame->Latency()) / 1000;
		EstimatedMaxPosition = (Unit->GetPosition()).Extend (Path.at (1), Unit->MovementSpeed() * TravelTime);
	}
	return EstimatedMaxPosition;
}

bool Orianna::CheckForCollision (ISpell2* Skillshot, Vec3 CheckAtPosition)
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

bool Orianna::BestCastPosition (IUnit* Unit, ISpell2* Skillshot, Vec3& CastPosition, bool CheckCollision)
{
	if (Extensions::Validate (StationaryBall) && Extensions::Validate (Unit))
	{
		float TravelTime = ( (Unit->GetPosition() - StationaryBall->GetPosition()).Length2D() - Unit->BoundingRadius()) / Skillshot->Speed() + Skillshot->GetDelay() + (GGame->Latency()) / 1000;
		auto Path = Unit->GetWaypointList();
		if (Path.size() > 1)   //target is moving
		{
			Vec3 EstimatedMaxPosition;
			EstimatedMaxPosition = (Unit->GetPosition()).Extend (Path.at (1), Unit->MovementSpeed() * TravelTime);
			for (int i = 0; i < 10; i++)
			{
				TravelTime = ( (EstimatedMaxPosition - StationaryBall->GetPosition()).Length2D() - Unit->BoundingRadius()) / Skillshot->Speed() + Skillshot->GetDelay() + (GGame->Latency()) / 1000;
				EstimatedMaxPosition = (Unit->GetPosition()).Extend (Path.at (1), Unit->MovementSpeed() * TravelTime);
			}
			CastPosition = EstimatedMaxPosition.Extend (Unit->GetPosition(), Unit->BoundingRadius() + Skillshot->Radius() * 0.8);
		}
		else if (Extensions::Validate (Unit))
		{
			CastPosition = Unit->GetPosition();
		}
		else
		{
			return false;
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
	return false;
}

int Orianna::SpellCheck (IUnit* pos, int range, float delay)
{
	auto count = 0;
	Vec3 futurePos;
	if (Extensions::Validate (pos))
	{
		if (pos->IsMissile())
		{
			for (auto target : GEntityList->GetAllHeros (false, true))
			{
				if (Extensions::Validate (target) && target->IsVisible() && !target->IsDead() && target->IsHero())
				{
					GPrediction->GetFutureUnitPosition (target, delay, true, futurePos);
					if ( (futurePos - GMissileData->GetEndPosition (pos)).LengthSqr() <= range*range)
					{
						count++;
					}
				}
			}
		}
		else
		{
			for (auto target : GEntityList->GetAllHeros (false, true))
			{
				if (Extensions::Validate (target) && target->IsVisible() && !target->IsDead() && target->IsHero())
				{
					GPrediction->GetFutureUnitPosition (target, delay, true, futurePos);
					if ( (futurePos - pos->GetPosition()).LengthSqr() <= range*range)
					{
						count++;
					}
				}
			}
		}
	}
	return (count);
}

bool Orianna::SpellCheckKS (IUnit* pos, int range, float delay, IUnit* target)
{
	Vec3 futurePos;
	if (Extensions::Validate (pos))
	{
		if (pos->IsMissile())
		{
			if (target != nullptr && target->IsValidTarget() && isBallMoving() && target->IsVisible() && !target->IsDead())
			{
				GPrediction->GetFutureUnitPosition (target, delay, true, futurePos);
				if ( (futurePos - GMissileData->GetEndPosition (pos)).Length() <= range)
				{
					return true;
				}
			}
		}
		else if (target != nullptr && target->IsValidTarget() && target->IsVisible() && !target->IsDead())
		{
			GPrediction->GetFutureUnitPosition (target, delay, true, futurePos);
			if ( (futurePos - pos->GetPosition()).Length() <= range)
			{
				return true;
			}
		}
	}
	return false;
}



void Orianna::CastE (IUnit* target)
{
	auto player = Hero;
	if (target != nullptr && E->IsReady() && (target->IsHero() && player->GetMana() > Q->ManaCost() + W->ManaCost() + E->ManaCost()))
	{
		E->CastOnTarget (target);
	}
}

std::tuple<int, Vec3> Orianna::GetBestQLocation (IUnit* mainTarget)
{
	auto points = std::vector<Vec2>();
	AdvPredictionOutput result;
	Q->RunPrediction (mainTarget, true, kCollidesWithYasuoWall, &result);
	auto qPrediction = result;
	if (qPrediction.HitChance < kHitChanceHigh)
	{
		return  std::tuple<int, Vec3> (1, Vec3 (0, 0, 0));
	}
	points.push_back (qPrediction.TargetPosition.To2D());
	auto enemy = GEntityList->GetAllHeros (false, true);
	for (IUnit* enemy : enemy)
	{
		if (enemy->IsValidTarget() && Extensions::GetDistance (Hero, enemy) < Q->Range())
		{
			AdvPredictionOutput result2;
			Q->RunPrediction (enemy, true, kCollidesWithYasuoWall, &result2);
			if (result.HitChance >= kHitChanceHigh)
			{
				points.push_back (result2.TargetPosition.To2D());
			}
		}
	}
	for (int j = 0; j < 5; j++)
	{
		auto mecResult = MEC::GetMec (points);
		if (mecResult.Radius < (R->Range() - 75) && points.size() >= 3 && R->IsReady())
		{
			return  std::tuple<int, Vec3> (3, Extensions::To3D (mecResult.Center));
		}
		if (mecResult.Radius < (W->Range() - 75) && points.size() >= 2 && W->IsReady())
		{
			return std::tuple<int, Vec3> (2, Extensions::To3D (mecResult.Center));
		}
		if (points.size() == 1)
		{
			return std::tuple<int, Vec3> (1, Extensions::To3D (mecResult.Center));
		}
		if (mecResult.Radius < Q->Radius() && points.size() == 2)
		{
			return  std::tuple<int, Vec3> (2, Extensions::To3D (mecResult.Center));
		}
		float maxdist = -1;
		auto maxdistindex = 1;
		for (auto i = 1; i < points.size(); i++)
		{
			auto distance = (Extensions::GetDistanceVectors (Extensions::To3D (points[i]), Extensions::To3D (points[0])));
			if (distance > maxdist || maxdist == -1)
			{
				maxdistindex = i;
				maxdist = distance;
			}
		}
		points.erase (points.begin() + maxdistindex);
	}
	return  std::tuple<int, Vec3> (1, Extensions::To3D (points[0]));
}

/* void Orianna::SaveTeam()
{
if (E->IsReady() && !(Hero->IsDead()))
{
if (Hero->HealthPercent() <= HealthPercent->GetInteger() && Extensions::EnemiesInRange(Hero->GetPosition(), 600) > 0) {
E->CastOnPlayer();
} //Cast on self

if (ShieldTeamate->Enabled())
{
auto Teamates = GEntityList->GetAllHeros(true, false);
for (IUnit* Teamate : Teamates)
{
if (Extensions::GetDistance(Hero, Teamate->GetPosition()) <= E->Range()) {
if (!(Teamate->IsDead()) && Teamate->HealthPercent() <= ShieldTeamatePercent->GetInteger() && Extensions::EnemiesInRange(Teamate->GetPosition(), 600) > 0) {
E->CastOnUnit(Teamate);
} //Cast on injured teamate
}
}*/

void Orianna::CastQ (IUnit* target)
{
	if (!Q->IsReady())
	{
		return;
	}
	float distance = Extensions::GetDistance (StationaryBall->GetPosition(), target->ServerPosition());
	auto player = Hero;
	if (E->IsReady() && player->GetMana() > R->ManaCost() + Q->ManaCost() + W->ManaCost() + E->ManaCost() && distance > Extensions::GetDistance (player->GetPosition(), target->ServerPosition()) + 330)
	{
		E->CastOnPlayer();
		return;
	}
	Vec3 castOn;
	BestCastPosition (target, Q, castOn, false);
	if (ComboE->Enabled() && E->IsReady() && Extensions::GetDistance (target, StationaryBall) >= 250)
	{
		auto directTravelTime = Extensions::GetDistance (StationaryBall, castOn) / Q->Speed();
		auto bestEqTravelTime = FLT_MAX;
		std::vector<std::pair<int, IUnit*>> bestAlly;
		for (auto ally : GEntityList->GetAllHeros (true, false))
		{
			if (!ally->IsDead() && ally != nullptr && ally != Hero)
			{
				int distanceA = Extensions::GetDistance (Hero, ally);
				if (distanceA < E->Range())
				{
					bestAlly.push_back (std::make_pair (distanceA, ally));
				}
			}
			std::sort (bestAlly.begin(), bestAlly.end(), [] (auto &left, auto &right)
			{
				return left.first < right.first;
			});
		}
		for (auto entry : bestAlly)
		{
			if (entry.second != nullptr)
			{
				auto t = Extensions::GetDistance (StationaryBall, entry.second->ServerPosition()) / E->Speed() +
				         Extensions::GetDistance (entry.second, castOn) / Q->Speed();
				if (t < bestEqTravelTime)
				{
					bestEqTravelTime = t;
				}
			}
			if (entry.second != nullptr && bestEqTravelTime < directTravelTime * 1.3f)
			{
				CastE (entry.second);
				return;
			}
		}
	}
	if (PredictionType->GetInteger() == 2)
	{
		QCast (Q, target);
	}
	else if (PredictionType->GetInteger() == 1)
	{
		Q->CastOnTarget (target, kHitChanceHigh);
	}
	else if (PredictionType->GetInteger() == 3)
	{
		auto cast = GetBestQLocation (target);
		if (std::get<int> (cast) > 1)
		{
			Q->CastOnPosition (std::get<Vec3> (cast));
		}
		else
		{
			Q->CastOnTarget (target, kHitChanceVeryHigh);
		}
	}
	else if (PredictionType->GetInteger() == 0 && Extensions::Validate (target))
	{
		Vec3 castOn2;
		BestCastPosition (target, Q, castOn2, false);
		if (Extensions::GetDistance (castOn2, target->ServerPosition()) <150)
		{
			Q->CastOnPosition (castOn2);
		}
	}
}

bool Orianna::IsOneVsOne()
{
	if (Extensions::EnemiesInRange (Hero->GetPosition(), 1000) == 1 && Extensions::AlliesInRange (Hero->GetPosition(), 950) <= 3)
	{
		return true;
	}
	return false;
}


bool Orianna::isChasing (IUnit* Target)
{
	if (!Target->IsFacing (Hero) && Hero->IsFacing (Target))
	{
		return true;
	}
	else
	{
		return false;
	}
}

void Orianna::eLogic()
{
	auto player = Hero;//sebby start
	if (isBallMoving() || PriorityHit() || !E->IsReady() || SpellCheck (BallMissile, R->Radius(), R->GetDelay() + 0.5) >= ultMin->GetInteger() || Extensions::Validate (StationaryBall) && SpellCheck (StationaryBall, R->Radius(), R->GetDelay()) >= ultMin->GetInteger())
	{
		return;
	}
	auto ballEnemies = Extensions::EnemiesInRange (StationaryBall->GetPosition(), 600);
	int playerEnemies = 0;
	if (player != StationaryBall)
	{
		playerEnemies = Extensions::EnemiesInRange (player->GetPosition(), 600);
	}
	if (!Q->IsReady() && player->GetSpellRemainingCooldown (kSlotW) < player->GetSpellRemainingCooldown (kSlotQ) &&
	    (ballEnemies > playerEnemies || ballEnemies == playerEnemies))
	{
		return;
	}
	if (GetEHits() > (ballEnemies <= 2 ? 1 : 2))   //sebby end
	{
		CastE (player);
		return;
	}
	for (auto ally : GEntityList->GetAllHeros (true, false))
	{
		if (Extensions::EnemiesInRange (ally->GetPosition(), R->Radius() * 2) >= 2 && (ally->GetPosition() - player->GetPosition()).Length() <= E->Range() && R->IsReady() && !isBallMoving() && Hero != ally)
		{
			CastE (ally);
			return;
		}
	}
	if (!Hero->IsDead() && Hero->HealthPercent() <= HealthPercentage->GetInteger() && Extensions::EnemiesInRange (Hero->GetPosition(), 600) > 0)
	{
		E->CastOnPlayer();
		return;
	}
}

void Orianna::Combo()
{
	if (W->IsReady() && ComboW->Enabled() && ( (isBallMoving() && Extensions::EnemiesInRange (GetMovingBallPosW(), W->Radius() - 45)) || (Extensions::Validate (StationaryBall) && Extensions::EnemiesInRange (StationaryBall->GetPosition(), W->Radius()))))
	{
		W->CastOnPlayer();
	}
	eLogic();
	auto target1 = GTargetSelector->FindTarget (QuickestKill, SpellDamage, E->Range() + R->Radius() * 2);
	if (target1 == nullptr || !target1->IsHero() || target1->IsDead() || isBallMoving() || !target1->IsVisible())
	{
		return;
	}
	if (R->IsReady() && ComboR->Enabled() && Extensions::Validate (StationaryBall))
	{
		if (PriorityHit())
		{
			R->CastOnPlayer();
			return;
		}
		else if (SpellCheck (StationaryBall, R->Radius(), 0.5) >= ultMin->GetInteger())
		{
			R->CastOnPlayer();
			return;
		}
		else if (onev1->Enabled() && IsOneVsOne() && rD1v1 (target1) >= target1->GetHealth() && (SpellCheckKS (StationaryBall, R->Range(), 0.5, target1)))
		{
			R->CastOnPlayer();
			return;
		}
		else if (KillStealR->Enabled() && rDmg (target1) >= target1->GetHealth() && (SpellCheckKS (StationaryBall, R->Range(), 0.5, target1)))
		{
			R->CastOnPlayer();
			return;
		}
	}
	// check if more than X target to try aoe q position
	//Cast on self
	/*	if (ShieldTeamate->Enabled())
	{
	auto Teamates = GEntityList->GetAllHeros(true, false);
	for (IUnit* Teamate : Teamates)
	{
	if (Extensions::GetDistance(Hero, Teamate->GetPosition()) <= E->Range()) {
	if (!(Teamate->IsDead()) && Teamate->HealthPercent() <= ShieldTeamatePercent->GetInteger() && Extensions::EnemiesInRange(Teamate->GetPosition(), 600) > 0) {
	E->CastOnUnit(Teamate);
	} //Cast on injured teamate
	}
	}
	}*/
}




void Orianna::Harass()
{
	if (harassE->Enabled())
	{
		eLogic();
	}
	if (W->IsReady() && harassW->Enabled() && Hero->ManaPercent() > harassWMana->GetFloat() && ( (isBallMoving() && Extensions::EnemiesInRange (GetMovingBallPosW(), W->Radius() - 45)) || (Extensions::Validate (StationaryBall) && Extensions::EnemiesInRange (StationaryBall->GetPosition(), W->Radius()))))
	{
		W->CastOnPlayer();
	}
	auto target = GTargetSelector->FindTarget (QuickestKill, SpellDamage, Q->Range());
	if (target == nullptr || !target->IsHero() || !target->IsValidTarget())
	{
		return;
	}
	if (harassQ->Enabled() && Hero->ManaPercent() > harassQMana->GetFloat())
	{
		if (Q->IsReady() && target != nullptr && Hero->IsValidTarget (target, Q->Range()) && Extensions::EnemiesInRange (target->ServerPosition(), R->Radius() * 2) > 1)
		{
			(TeamFightQ (target->ServerPosition()));
		}
		else if (Q->IsReady() && harassQ->Enabled() && Hero->IsValidTarget (target, Q->Range()))
		{
			CastQ (target);
		}
	}
}


bool Orianna::onMouseWheel (HWND wnd, UINT message, WPARAM wparam, LPARAM lparam)
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

void Orianna::LaneClear()
{
	if (Laneclear->Enabled() && Extensions::Validate (StationaryBall))
	{
		if (Extensions::CountMinionsInTargetRange (StationaryBall->GetPosition(), W->Radius() + 25) > 2 && laneClearW->Enabled() && W->IsReady() && Hero->ManaPercent() > laneClearWMana->GetFloat() && (!isBallMoving()))
		{
			W->CastOnPlayer();
		}
		if (Q->IsReady() && laneClearQ->Enabled() && Hero->ManaPercent() > laneClearQMana->GetFloat())
		{
			for (auto minion : GEntityList->GetAllMinions (false, true, true))
			{
				if (minion != nullptr && !minion->IsWard() && minion->IsCreep() && Extensions::GetDistance (Hero, minion->GetPosition()) <= 1000)
				{
					if (!minion->IsDead())
					{
						/*
						auto hp = GHealthPrediction->GetPredictedHealth (minion, kLastHitPrediction, 350, 350);
						if (hp < qDmg (minion))
						{
						Q->CastOnPosition (minion->GetPosition());
						return;
						}*/
						FarmQ (minion->GetPosition());
						return;
					}
				}
			}
		}
	}
}


void Orianna::Automatic()
{
	if (Hero->HasBuff ("OrianaGhostSelf"))
	{
		BallPosition = Hero->GetPosition();
		StationaryBall = Hero;
	}
	std::vector<IUnit*> result;
	for (auto target : GEntityList->GetAllHeros (true, false))
	{
		if (target->HasBuff ("orianaghost"))
		{
			BallPosition = target->GetPosition();
			StationaryBall = target;
			result.push_back (target);
		}
	}
	if (isBallMoving())
	{
		return;
	}
	if (autoW->Enabled() && W->IsReady() && harassW->Enabled() && Hero->ManaPercent() > harassWMana->GetFloat() && ( (Extensions::IsValid (GetMovingBallPosW()) && Extensions::EnemiesInRange (GetMovingBallPosW(), W->Radius() - 45)) || (Extensions::Validate (StationaryBall) && Extensions::EnemiesInRange (StationaryBall->GetPosition(), W->Radius()))))
	{
		W->CastOnPlayer();
	}
	if (GetAsyncKeyState (FlashUlt->GetInteger()) && !GGame->IsChatOpen() && GUtility->IsLeagueWindowFocused())
	{
		PerformFlashUlt();
	}
}

float Orianna::qDmg (IUnit* Target)
{
	float InitDamage = 0;
	float BonusStackDamage = ( (0.50 * Hero->TotalMagicDamage()));
	if (Hero->GetSpellLevel (kSlotQ) == 1)
	{
		InitDamage += (60 + BonusStackDamage);
	}
	else if (Hero->GetSpellLevel (kSlotQ) == 2)
	{
		InitDamage += (90 + BonusStackDamage);
	}
	else if (Hero->GetSpellLevel (kSlotQ) == 3)
	{
		InitDamage += (120 + BonusStackDamage);
	}
	else if (Hero->GetSpellLevel (kSlotQ) == 4)
	{
		InitDamage += (150 + BonusStackDamage);
	}
	else if (Hero->GetSpellLevel (kSlotQ) == 5)
	{
		InitDamage += (180 + BonusStackDamage);
	}
	//「 MAGIC DAMAGE: 60 / 90 / 120 / 150 / 180 (+ 50% AP) 」
	return GDamage->CalcMagicDamage (Hero, Target, InitDamage);
}

float Orianna::wDmg (IUnit* Target)
{
	float InitDamage = 0;
	float BonusStackDamage = (0.7 * Hero->TotalMagicDamage());
	if (Hero->GetSpellLevel (kSlotW) == 1)
	{
		InitDamage += 70 + BonusStackDamage;
	}
	else if (Hero->GetSpellLevel (kSlotW) == 2)
	{
		InitDamage += 115 + BonusStackDamage;
	}
	else if (Hero->GetSpellLevel (kSlotW) == 3)
	{
		InitDamage += 160 + BonusStackDamage;
	}
	else if (Hero->GetSpellLevel (kSlotW) == 4)
	{
		InitDamage += 205 + BonusStackDamage;
	}
	else if (Hero->GetSpellLevel (kSlotW) == 5)
	{
		InitDamage += 250 + BonusStackDamage;
	}
	// MAGIC DAMAGE: 70 / 115 / 160 / 205 / 250 (+ 70% AP)
	return GDamage->CalcMagicDamage (Hero, Target, InitDamage);
}

float Orianna::rDmg (IUnit* Target)
{
	float InitDamage = 0;
	float BonusStackDamage = (0.7 * Hero->TotalMagicDamage());
	if (Hero->GetSpellLevel (kSlotR) == 1)
	{
		InitDamage += 150 + BonusStackDamage;
	}
	else if (Hero->GetSpellLevel (kSlotR) == 2)
	{
		InitDamage += 225 + BonusStackDamage;
	}
	else if (Hero->GetSpellLevel (kSlotR) == 3)
	{
		InitDamage += 300 + BonusStackDamage;
	}
	// MAGIC DAMAGE: 150 / 225 / 300 (+ 70% AP)
	return GDamage->CalcMagicDamage (Hero, Target, InitDamage);
}

float Orianna::rD1v1 (IUnit* Target)
{
	float InitDamage = 0;
	float BonusStackDamage = (0.7 * Hero->TotalMagicDamage());
	if (Hero->GetSpellLevel (kSlotR) == 1)
	{
		InitDamage += 150 + BonusStackDamage;
	}
	else if (Hero->GetSpellLevel (kSlotR) == 2)
	{
		InitDamage += 225 + BonusStackDamage;
	}
	else if (Hero->GetSpellLevel (kSlotR) == 3)
	{
		InitDamage += 300 + BonusStackDamage;
	}
	// MAGIC DAMAGE: 150 / 225 / 300 (+ 70% AP)
	auto total = GDamage->CalcPhysicalDamage (Hero, Target, GDamage->GetAutoAttackDamage (Hero, Hero, true) * extraAutos->GetInteger()) + GDamage->CalcMagicDamage (Hero, Target, InitDamage);
	return total;
}

void Orianna::KillSteal() {}



//Author = Divine
void Orianna::DrawGagongReplicate (Vec3 BallPos)
{
	Vec3 perp1 = Extensions::Cross (BallPos, UpVec).VectorNormalize() * BallRad;
	Vec3 perp2 = -perp1;
	Vec3 BallPosNorm = Vec3 (BallPos).VectorNormalize();
	Vec3 forwardExtend = (BallPosNorm*BallRad);
	Vec2 UpperLeft (-1, -1), BottomRight (-1, -1), BottomLeft (-1, -1), UpperRight (-1, -1);
	Vec2 Left (-1, -1), Right (-1, -1), Bottom (-1, -1), Up (-1, -1);
	Vec3 v3Left = Extensions::Cross (perp1, UpVec).VectorNormalize() * BallRad;
	Vec3 v3Right = Extensions::Cross (perp2, UpVec).VectorNormalize() * BallRad;
	Vec3 BallPosNoY (Vec3 (BallPos.x, 0, BallPos.z));
	//Vec3 BallPosNoYZ(Vec3(BallPos.x, 0, 0));
	if (GGame->Time() - lastTimeStamp > 0.05)
	{
		lastTimeStamp = GGame->Time();
		if (gagongAngle > 360)
		{
			gagongAngle = 0;
		}
		gagongAngle += TO_RAD (5);
	}
	if (GGame->Projection (BallPosNoY + Extensions::RotateZ (perp1 + BallPos, BallPos, gagongAngle), &UpperLeft) && GGame->Projection (BallPosNoY + Extensions::RotateZ (perp2 + BallPos, BallPos, gagongAngle),
	        &BottomRight) && GGame->Projection (BallPosNoY + Extensions::RotateZ (BallPos + forwardExtend, BallPos, gagongAngle), &UpperRight) && GGame->Projection (BallPosNoY + Extensions::RotateZ (BallPos - forwardExtend, BallPos, gagongAngle), &BottomLeft))
	{
		float width = 0.1;
		for (float i = 0; i < 5; i += 0.3)
		{
			UpperLeft -= Vec2 (i * width, i * width);
			UpperRight += Vec2 (i * width, - (i * width));
			BottomLeft += Vec2 (-i * width, i * width);
			BottomRight += Vec2 (i * width, i * width);
			GRender->DrawLine (UpperLeft, UpperRight, Color1);
			GRender->DrawLine (UpperLeft, BottomLeft, Color2);
			GRender->DrawLine (UpperRight, BottomRight, Color3);
			GRender->DrawLine (BottomLeft, BottomRight, Color4);
		}
	}
	if (GGame->Projection (BallPosNoY + Extensions::RotateZ (perp1*0.73 + v3Left*0.8 + BallPos, BallPos, -gagongAngle), &Left) && GGame->Projection (BallPosNoY + Extensions::RotateZ (perp1*0.73 - v3Left*0.8 + BallPos, BallPos, -gagongAngle),
	        &Up) && GGame->Projection (BallPosNoY + Extensions::RotateZ (perp2*0.73 - v3Right*0.8 + BallPos, BallPos, -gagongAngle), &Bottom) && GGame->Projection (BallPosNoY + Extensions::RotateZ (perp2*0.73 + v3Right*0.8 + BallPos, BallPos, -gagongAngle), &Right))
	{
		float width = 0.1;
		for (float i = 0; i < 6; i += 0.3)
		{
			Left.x -= i * width;
			Right.x += i * width;
			Up.y -= i*width;
			Bottom.y += i*width;
			GRender->DrawLine (Left, Up, Color5);
			GRender->DrawLine (Left, Bottom, Color6);
			GRender->DrawLine (Right, Up, Color7);
			GRender->DrawLine (Right, Bottom, Color8);
		}
	}
	//GRender->DrawOutlinedCircle(BallPos, GagongColors[0], 83);
	int spiral_num_segments = 50;
	int spiral_rad = 50;
	Vec3 spiral_prev_vec (-1, -1, -1);
	Vec2 currv2D (-1, -1), prev2D (-1, -1);
	/*for (int i = 0; i < spiral_num_segments; i++)
	{
	float theta = 10.0f * 3.1415926f * i / spiral_num_segments; //the current angle
	float x = (spiral_rad / spiral_num_segments) *i * cosf (theta); //the x component
	float z = (spiral_rad / spiral_num_segments) *i * sinf (theta); //the y component
	Vec3 spiral_curr_vec (x + BallPos.x, GNavMesh->GetHeightForPoint (Vec2 (BallPos.x, BallPos.z)), z + BallPos.z);
	if (spiral_prev_vec.x == -1) { spiral_prev_vec = BallPos; }
	if (GGame->Projection (spiral_curr_vec, &currv2D) && GGame->Projection (spiral_prev_vec, &prev2D))
	//GRender->DrawLine(currv2D, prev2D, i? GagongColors[1]:GagongColors[0]);
	if (BallPos != Hero->GetPosition())
	{
	if (!isBallMoving())
	{
	GRender->DrawTextW (currv2D, GagongColors[0], "*");
	}
	else
	{
	GRender->DrawTextW (currv2D, GagongColors[1], "*");
	}
	spiral_prev_vec = spiral_curr_vec;*/
}



void Orianna::dmgdraw()
{
	if (drawDmg->Enabled())
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
					WDamage = wDmg (hero);
				}
				if (Q->IsReady())
				{
					QDamage = qDmg (hero);
				}
				if (R->IsReady())
				{
					RDamage = rDmg (hero) + GDamage->GetAutoAttackDamage (Hero, hero, true);
				}
				Vec4 BarColor;
				HPBarColor->GetColor (&BarColor);
				float totalDamage = QDamage + WDamage + EDamage + RDamage;
				Rembrandt::DrawDamageOnChampionHPBar (hero, totalDamage, BarColor);
			}
		}
	}
}
//Author Divine Nader[Sl]
void Orianna::DrawGagonDix (Vec3 BallPos, Vec4 Color)
{
	Vec3 perp1 = Extensions::Cross (BallPos, UpVec).VectorNormalize() * BallRad;
	Vec3 perp2 = -perp1;
	Vec3 BallPosNorm = Vec3 (BallPos).VectorNormalize();
	Vec3 forwardExtend = (BallPosNorm*BallRad);
	Vec2 UpperLeft (-1, -1), BottomRight (-1, -1), BottomLeft (-1, -1), UpperRight (-1, -1);
	Vec2 Left (-1, -1), Right (-1, -1), Bottom (-1, -1), Up (-1, -1);
	Vec3 v3Left = Extensions::Cross (perp1, UpVec).VectorNormalize() * BallRad;
	Vec3 v3Right = Extensions::Cross (perp2, UpVec).VectorNormalize() * BallRad;
	if (GGame->Projection (perp1 + BallPos, &UpperLeft) && GGame->Projection (perp2 + BallPos, &BottomRight) && GGame->Projection (BallPos + forwardExtend, &UpperRight) && GGame->Projection (BallPos - forwardExtend, &BottomLeft))
	{
		float width = 0.1;
		for (float i = 0; i < 5; i += 0.3)
		{
			UpperLeft -= Vec2 (i * width, i * width);
			UpperRight += Vec2 (i * width, - (i * width));
			BottomLeft += Vec2 (-i * width, i * width);
			BottomRight += Vec2 (i * width, i * width);
			GRender->DrawLine (UpperLeft, UpperRight, Color);
			GRender->DrawLine (UpperLeft, BottomLeft, Color);
			GRender->DrawLine (UpperRight, BottomRight, Color);
			GRender->DrawLine (BottomLeft, BottomRight, Color);
		}
	}
	if (GGame->Projection (BallPos + perp1*0.73 + v3Left*0.8, &Left) && GGame->Projection (BallPos + perp1*0.73 - v3Left*0.8, &Up) && GGame->Projection (BallPos + perp2*0.73 - v3Right*0.8, &Bottom) && GGame->Projection (BallPos + perp2*0.73 + v3Right*0.8, &Right))
	{
		float width = 0.1;
		for (float i = 0; i < 4; i += 0.3)
		{
			Left.x -= i * width;
			Right.x += i * width;
			Up.y -= i*width;
			Bottom.y += i*width;
			GRender->DrawLine (Left, Up, Color);
			GRender->DrawLine (Left, Bottom, Color);
			GRender->DrawLine (Right, Up, Color);
			GRender->DrawLine (Right, Bottom, Color);
		}
	}
	GRender->DrawOutlinedCircle (BallPos, Color, 83);
}

void Orianna::Drawing()
{
	auto player = Hero;
	if (drawBall->Enabled() && ballSelect->GetInteger() == 0)
	{
		if (StationaryBall && !isBallMoving() && (StationaryBall->IsValidObject() || StationaryBall->IsValidTarget()))
		{
			Vec3 Pos = StationaryBall->GetPosition();
			DrawGagongReplicate (Pos);
		}
		if (isBallMoving())
		{
			DrawGagongReplicate (GetMovingBallPos());
		}
	}
	else if (drawBall->Enabled() && ballSelect->GetInteger() == 1)
	{
		if (StationaryBall && (StationaryBall->IsValidObject() || StationaryBall->IsValidTarget()))
		{
			Vec3 Pos = StationaryBall->GetPosition();
			DrawGagonDix (Pos, BallIndicatorColor);
		}
		if (isBallMoving())
		{
			DrawGagonDix (GetMovingBallPos(), BallIndicatorMovingColor);
		}
	}
	if (DrawReady->Enabled())
	{
		if (Q->IsReady() && DrawQ->Enabled())
		{
			GRender->DrawCircle (player->GetPosition(), Q->Range(), Vec4 (0, 225, 0, 225));
		}
		if (W->IsReady() && DrawW->Enabled())
		{
			GRender->DrawCircle (player->GetPosition(), W->Range(), Vec4 (0, 225, 0, 225));
		}
		if (E->IsReady() && DrawE->Enabled())
		{
			GRender->DrawCircle (player->GetPosition(), E->Range(), Vec4 (0, 225, 0, 225));
		}
	}
	else
	{
		if (DrawQ->Enabled())
		{
			GRender->DrawCircle (player->GetPosition(), Q->Range(), Vec4 (0, 225, 0, 225));
		}
		if (DrawW->Enabled())
		{
			GRender->DrawCircle (player->GetPosition(), W->Range(), Vec4 (0, 225, 0, 225));
		}
		if (DrawE->Enabled())
		{
			GRender->DrawCircle (player->GetPosition(), E->Range(), Vec4 (0, 225, 0, 225));
		}
	}
}