#include "Orianna.h"
#include <time.h>
#include "Lords.h"
#define M_PI 3.14159265358979323846
bool FlashUlting = false;
IUnit *StationaryBall, *MovingBall;
Vec3 BallPosition;
int Ticks;
auto VecG = std::vector<Vec3>();
float BallRad = 75;
Vec4 BallIndicatorColor(52, 152, 219, 255);
Vec4 BallIndicatorMovingColor(255, 0, 255, 255);
Vec4 GagongColors[9] =
{ Vec4(255,255,26,255),Vec4(242,56,90,255),Vec4(245,165,3,255),
Vec4(167,197,32,255),Vec4(73,63,11	,255),Vec4(247,233,103,255)
,Vec4(169,207,84,255),Vec4(255,133,52,255),Vec4(201,215,135,255)
};

Vec4 GagongColorsMove[9] =
{ Vec4(65,115,120,255),Vec4(255,53,139,255),Vec4(1,176,240,255),
Vec4(174,238,0,255),Vec4(217,4,43,255),Vec4(44,29,255,255)
,Vec4(183,0,0,255),Vec4(255,202,61,255),Vec4(0,146,178,255)
};

Vec3 UpVec(0, 1, 0);

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

Orianna::Orianna(IMenu* Parent, IUnit* Hero) :Champion(Parent, Hero)
{
	srand(time(NULL));
	Q = GPluginSDK->CreateSpell2(kSlotQ, kCircleCast, true, true, kCollidesWithYasuoWall);
	Q->SetSkillshot(0.f, 150.f, 1400.f, 810.f);
	W = GPluginSDK->CreateSpell2(kSlotW, kCircleCast, false, true, kCollidesWithNothing);
	W->SetSkillshot(0.f, 220.f, FLT_MAX, 245.f);
	E = GPluginSDK->CreateSpell2(kSlotE, kTargetCast, false, true, kCollidesWithYasuoWall);
	E->SetSkillshot(0.25f, 80.f, 1700.f, 1095.f);
	R = GPluginSDK->CreateSpell2(kSlotR, kCircleCast, false, true, kCollidesWithNothing);
	R->SetSkillshot(0.6f, 365.f, FLT_MAX, 800.f);

	RFlash = GPluginSDK->CreateSpell2(kSlotR, kLineCast, false, false, static_cast<eCollisionFlags>(kCollidesWithNothing));
	RFlash->SetSkillshot(0.25f, 60, FLT_MAX, 775);


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


	OriannaMenu = Parent->AddMenu("Orianna Menu");


	qMenu = Parent->AddMenu("Q Settings");
	wMenu = Parent->AddMenu("W Settings");
	eMenu = Parent->AddMenu("E Settings");
	rMenu = Parent->AddMenu("R Settongs");
	LaneClearMenu = Parent->AddMenu("Lane Clear");
	MiscMenu = Parent->AddMenu("Miscs");
	Drawings = Parent->AddMenu("All Drawings");


	killStealQ = qMenu->CheckBox("Kill Steal with Q", true);
	ComboQ = qMenu->CheckBox("Use Q in Combo", true);
	autoQ = qMenu->CheckBox("Automatic Harass Q", false);
	harassQ = qMenu->CheckBox("Harass with Q", true);
	harassQMana = qMenu->AddFloat("^-> Only Harras Q if Mana >", 0, 100, 50);
	gapcloseQ = qMenu->CheckBox("Use Q on Gap Closers", true);



	killStealW = wMenu->CheckBox("Kill Steal with W", true);
	ComboW = wMenu->CheckBox("Use W in Combo", true);
	autoW = wMenu->CheckBox("Automatic Harass W", false);
	harassW = wMenu->CheckBox("Harass with W", true);
	harassWMana = wMenu->AddFloat("^-> Only Harras W if Mana >", 0, 100, 50);



	ComboE = eMenu->CheckBox("Use E in Combo", true);
	killStealE = eMenu->CheckBox("Kill Steal with E", true);
	HealthPercent = eMenu->AddFloat("Shield  Self if Health Percent Below: ", 0, 100, 30);
	ShieldTeamate = eMenu->CheckBox("Shield Teammates", true);
	ShieldTeamatePercent = eMenu->AddFloat("^->Shield Teammate if Health Percent Below: ", 0, 100, 30);

	ComboR = rMenu->CheckBox("Use Ult in Combo", true);
	ultMin = rMenu->AddFloat("Only Ult if it will hit atleast: ", 0, 5, 2);
	BlockR = rMenu->CheckBox("Block R on no hits", true);
	FlashUlt = rMenu->AddKey("Flash Ult key", 84);
	InterruptR = rMenu->CheckBox("Use Ult to Interrupt Spells", true);
	KillStealR = rMenu->CheckBox("Use Ult to Kill Steal", true);

	Laneclear = LaneClearMenu->CheckBox("Use Spells in Lane Clear", true);
	laneClearQ = LaneClearMenu->CheckBox("Wave Clear with Q", true);
	laneClearQMana = LaneClearMenu->AddFloat("^-> Only Wave Clear Q if Mana >", 0, 100, 50);
	laneClearW = LaneClearMenu->CheckBox("Wave Clear with W", true);
	laneClearWMana = LaneClearMenu->AddFloat("^-> Only Wave Clear W if Mana >", 0, 100, 50);


	DrawReady = Drawings->CheckBox("Draw Ready Spells", true);
	drawDmg = Drawings->CheckBox("Draw Damage", true);
	DrawQ = Drawings->CheckBox("Draw Q", true);
	DrawW = Drawings->CheckBox("Draw W", true);
	DrawE = Drawings->CheckBox("Draw E", true);
	DrawR = Drawings->CheckBox("Draw R", true);
	drawBall = Drawings->CheckBox("Draw Ball Animation", true);
	ballSelect = Drawings->AddFloat("Ball Style 0 = Divine [Nader Sl] 1 = Gagondix", 0, 1, 0);


}

Vec3 Orianna::getPosToRflash(Vec3 target)
{

	return  GEntityList->Player()->ServerPosition().Extend(GGame->CursorPosition(), Flash->Range());
}

void Orianna::CastFlash() {
	auto target = GTargetSelector->GetFocusedTarget() != nullptr
		? GTargetSelector->GetFocusedTarget()
		: GTargetSelector->FindTarget(QuickestKill, SpellDamage, RFlash->Range());
	Flash->CastOnPosition(getPosToRflash(target->GetPosition()));
}

void Orianna::PerformFlashUlt()
{
	GGame->IssueOrder(GEntityList->Player(), kMoveTo, GGame->CursorPosition());
	auto target1 = GTargetSelector->GetFocusedTarget();
	if (target1 == nullptr || !target1->IsHero() || target1->IsDead())
		return;
	if (R->IsReady() && Flash->IsReady() && StationaryBall == GEntityList->Player())
	{
		auto target = GTargetSelector->GetFocusedTarget() != nullptr
			? GTargetSelector->GetFocusedTarget()
			: GTargetSelector->FindTarget(QuickestKill, SpellDamage, RFlash->Range());


		auto flashPosition = GEntityList->Player()->ServerPosition().Extend(GGame->CursorPosition(), Flash->Range());
		AdvPredictionOutput result;
		RFlash->RunPrediction(target, false, kCollidesWithNothing, &result);

		if (target != nullptr && target->IsValidTarget() && !target->IsDead() && !target->IsInvulnerable() && result.HitChance >= kHitChanceVeryHigh)
		{
			RFlash->CastOnTarget(target, kHitChanceVeryHigh);
			GPluginSDK->DelayFunctionCall(500 + (GGame->Latency()) / 2, [=]() { CastFlash(); FlashUlting = false; });

		}
	}
}

void Orianna::OnNewPath(IUnit* Source, const std::vector<Vec3>& path_)
{
	OnRunPath(Source, path_);
}

void Orianna::OnGameUpdate()
{
	//KillSteal();
	Automatic();

	//debug key

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
}



void Orianna::OnRender()
{
	Drawing();
	dmgdraw();
	if (Laneclear->Enabled()) {
		Vec2 pos;
		if (GGame->Projection(GEntityList->Player()->GetPosition(), &pos));
		GRender->DrawTextW(Vec2(pos.x + 72, pos.y + 10), Vec4(0, 255, 0, 255), "LANE CLEAR ON");
		/*static auto message = GRender->CreateFontW("Impact", 30.f, kFontWeightNormal);
		message->SetColor(Vec4(0, 255, 0, 255));
		message->SetOutline(false);
		message->Render(pos.x + 60, pos.y + 23, "LANE CLEAR ON");*/
	}
}

bool Orianna::OnPreCast(int Slot, IUnit* Target, Vec3* StartPosition, Vec3* EndPosition)
{
	if (Slot == kSlotR && BlockR->Enabled() && FlashUlting == false)
	{
		if (SpellCheck(StationaryBall->GetPosition(), R->Radius(), 0.2) == 0) {
			return false;
		}
	}

	if (Slot == kSlotW && R->IsReady() && ComboR->Enabled() && SpellCheck(StationaryBall->GetPosition(), R->Radius(), R->GetDelay()) >= ultMin->GetFloat()) {
		R->CastOnPlayer();
		return false;
	}


	if (Slot == kSlotE && W->IsReady() && !Q->IsReady() && ComboW->Enabled()) {
		if (SpellCheck(StationaryBall->GetPosition(), W->Radius(), W->GetDelay()) > 0)
			W->CastOnPlayer();
		return false;
	}


	return true;

}

void Orianna::OnInterrupt(InterruptibleSpell const& Args)
{
	auto player = GEntityList->Player();
	if (Args.Source == nullptr || Args.Source->IsDead()) return;
	if (Q->IsReady() && player->IsValidTarget(Args.Source, Q->Range()) && Args.Source != nullptr && Args.Source != GEntityList->Player() && Args.Source->IsEnemy(GEntityList->Player()))
	{
		Q->CastOnPosition(Args.Source->ServerPosition());
	}

	if (R->IsReady() && InterruptR->Enabled() && SpellCheck(StationaryBall->GetPosition(), R->Radius(), R->GetDelay()) > 0)
	{
		R->CastOnPlayer();
	}
}

void Orianna::AntiGapclose(GapCloserSpell const& args)
{
	if (Q->IsReady() && args.Source->IsEnemy(GEntityList->Player()) && gapcloseQ->Enabled() && args.Source->IsValidTarget())
	{
		if (Extensions::GetDistance(GEntityList->Player(), args.EndPosition) <= Q->Range())
		{
			auto delay = (GBuffData->GetEndTime(args.Data) - GGame->Time() - Q->GetDelay());
			if (delay > 0)
				GPluginSDK->DelayFunctionCall(delay * 1000, [=]() { Q->CastOnPosition(args.EndPosition); });
			else
				Q->CastOnPosition(args.EndPosition);
		}
	}
}

int Orianna::GetEHits()
{
	auto player = GEntityList->Player();
	auto hits = std::vector <IUnit*>();
	for (auto enemy : GEntityList->GetAllHeros(false, true))
	{
		if (player->IsValidTarget(enemy, 2000)) {
			Vec3 position;
			GPrediction->FindBestCastPositionEx(player->GetPosition(), 0, E->Range(), E->Radius(), true, false, true, position, hits);
			if (Extensions::GetDistance(position, StationaryBall->GetPosition()) < 100) {
				return hits.size();
			}
		}
	} return 0;
}


//t[0,1]


//Author= Divine[NaderSl]
BOOL Orianna::isBallMoving()
{
	return 	GGame->Time() - QBallData.StartTime < QBallData.OverAllTime;
}

//Author= Divine[NaderSl]
Vec3 Orianna::GetMovingBallPos()
{
	float EndTime = QBallData.StartTime + QBallData.OverAllTime;
	Vec3 CurrPos = Extensions::lerp((EndTime - GGame->Time()) / QBallData.OverAllTime, QBallData.EndPos, QBallData.StartPos);
	return Vec3(CurrPos.x, GNavMesh->GetHeightForPoint(Vec2(CurrPos.x, CurrPos.z)) + 110, CurrPos.z);
}

void Orianna::OnSpellCast(CastedSpell const& args) {


	if (!args.Caster_->IsEnemy(GEntityList->Player()))
	{
		if (std::string(args.Name_) == "OrianaIzunaCommand")
		{
			//Author= Divine[NaderSl]
			if (&args.Position_ && &args.EndPosition_ && args.Speed_)
			{
				// v = d/t => t = d/v
				Vec3 StartPos = (StationaryBall ? StationaryBall->GetPosition() : args.Position_);
				float dist = (args.EndPosition_ - StartPos).Length();
				QBallData = { StartPos,args.EndPosition_,(dist / args.Speed_), GGame->Time() };
			}
		}

		if (std::string(args.Name_) == "OrianaRedactCommand")
		{
			//Author= Divine[NaderSl]
			if (&args.Position_ && &args.EndPosition_ && args.Speed_)
			{
				// v = d/t => t = d/v


				Vec3 StartPos = (StationaryBall ? StationaryBall->GetPosition() : args.Position_);
				float dist = (args.EndPosition_ - StartPos).Length();
				QBallData = { StartPos,args.EndPosition_,(dist / 1900), GGame->Time() };
			}
		}

		if (Extensions::GetDistance(GEntityList->Player(), args.Caster_->GetPosition()) <= E->Range() && args.Caster_->IsValidTarget() && E->IsReady()) {

			if (std::string(args.Name_) == "ShenE") {
				E->CastOnTarget(args.Caster_);

			}
			if (std::string(args.Name_) == "JaxLeapStrike") {
				E->CastOnTarget(args.Caster_);

			}
			if (std::string(args.Name_) == "AatroxQ") {
				E->CastOnTarget(args.Caster_);

			}
			if (std::string(args.Name_) == "AkaliShadowDance") {
				E->CastOnTarget(args.Caster_);

			}
			if (std::string(args.Name_) == "HeadButt") {
				E->CastOnTarget(args.Caster_);

			}
			if (std::string(args.Name_) == "BandageToss") {
				E->CastOnTarget(args.Caster_);

			}
			if (std::string(args.Name_) == "DianaTeleport") {
				E->CastOnTarget(args.Caster_);

			}
			if (std::string(args.Name_) == "EkkoE") {
				E->CastOnTarget(args.Caster_);

			}
			if (std::string(args.Name_) == "EliseSpidereInitial") {
				E->CastOnTarget(args.Caster_);

			}
			if (std::string(args.Name_) == "CamilleE") {
				E->CastOnTarget(args.Caster_);

			}
			if (std::string(args.Name_) == "KledR") {
				E->CastOnTarget(args.Caster_);

			}
			if (std::string(args.Name_) == "KledE") {
				E->CastOnTarget(args.Caster_);

			}
			if (std::string(args.Name_) == "IvernQ") {
				E->CastOnTarget(args.Caster_);

			}
			if (std::string(args.Name_) == "IllaoiW") {
				E->CastOnTarget(args.Caster_);

			}
			if (std::string(args.Name_) == "Crowstorm") {
				E->CastOnTarget(args.Caster_);

			}
			if (std::string(args.Name_) == "FioraQ") {
				E->CastOnTarget(args.Caster_);

			}
			if (std::string(args.Name_) == "GnarE") {
				E->CastOnTarget(args.Caster_);

			}
			if (std::string(args.Name_) == "GnarBigE") {
				E->CastOnTarget(args.Caster_);

			}
			if (std::string(args.Name_) == "GragasE") {
				E->CastOnTarget(args.Caster_);

			}
			if (std::string(args.Name_) == "HecarimUlt") {
				E->CastOnTarget(args.Caster_);

			}
			if (std::string(args.Name_) == "IreliaGatotsu") {
				E->CastOnTarget(args.Caster_);

			}
			if (std::string(args.Name_) == "JarvanIVCataclysm") {
				E->CastOnTarget(args.Caster_);

			}
			if (std::string(args.Name_) == "JarvanIVDragonStrike") {
				E->CastOnTarget(args.Caster_);

			}
			if (std::string(args.Name_) == "KatarinaE") {
				E->CastOnTarget(args.Caster_);

			}
			if (std::string(args.Name_) == "KennenLightningRush") {
				E->CastOnTarget(args.Caster_);

			}
			if (std::string(args.Name_) == "KhazixE") {
				E->CastOnTarget(args.Caster_);

			}
			if (std::string(args.Name_) == "LeblancSlide") {
				E->CastOnTarget(args.Caster_);

			}
			if (std::string(args.Name_) == "LeblancSlideM") {
				E->CastOnTarget(args.Caster_);

			}
			if (std::string(args.Name_) == "LeonaZenithBlade") {
				E->CastOnTarget(args.Caster_);

			}
			if (std::string(args.Name_) == "MaokaiUnstableGrowth") {
				E->CastOnTarget(args.Caster_);

			}
			if (std::string(args.Name_) == "NocturneParanoia") {
				E->CastOnTarget(args.Caster_);

			}
			if (std::string(args.Name_) == "OlafRagnarok") {
				E->CastOnTarget(args.Caster_);

			}
			if (std::string(args.Name_) == "SionR") {
				E->CastOnTarget(args.Caster_);

			}
			if (std::string(args.Name_) == "RengarR") {
				E->CastOnTarget(args.Caster_);

			}
			if (std::string(args.Name_) == "ShyvanaTransformCast") {
				E->CastOnTarget(args.Caster_);

			}
			if (std::string(args.Name_) == "ShyvanaTransformLeap") {
				E->CastOnTarget(args.Caster_);

			}
			if (std::string(args.Name_) == "ThreshQLeap") {
				E->CastOnTarget(args.Caster_);

			}
			if (std::string(args.Name_) == "WarwickR") {
				E->CastOnTarget(args.Caster_);

			}
			if (std::string(args.Name_) == "GallioE") {
				E->CastOnTarget(args.Caster_);

			}
			if (std::string(args.Name_) == "ZacE") {
				E->CastOnTarget(args.Caster_);

			}
			if (std::string(args.Name_) == "LucianE") {
				E->CastOnTarget(args.Caster_);

			}
			if (std::string(args.Name_) == "MonkeyKingNimbus") {
				E->CastOnTarget(args.Caster_);

			}
			if (std::string(args.Name_) == "NautilusAnchorDrag") {
				E->CastOnTarget(args.Caster_);

			}
			if (std::string(args.Name_) == "Pantheon_LeapBash") {
				E->CastOnTarget(args.Caster_);

			}
			if (std::string(args.Name_) == "PoppyHeroicCharge") {
				E->CastOnTarget(args.Caster_);

			}
			if (std::string(args.Name_) == "QuinnE") {
				E->CastOnTarget(args.Caster_);

			}
			if (std::string(args.Name_) == "RenektonSliceAndDice") {
				E->CastOnTarget(args.Caster_);

			}
			if (std::string(args.Name_) == "RiftWalk") {
				E->CastOnTarget(args.Caster_);

			}
			if (std::string(args.Name_) == "RivenTriCleave") {
				E->CastOnTarget(args.Caster_);

			}
			if (std::string(args.Name_) == "RocketJump") {
				E->CastOnTarget(args.Caster_);

			}
			if (std::string(args.Name_) == "SejuaniArcticAssault") {
				E->CastOnTarget(args.Caster_);

			}
			if (std::string(args.Name_) == "TalonCutThroat") {
				E->CastOnTarget(args.Caster_);

			}
			if (std::string(args.Name_) == "UFSlash") {
				E->CastOnTarget(args.Caster_);

			}
			if (std::string(args.Name_) == "UdyrBearStance") {
				E->CastOnTarget(args.Caster_);

			}
			if (std::string(args.Name_) == "KatarinaE") {
				E->CastOnTarget(args.Caster_);

			}
			if (std::string(args.Name_) == "Valkyrie") {
				E->CastOnTarget(args.Caster_);

			}
			if (std::string(args.Name_) == "ViQ") {
				E->CastOnTarget(args.Caster_);

			}
			if (std::string(args.Name_) == "ViR") {
				E->CastOnTarget(args.Caster_);

			}
			if (std::string(args.Name_) == "VolibearQ") {
				E->CastOnTarget(args.Caster_);

			}
			if (std::string(args.Name_) == "XenZhaoSweep") {
				E->CastOnTarget(args.Caster_);

			}
			if (std::string(args.Name_) == "YasuoDashWrapper") {
				E->CastOnTarget(args.Caster_);

			}
			if (std::string(args.Name_) == "blindmonkqtwo") {
				E->CastOnTarget(args.Caster_);

			}
			if (std::string(args.Name_) == "khazixelong") {
				E->CastOnTarget(args.Caster_);

			}
			if (std::string(args.Name_) == "reksaieburrowed") {
				E->CastOnTarget(args.Caster_);

			}
			if (std::string(args.Name_) == "TryndamereE") {
				E->CastOnTarget(args.Caster_);

			}
		}
	}
}


void Orianna::OnCreate(IUnit* object) {
	auto objectName = object->GetObjectName();
	if (!objectName)return;
	//auto player = GEntityList->Player();
	//	GUtility->CreateDebugConsole();
	//	GUtility->LogConsole(objectName);

	if (object->IsValidObject() && object->IsVisible())
	{
		if ((!strcmp(objectName, "Orianna_Base_Z_ball_glow_green.troy")))
		{
			StationaryBall = object;
		}
		else  if ((!strcmp(objectName, "Orianna_Base_Q_yomu_ring_green.troy")))
		{
			MovingBall = object;
		}
	}

	std::vector<IUnit*> result1;
	for (auto ball : GEntityList->GetAllUnits())
	{
		if (strcmp(objectName, "Orianna_Base_Z_ball_glow_green.troy") == 0 && object->IsVisible() && object->IsValidObject())
		{
			BallPosition = ball->GetPosition();
			result1.push_back(ball);

		}
	}


}

bool Orianna::pairCompare(const std::pair<int, Vec2>& firstElem, const std::pair<int, Vec2>& secondElem) {
	return firstElem.first < secondElem.first;

}


void Orianna::TeamFightQ(Vec3 pos)
{
	auto posChecked = 0;
	auto maxPosChecked = 85;
	auto posRadius = 65;
	auto radiusIndex = 0;
	std::vector<std::pair<int, Vec2>> CloseQPositions;
	std::vector<std::pair<int, Vec2>> possibleQPositions;

	while (posChecked < maxPosChecked) {
		radiusIndex++;
		auto curRadius = radiusIndex * (0x2 * posRadius);
		auto curCurcleChecks = static_cast<int>(ceil((0x2 * M_PI * curRadius) / (0x2 * static_cast<double>(posRadius))));

		for (auto i = 1; i < curCurcleChecks; i++) {
			posChecked++;

			auto cRadians = (0x2 * M_PI / (curCurcleChecks - 1)) * i;
			auto xPos = static_cast<float>(floor(pos.x + curRadius * cos(cRadians)));
			auto zPos = static_cast<float>(floor(pos.z + curRadius * sin(cRadians)));
			auto posFor2D = Vec2(xPos, zPos);
			auto count = Extensions::EnemiesInRange(Extensions::To3D(posFor2D), R->Radius());

			//GGame->PrintChat(std::to_string(count).c_str());

			if (GNavMesh->IsPointWall(Extensions::To3D(posFor2D))) {

				// dont push is wall
				continue;
			}

			if (Extensions::Dist2D(posFor2D, GEntityList->Player()->ServerPosition()) > Q->Range())
			{
				// dont push to far away to cast;
				continue;
			}

			if (Extensions::Dist2D(posFor2D, pos) <= R->Radius() - 15)
			{
				//	GGame->ShowPing(kPingAssistMe, To3D(posFor2D), false);
				possibleQPositions.push_back(std::make_pair(count, posFor2D));
			}
		}
	}

	std::sort(possibleQPositions.begin(), possibleQPositions.end(), [](auto &left, auto &right) {
		return left.first > right.first;
	});

	for (int i = 0; i < 15; i++) {
		CloseQPositions.push_back(possibleQPositions[i]);

	}

	std::sort(CloseQPositions.begin(), CloseQPositions.end(), [&](auto &a, auto &b) {
		return Extensions::Dist2D(a.second, pos) < Extensions::Dist2D(b.second, pos); });


	//	for (auto entry : CloseQPositions) {

	//		Q->CastOnPosition(To3D(entry.second));
	//	}

	for (auto entry : possibleQPositions) {

		Q->CastOnPosition(Extensions::To3D(entry.second));
	}
}

Vec3 Orianna::FarmQ(Vec3 pos)
{
	auto posChecked = 0;
	auto maxPosChecked = 52;
	auto posRadius = 65;
	auto radiusIndex = 0;
	std::vector<std::pair<int, Vec2>> CloseQPositions;
	std::vector<std::pair<int, Vec2>> possibleQPositions;



	while (posChecked < maxPosChecked) {
		radiusIndex++;
		auto curRadius = radiusIndex * (0x2 * posRadius);
		auto curCurcleChecks = static_cast<int>(ceil((0x2 * M_PI * curRadius) / (0x2 * static_cast<double>(posRadius))));

		for (auto i = 1; i < curCurcleChecks; i++) {
			posChecked++;

			auto cRadians = (0x2 * M_PI / (curCurcleChecks - 1)) * i;
			auto xPos = static_cast<float>(floor(pos.x + curRadius * cos(cRadians)));
			auto zPos = static_cast<float>(floor(pos.z + curRadius * sin(cRadians)));
			auto posFor2D = Vec2(xPos, zPos);
			auto count = Extensions::CountMinionsInTargetRange(Extensions::To3D(posFor2D), W->Radius());

			//GGame->PrintChat(std::to_string(count).c_str());

			if (GNavMesh->IsPointWall(Extensions::To3D(posFor2D))) {

				// dont push is wall
				continue;
			}

			if (Extensions::Dist2D(posFor2D, GEntityList->Player()->ServerPosition()) > Q->Range())
			{
				// dont push to far away to cast;
				continue;
			}

			if (Extensions::Dist2D(posFor2D, pos) <= Q->Radius())
			{
				//	GGame->ShowPing(kPingAssistMe, To3D(posFor2D), false);
				possibleQPositions.push_back(std::make_pair(count, posFor2D));
			}
		}
	}

	std::sort(possibleQPositions.begin(), possibleQPositions.end(), [](auto &left, auto &right) {
		return left.first > right.first;
	});

	for (int i = 0; i < 15; i++) {
		CloseQPositions.push_back(possibleQPositions[i]);

	}

	std::sort(CloseQPositions.begin(), CloseQPositions.end(), [&](auto &a, auto &b) {
		return Extensions::Dist2D(a.second, pos) < Extensions::Dist2D(b.second, pos); });


	//	for (auto entry : CloseQPositions) {

	//		Q->CastOnPosition(To3D(entry.second));
	//	}

	for (auto entry : possibleQPositions) {

		return (Extensions::To3D(entry.second));

	}
}


int Orianna::SpellCheck(Vec3 pos, int range, double delay)
{

	auto count = 0;
	Vec3 futurePos;

	for (auto target : GEntityList->GetAllHeros(false, true))
	{
		if (target != nullptr && target->IsValidTarget() && target->IsVisible() && !target->IsDead())
		{
			GPrediction->GetFutureUnitPosition(target, delay, true, futurePos);
			if ((futurePos - pos).Length() <= range)
			{
				count++;
			}
		}
	}
	return (count);
}

void Orianna::CastE(IUnit* target)
{
	auto player = GEntityList->Player();
	if (target != nullptr && (target->IsHero() && player->GetMana() > Q->ManaCost() + W->ManaCost() + E->ManaCost()))
	{
		E->CastOnTarget(target);
	}
}

void Orianna::CastQ(IUnit* target)
{
	if (!Q->IsReady())
		return;
	float distance = Extensions::GetDistance(StationaryBall->GetPosition(), target->ServerPosition());
	auto player = GEntityList->Player();

	if (E->IsReady() && player->GetMana() > R->ManaCost() + Q->ManaCost() + W->ManaCost() + E->ManaCost() && distance > Extensions::GetDistance(player->GetPosition(), target->ServerPosition()) + 380)
	{
		E->CastOnPlayer();
		return;
	}

	QCast(Q);


}

void Orianna::eLogic() {
	auto player = GEntityList->Player();//sebby start
	if (isBallMoving() || !E->IsReady())
		return;
	auto ballEnemies = Extensions::EnemiesInRange(StationaryBall->GetPosition(), 600);
	int playerEnemies;
	if (player->GetPosition() != StationaryBall->GetPosition())
	{
		playerEnemies = Extensions::EnemiesInRange(player->GetPosition(), 600);
	}
	if (!Q->IsReady() && player->GetSpellRemainingCooldown(kSlotW) < player->GetSpellRemainingCooldown(kSlotQ) &&
		(ballEnemies > playerEnemies || ballEnemies == playerEnemies))
	{
		return;
	}
	if (GetEHits() > (ballEnemies <= 2 ? 1 : 2)) {//sebby end

		CastE(player);
		return;
	}
	
	for (auto ally : GEntityList->GetAllHeros(true, false))
	{
		if (Extensions::EnemiesInRange(ally->GetPosition(), R->Radius() * 2) >= 2 && (ally->GetPosition() - player->GetPosition()).Length() <= E->Range())
		{
			CastE(ally);
			return;
		}
	}

}

void Orianna::Combo()
{
	eLogic();
	if (W->IsReady() && ComboW->Enabled() && (Extensions::EnemiesInRange(StationaryBall->GetPosition(), W->Radius()) > 0) || Extensions::EnemiesInRange(GetMovingBallPos(), W->Radius())) {
		W->CastOnPlayer();
	}
	if (isBallMoving())
		return;

	auto target1 = GTargetSelector->FindTarget(QuickestKill, SpellDamage, E->Range() + R->Radius() * 2);
	if (target1 == nullptr || !target1->IsHero() || target1->IsDead())
		return;


	if (R->IsReady() && ComboR->Enabled() && Extensions::EnemiesInRange(StationaryBall->GetPosition(), R->Radius() * 2) > 0)
	{
		if (SpellCheck(StationaryBall->GetPosition(), R->Radius(), R->GetDelay()) >= ultMin->GetFloat())
			R->CastOnPlayer();
		if (rDmg(target1) > target1->GetHealth() && KillStealR->Enabled() && (target1->GetPosition() - StationaryBall->GetPosition()).Length() <= R->Radius() - 25)
			R->CastOnPlayer();
	}
	auto target = GTargetSelector->FindTarget(QuickestKill, SpellDamage, Q->Range());
	if (target == nullptr || !target->IsHero() || isBallMoving() || target->IsDead())
		return;


	if (!Q->IsReady() && !W->IsReady() && ComboE->Enabled()) {

	}
	// check if more than X target to try aoe q position
	if (Q->IsReady() && ComboQ->Enabled() && GEntityList->Player()->IsValidTarget(target, Q->Range()) && R->IsReady() && Extensions::EnemiesInRange(target->ServerPosition(), R->Radius() * 2) > 1)
		(TeamFightQ(target->ServerPosition()));
	else
	{
		// do a normal cast not aoe one
		CastQ(target);

	}

	if (E->IsReady() && !(GEntityList->Player()->IsDead()))
	{
		if (GEntityList->Player()->HealthPercent() <= HealthPercent->GetInteger() && Extensions::EnemiesInRange(GEntityList->Player()->GetPosition(), 600) > 0) {
			E->CastOnPlayer();
		} //Cast on self

		  /*if (ShieldTeamate->Enabled())
		  {
		  auto Teamates = GEntityList->GetAllHeros(true, false);
		  for (IUnit* Teamate : Teamates)
		  { if( GetDistance(GEntityList->Player(), Teamate->GetPosition()) <= E->Range() ){
		  if (!(Teamate->IsDead()) && Teamate->HealthPercent() <= ShieldTeamatePercent->GetInteger() && EnemiesInRange(Teamate->GetPosition(), 600) > 0) {
		  E->CastOnUnit(Teamate); } //Cast on injured teamate*/
	}
}



void Orianna::Harass()
{
	if (harassW->Enabled() && W->IsReady() && GEntityList->Player()->ManaPercent() > harassWMana->GetFloat() && SpellCheck(StationaryBall->GetPosition(), W->Radius(), W->GetDelay()))
	{
		W->CastOnPlayer();
	}

	auto target = GTargetSelector->FindTarget(QuickestKill, SpellDamage, Q->Range());
	if (target == nullptr || !target->IsHero())
		return;


	if (harassQ->Enabled() && GEntityList->Player()->ManaPercent() > harassQMana->GetFloat()) {
		if (Q->IsReady() && target != nullptr && GEntityList->Player()->IsValidTarget(target, 900) && Extensions::EnemiesInRange(target->ServerPosition(), R->Radius() * 2) > 1)
			(TeamFightQ(target->ServerPosition()));
		else
		{
			CastQ(target);
		}
	}



}

bool Orianna::onMouseWheel(HWND wnd, UINT message, WPARAM wparam, LPARAM lparam) {
	if (message != 0x20a)
		return true;
	Laneclear->UpdateInteger(!Laneclear->Enabled());

	return false;
}


void Orianna::LaneClear()
{
	if (isBallMoving())
		return;

	if (Laneclear->Enabled()) {
		auto minions = 0;

		for (auto minion : GEntityList->GetAllMinions(false, true, true))
		{
			if (minion != nullptr && minion->IsValidObject() && minion->IsValidTarget() && Extensions::GetDistance(GEntityList->Player(), minion->GetPosition()) <= Q->Range())
			{
				if (!minion->IsDead())
				{
					if (laneClearQ->Enabled() && Q->IsReady() && GEntityList->Player()->ManaPercent() > laneClearQMana->GetFloat()) {
						Q->CastOnPosition(FarmQ(minion->GetPosition()));


					}


					if (Extensions::CountMinionsInTargetRange(StationaryBall->GetPosition(), W->Radius()) > 2 && laneClearW->Enabled() && W->IsReady() && GEntityList->Player()->ManaPercent() > laneClearWMana->GetFloat() && (!isBallMoving())) {
						W->CastOnPlayer();
					}
				}
			}

		}
	}
}

void Orianna::Automatic()
{
	if (GEntityList->Player()->HasBuff("OrianaGhostSelf"))
	{
		BallPosition = GEntityList->Player()->GetPosition();
		StationaryBall = GEntityList->Player();
	}

	std::vector<IUnit*> result;
	for (auto target : GEntityList->GetAllHeros(true, false))
	{
		if (target->HasBuff("orianaghost")) {
			BallPosition = target->GetPosition();
			StationaryBall = target;
			result.push_back(target);

		}

	}

	if (isBallMoving())
		return;
	if (autoQ->Enabled() && Q->IsReady())
	{
		auto target1 = GTargetSelector->FindTarget(QuickestKill, SpellDamage, Q->Range());
		if (target1 == nullptr || !target1->IsHero())
			return;

		CastQ(target1);
	}

	if (autoW->Enabled() && W->IsReady() && GEntityList->Player()->ManaPercent() > harassWMana->GetFloat() && SpellCheck(StationaryBall->GetPosition(), W->Radius(), W->GetDelay() > 0))
	{

		W->CastOnPlayer();

	}
	if (GetAsyncKeyState(FlashUlt->GetInteger()) && !GGame->IsChatOpen() && GUtility->IsLeagueWindowFocused())
	{
		FlashUlting = true;
		PerformFlashUlt();
	}


}

float Orianna::qDmg(IUnit* Target)
{
	float InitDamage = 0;
	float BonusStackDamage = ((0.50 * GEntityList->Player()->TotalMagicDamage()));

	if (GEntityList->Player()->GetSpellLevel(kSlotQ) == 1)
		InitDamage += (60 + BonusStackDamage);
	else if (GEntityList->Player()->GetSpellLevel(kSlotQ) == 2)
		InitDamage += (90 + BonusStackDamage);
	else if (GEntityList->Player()->GetSpellLevel(kSlotQ) == 3)
		InitDamage += (120 + BonusStackDamage);
	else if (GEntityList->Player()->GetSpellLevel(kSlotQ) == 4)
		InitDamage += (150 + BonusStackDamage);
	else if (GEntityList->Player()->GetSpellLevel(kSlotQ) == 5)
		InitDamage += (180 + BonusStackDamage);
	//「 MAGIC DAMAGE: 60 / 90 / 120 / 150 / 180 (+ 50% AP) 」


	return GDamage->CalcMagicDamage(GEntityList->Player(), Target, InitDamage);
}

float Orianna::wDmg(IUnit* Target)
{
	float InitDamage = 0;

	float BonusStackDamage = (0.7 * GEntityList->Player()->TotalMagicDamage());

	if (GEntityList->Player()->GetSpellLevel(kSlotW) == 1)
		InitDamage += 70 + BonusStackDamage;
	else if (GEntityList->Player()->GetSpellLevel(kSlotW) == 2)
		InitDamage += 115 + BonusStackDamage;
	else if (GEntityList->Player()->GetSpellLevel(kSlotW) == 3)
		InitDamage += 160 + BonusStackDamage;
	else if (GEntityList->Player()->GetSpellLevel(kSlotW) == 4)
		InitDamage += 205 + BonusStackDamage;
	else if (GEntityList->Player()->GetSpellLevel(kSlotW) == 5)
		InitDamage += 250 + BonusStackDamage;

	// MAGIC DAMAGE: 70 / 115 / 160 / 205 / 250 (+ 70% AP)

	return GDamage->CalcMagicDamage(GEntityList->Player(), Target, InitDamage);
}

float Orianna::rDmg(IUnit* Target)
{
	float InitDamage = 0;

	float BonusStackDamage = (0.7 * GEntityList->Player()->TotalMagicDamage());

	if (GEntityList->Player()->GetSpellLevel(kSlotR) == 1)
		InitDamage += 150 + BonusStackDamage;
	else if (GEntityList->Player()->GetSpellLevel(kSlotR) == 2)
		InitDamage += 225 + BonusStackDamage;
	else if (GEntityList->Player()->GetSpellLevel(kSlotR) == 3)
		InitDamage += 300 + BonusStackDamage;


	// MAGIC DAMAGE: 150 / 225 / 300 (+ 70% AP)

	return GDamage->CalcMagicDamage(GEntityList->Player(), Target, InitDamage);
}

void Orianna::KillSteal() {


}



//Author = Divine
void Orianna::DrawGagongReplicate(Vec3 BallPos)
{
	Vec3 perp1 = Extensions::Cross(BallPos, UpVec).VectorNormalize() * BallRad;
	Vec3 perp2 = -perp1;
	Vec3 BallPosNorm = Vec3(BallPos).VectorNormalize();
	Vec3 forwardExtend = (BallPosNorm*BallRad);
	Vec2 UpperLeft(-1, -1), BottomRight(-1, -1), BottomLeft(-1, -1), UpperRight(-1, -1);
	Vec2 Left(-1, -1), Right(-1, -1), Bottom(-1, -1), Up(-1, -1);
	Vec3 v3Left = Extensions::Cross(perp1, UpVec).VectorNormalize() * BallRad;
	Vec3 v3Right = Extensions::Cross(perp2, UpVec).VectorNormalize() * BallRad;
	Vec3 BallPosNoY(Vec3(BallPos.x, 0, BallPos.z));
	//Vec3 BallPosNoYZ(Vec3(BallPos.x, 0, 0));

	if (GGame->Time() - lastTimeStamp > 0.05)
	{
		lastTimeStamp = GGame->Time();
		if (gagongAngle > 360)gagongAngle = 0;
		gagongAngle += TO_RAD(5);
	}
	if (GGame->Projection(BallPosNoY + Extensions::RotateZ(perp1 + BallPos, BallPos, gagongAngle), &UpperLeft) && GGame->Projection(BallPosNoY + Extensions::RotateZ(perp2 + BallPos, BallPos, gagongAngle),
		&BottomRight) && GGame->Projection(BallPosNoY + Extensions::RotateZ(BallPos + forwardExtend, BallPos, gagongAngle), &UpperRight) && GGame->Projection(BallPosNoY + Extensions::RotateZ(BallPos - forwardExtend, BallPos, gagongAngle), &BottomLeft))
	{
		float width = 0.1;
		for (float i = 0; i < 5; i += 0.3)
		{

			UpperLeft -= Vec2(i * width, i * width);
			UpperRight += Vec2(i * width, -(i * width));
			BottomLeft += Vec2(-i * width, i * width);
			BottomRight += Vec2(i * width, i * width);

			GRender->DrawLine(UpperLeft, UpperRight, GagongColorsMove[1]);
			GRender->DrawLine(UpperLeft, BottomLeft, GagongColorsMove[2]);
			GRender->DrawLine(UpperRight, BottomRight, GagongColorsMove[3]);
			GRender->DrawLine(BottomLeft, BottomRight, GagongColorsMove[4]);
		}

	}

	if (GGame->Projection(BallPosNoY + Extensions::RotateZ(perp1*0.73 + v3Left*0.8 + BallPos, BallPos, -gagongAngle), &Left) && GGame->Projection(BallPosNoY + Extensions::RotateZ(perp1*0.73 - v3Left*0.8 + BallPos, BallPos, -gagongAngle),
		&Up) && GGame->Projection(BallPosNoY + Extensions::RotateZ(perp2*0.73 - v3Right*0.8 + BallPos, BallPos, -gagongAngle), &Bottom) && GGame->Projection(BallPosNoY + Extensions::RotateZ(perp2*0.73 + v3Right*0.8 + BallPos, BallPos, -gagongAngle), &Right))
	{

		float width = 0.1;
		for (float i = 0; i < 6; i += 0.3)
		{

			Left.x -= i * width;
			Right.x += i * width;
			Up.y -= i*width;
			Bottom.y += i*width;

			GRender->DrawLine(Left, Up, GagongColorsMove[5]);
			GRender->DrawLine(Left, Bottom, GagongColorsMove[6]);
			GRender->DrawLine(Right, Up, GagongColorsMove[7]);
			GRender->DrawLine(Right, Bottom, GagongColorsMove[8]);
		}
	}
	//GRender->DrawOutlinedCircle(BallPos, GagongColors[0], 83);

	int spiral_num_segments = 50;
	int spiral_rad = 50;
	Vec3 spiral_prev_vec(-1, -1, -1);
	Vec2 currv2D(-1, -1), prev2D(-1, -1);


	for (int i = 0; i < spiral_num_segments; i++)
	{
		float theta = 10.0f * 3.1415926f * i / spiral_num_segments; //the current angle

		float x = (spiral_rad / spiral_num_segments)*i * cosf(theta); //the x component
		float z = (spiral_rad / spiral_num_segments)*i * sinf(theta); //the y component
		Vec3 spiral_curr_vec(x + BallPos.x, GNavMesh->GetHeightForPoint(Vec2(BallPos.x, BallPos.z)), z + BallPos.z);

		if (spiral_prev_vec.x == -1)spiral_prev_vec = BallPos;
		if (GGame->Projection(spiral_curr_vec, &currv2D) && GGame->Projection(spiral_prev_vec, &prev2D))
			//GRender->DrawLine(currv2D, prev2D, i? GagongColors[1]:GagongColors[0]);
			if (!isBallMoving()) {
				GRender->DrawTextW(currv2D, GagongColors[0], "*");
			}
			else {

				GRender->DrawTextW(currv2D, GagongColors[1], "*");

			}

			spiral_prev_vec = spiral_curr_vec;

	}
}

void Orianna::dmgdraw()
{
	if (drawDmg->Enabled()) {
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
					WDamage = wDmg(hero);
				}
				if (Q->IsReady()) {
					QDamage = qDmg(hero);
				}
				if (R->IsReady()) {
					RDamage = rDmg(hero) + GDamage->GetAutoAttackDamage(GEntityList->Player(), hero, true);
				}

				float totalDamage = QDamage + WDamage + EDamage + RDamage;
				float percentHealthAfterDamage = max(0, hero->GetHealth() - float(totalDamage)) / hero->GetMaxHealth();
				float yPos = barPos.y + yOffset;
				float xPosDamage = (barPos.x + xOffset) + Width * percentHealthAfterDamage;
				float xPosCurrentHp = barPos.x + xOffset + Width * (hero->GetHealth() / hero->GetMaxHealth());
				if (!hero->IsDead() && hero->IsValidTarget())
				{
					float differenceInHP = xPosCurrentHp - xPosDamage;
					float pos1 = barPos.x + 9 + (107 * percentHealthAfterDamage);

					for (int i = 0; i < differenceInHP; i++)
					{
						GRender->DrawLine(Vec2(pos1 + i, yPos), Vec2(pos1 + i, yPos + Height), FillColor);
					}
					if (!hero->IsVisible())
					{

					}
				}
			}
		}
	}
}
//Author Divine Nader[Sl]
void Orianna::DrawGagonDix(Vec3 BallPos, Vec4 Color)
{
	Vec3 perp1 = Extensions::Cross(BallPos, UpVec).VectorNormalize() * BallRad;
	Vec3 perp2 = -perp1;
	Vec3 BallPosNorm = Vec3(BallPos).VectorNormalize();
	Vec3 forwardExtend = (BallPosNorm*BallRad);
	Vec2 UpperLeft(-1, -1), BottomRight(-1, -1), BottomLeft(-1, -1), UpperRight(-1, -1);
	Vec2 Left(-1, -1), Right(-1, -1), Bottom(-1, -1), Up(-1, -1);
	Vec3 v3Left = Extensions::Cross(perp1, UpVec).VectorNormalize() * BallRad;
	Vec3 v3Right = Extensions::Cross(perp2, UpVec).VectorNormalize() * BallRad;
	if (GGame->Projection(perp1 + BallPos, &UpperLeft) && GGame->Projection(perp2 + BallPos, &BottomRight) && GGame->Projection(BallPos + forwardExtend, &UpperRight) && GGame->Projection(BallPos - forwardExtend, &BottomLeft))
	{
		float width = 0.1;
		for (float i = 0; i < 5; i += 0.3)
		{

			UpperLeft -= Vec2(i * width, i * width);
			UpperRight += Vec2(i * width, -(i * width));
			BottomLeft += Vec2(-i * width, i * width);
			BottomRight += Vec2(i * width, i * width);

			GRender->DrawLine(UpperLeft, UpperRight, Color);
			GRender->DrawLine(UpperLeft, BottomLeft, Color);
			GRender->DrawLine(UpperRight, BottomRight, Color);
			GRender->DrawLine(BottomLeft, BottomRight, Color);
		}

	}

	if (GGame->Projection(BallPos + perp1*0.73 + v3Left*0.8, &Left) && GGame->Projection(BallPos + perp1*0.73 - v3Left*0.8, &Up) && GGame->Projection(BallPos + perp2*0.73 - v3Right*0.8, &Bottom) && GGame->Projection(BallPos + perp2*0.73 + v3Right*0.8, &Right))
	{

		float width = 0.1;
		for (float i = 0; i < 4; i += 0.3)
		{

			Left.x -= i * width;
			Right.x += i * width;
			Up.y -= i*width;
			Bottom.y += i*width;

			GRender->DrawLine(Left, Up, Color);
			GRender->DrawLine(Left, Bottom, Color);
			GRender->DrawLine(Right, Up, Color);
			GRender->DrawLine(Right, Bottom, Color);
		}
	}
	GRender->DrawOutlinedCircle(BallPos, Color, 83);
}

void Orianna::Drawing()
{
	auto Hero = GEntityList->Player();
	auto player = GEntityList->Player();
	if (drawBall->Enabled() && ballSelect->GetInteger() == 0)
	{
		if (StationaryBall && !isBallMoving() && (StationaryBall->IsValidObject() || StationaryBall->IsValidTarget()))
		{
			Vec3 Pos = StationaryBall->GetPosition();
			DrawGagongReplicate(Pos);

		}
		if (isBallMoving())
		{
			DrawGagongReplicate(GetMovingBallPos());
		}
	}
	else if (drawBall->Enabled() && ballSelect->GetInteger() == 1)
	{
		if (StationaryBall && (StationaryBall->IsValidObject() || StationaryBall->IsValidTarget()))
		{
			Vec3 Pos = StationaryBall->GetPosition();
			DrawGagonDix(Pos, BallIndicatorColor);

		}
		if (isBallMoving())
		{
			DrawGagonDix(GetMovingBallPos(), BallIndicatorMovingColor);
		}


	}



	if (DrawReady->Enabled())
	{
		if (Q->IsReady() && DrawQ->Enabled())

		{
			GRender->DrawCircle(player->GetPosition(), Q->Range(), Vec4(225, 225, 0, 225), 1, false);
		}

		if (W->IsReady() && DrawW->Enabled())
		{
			GRender->DrawCircle(player->GetPosition(), W->Range(), Vec4(225, 225, 0, 225), 1, false);
		}

		if (E->IsReady() && DrawE->Enabled())
		{
			GRender->DrawCircle(player->GetPosition(), E->Range(), Vec4(225, 225, 0, 225), 1, false);
		}


	}

	else
	{
		if (DrawQ->Enabled())
		{
			GRender->DrawCircle(player->GetPosition(), Q->Range(), Vec4(225, 225, 0, 225), 1, false);
		}

		if (DrawW->Enabled())
		{
			GRender->DrawCircle(player->GetPosition(), W->Range(), Vec4(225, 225, 0, 225), 1, false);
		}

		if (DrawE->Enabled())
		{
			GRender->DrawCircle(player->GetPosition(), E->Range(), Vec4(225, 225, 0, 225), 1, false);
		}


	}
}