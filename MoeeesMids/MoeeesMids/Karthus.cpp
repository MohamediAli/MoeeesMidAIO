#define NOMINMAX
#include "Karthus.h"
#include <algorithm>
#include "Extensions.h"
#include "Praedictio.h"
#define M_PI 3.14159265358979323846


Karthus::~Karthus()
{
    KarthusMenu->Remove();
}


Karthus::Karthus(IMenu* Parent, IUnit* Hero) :Champion(Parent, Hero)
{
    Q = GPluginSDK->CreateSpell2(kSlotQ, kLineCast, false, true, kCollidesWithNothing);
    Q->SetSkillshot(1.f, 140.f, FLT_MAX, 890.f);
    W = GPluginSDK->CreateSpell2(kSlotW, kCircleCast, false, true, kCollidesWithNothing);
    W->SetSkillshot(1.0f, 200.f, FLT_MAX, 1000.f);
    E = GPluginSDK->CreateSpell2(kSlotE, kConeCast, false, true, kCollidesWithWalls);
    E->SetSkillshot(0.30f, 550.f, FLT_MAX, 550.f);
    R = GPluginSDK->CreateSpell2(kSlotR, kTargetCast, false, false, kCollidesWithNothing);
    KarthusMenu      = Parent->AddMenu("Karthus Menu");
    ComboMenu        = Parent->AddMenu("Combo");
    qMenu            = Parent->AddMenu("Q Settings");
    Prediction       = Parent->AddMenu("Prediction");
    wMenu            = Parent->AddMenu("W Settings");
    eMenu            = Parent->AddMenu("E Settings");
    rMenu            = Parent->AddMenu("R Settings");
    Drawings         = Parent->AddMenu("All Drawings");
    MiscMenu         = Parent->AddMenu("Miscs");
    ComboQ           = qMenu->CheckBox("Use Q in Combo", true);
    autoQ            = qMenu->CheckBox("Automatic Harass Q", true);
    harassQ          = qMenu->CheckBox("Harass with Q", true);
    harassQMana      = qMenu->AddFloat(":: Only Harras Q if Mana >", 0, 100, 50);
    lastHitQ         = qMenu->CheckBox("Only last hit Q if outside of AA Range", true);
    laneClearQ       = qMenu->CheckBox("Wave Clear with Q", true);
    laneClearQMana   = qMenu->AddFloat(":: Only Wave Clear Q if Mana >", 0, 100, 70);
    JungleClearQ     = qMenu->CheckBox("Jungle Clear with Q", true);
    JungleClearQMana = qMenu->AddFloat(":: Only Jungle Clear Q if Mana >", 0, 100, 50);
    ComboW           = wMenu->CheckBox("Use W in Combo", true);
    harassW          = wMenu->CheckBox("Harass with W", false);
    gapCloserW       = wMenu->CheckBox("W on Gap Closer", true);
    ComboE           = eMenu->CheckBox("Use E in Combo", true);
    harassE          = eMenu->CheckBox("Harass with E", true);
    harassEMana      = eMenu->AddFloat(":: Only Harras E if Mana >", -1000, 1000, 50);
    laneClearE       = eMenu->CheckBox("Wave Clear with E", true);
    laneClearEMana   = eMenu->AddFloat(":: Only Wave Clear E if Mana >", 0, 100, 70);
    JungleClearE     = eMenu->CheckBox("Jungle Clear with E", true);
    JungleClearEMana = eMenu->AddFloat(":: Only Jungle Clear E if Mana >", 0, 100, 50);
    EAutoOff         = eMenu->CheckBox("Automaticlly Turn Off E (Doesn't include combo) ", true);
    Ping0            = rMenu->CheckBox("Ping on Killiable on Ult", true);
    Ping             = { "Normal", "Danger", "Enemy Missing","OMW","Fall Back","Assist" };
    PingOption       = rMenu->AddSelection("Ping Selection", 1, Ping);
    PingDelay        = rMenu->AddInteger("Ping Delay in 100ms", 1, 10, 4);
    PredType         = { "SN Style", "Praedictio", "Core" };
    PredictionType   = Prediction->AddSelection("Choose Prediction Type", 0, PredType);
    ComboAALevel     = MiscMenu->AddInteger("At what level disable AA", 1, 18, 6);
    ComboAA          = MiscMenu->CheckBox("Disable AA", false);
    ComboAAkey       = MiscMenu->AddKey("Disable key", 32);
    DrawReady        = Drawings->CheckBox("Draw Ready Spells", true);
    drawDmg          = Drawings->CheckBox("Draw R killable", true);
    DrawQ            = Drawings->CheckBox("Draw Q", true);
    DrawW            = Drawings->CheckBox("Draw W", true);
    DrawE            = Drawings->CheckBox("Draw E", false);
}

bool Karthus::GetImpactTime(ISpell2* spell, IUnit* source, IUnit* unit, float& impact)
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
    auto unitDistance = Extensions::GetDistance(sourcePosition, unitPosition);
    //sourcePosition : dist(unitPosition)
    //Calculations //
    auto a = (unitSpeed * unitSpeed) - (spell->Speed() * spell->Speed());
    auto b = 2 * unitSpeed * unitDistance * theta;
    auto c = unitDistance * unitDistance;
    auto discriminant = b * b - 4 * a * c;
    if(discriminant < 0)
    {
        return false;
    }
    impact = 2 * c / (sqrt(discriminant) - b);
    if(impact < 0)
    {
        return false;
    }
    return true;
}

bool Karthus::GetCastPosition(ISpell2* spell, IUnit* source, IUnit* unit, Vec3& cast)
{
    Vec3 castPosition;
    auto index = 0;
    auto path = unit->GetWaypointList();
    if(unit->IsMoving() && path.size() > 1)
    {
        for(auto pathIndex : path)
        {
            auto unitPosition = unit->ServerPosition();
            auto unitPath = pathIndex;
            auto unitDirection = (pathIndex - unitPosition).VectorNormalize();
            float impactTime;
            if(GetImpactTime(spell, source, unit, impactTime))
            {
                if(!impactTime)
                {
                    return false;
                }
                castPosition = unitPosition + unitDirection * (unit->MovementSpeed() * impactTime);
                index++;
            }
            if(Extensions::GetDistance(source->ServerPosition(), castPosition) < spell->Range())
            {
                cast = castPosition;
                return true;
            }
        }
    }
    return false;
}

void Karthus::CastQ(IUnit* target)
{
    if(PredictionType->GetInteger() == 2)
    {
        Q->SetSkillshot(1.f, 140.f, 2000, 890.f);
        Q->CastOnTarget(target, kHitChanceMedium);
    }
    if(PredictionType->GetInteger() == 1)
    {
        Q->SetSkillshot(1.f, 140.f, 2000, 890.f);
        AdvPredictionOutput prediction_output;
        Q->RunPrediction(target, false, kCollidesWithYasuoWall | kCollidesWithMinions, &prediction_output);
        if(prediction_output.HitChance >= kHitChanceHigh)
        {
            Praedictio::Cast(Hero, Q, target);
        }
    }
    if(PredictionType->GetInteger() == 0)
    {
        Q->SetSkillshot(1.f, 140.f, FLT_MAX, 890.f);
        if(!cz)
        {
            Q->CastOnPosition(PredPos(QTarget, 0.75f + (GGame->Latency() / 1000)));
        }
        else
        {
            Q->SetSkillshot(1.f, 140.f, 2000, 890.f);
            Praedictio::Cast(Hero, Q, target);
        }
    }
}

void Karthus::OnGameUpdate()
{
    automatic();
    zigzag();
    if(!GUtility->IsLeagueWindowFocused() || GGame->IsChatOpen() || GGame->IsShopOpen())
    {
        return;
    }
    QTarget = GTargetSelector->FindTarget(QuickestKill, SpellDamage, Q->Range());
    if(GOrbwalking->GetOrbwalkingMode() == kModeCombo)
    {
        Combo();
    }
    if(GOrbwalking->GetOrbwalkingMode() == kModeMixed)
    {
        LastHit();
        Harass();
    }
    if(GOrbwalking->GetOrbwalkingMode() == kModeLastHit)
    {
        LastHit();
    }
    if(GOrbwalking->GetOrbwalkingMode() == kModeLaneClear)
    {
        LaneClear();
    }
    if(GetAsyncKeyState(ComboAAkey->GetInteger()))
    {
        auto level = Hero->GetLevel();
        if(ComboAA->Enabled() && level >= ComboAALevel->GetInteger() && Hero->GetMana() > 100)
        {
            GOrbwalking->SetAttacksAllowed(false);
        }
    }
    if(!GetAsyncKeyState(ComboAAkey->GetInteger()) || Hero->GetMana() < 100)
    {
        {
            GOrbwalking->SetAttacksAllowed(true);
        }
    }
}
void Karthus::OnRender()
{
    Drawing();
    dmgdraw();
}

void Karthus::AntiGapclose(GapCloserSpell const& args)
{
    if(W->IsReady() && gapCloserW->Enabled() && Hero->IsValidTarget(args.Source, W->Range()) && args.Source->IsEnemy(Hero))
    {
        AdvPredictionOutput outputfam;
        W->RunPrediction(args.Source, false, kCollidesWithMinions, &outputfam);
        if(outputfam.HitChance == kHitChanceDashing)
        {
            W->CastOnTarget(args.Source, kHitChanceDashing);
        }
    }
}

void Karthus::automatic()
{
    if(E->IsReady() && Hero->HasBuff("KarthusDefile") && (EAutoOff->Enabled() || GOrbwalking->GetOrbwalkingMode() == kModeCombo))
    {
        if(Extensions::CountMinionsInTargetRange(Hero->ServerPosition(), E->Range()) > 2 && (GOrbwalking->GetOrbwalkingMode() == kModeLaneClear))
        {
            return;
        }
        if(Extensions::EnemiesInRange(Hero->ServerPosition(), E->Range()) && (GOrbwalking->GetOrbwalkingMode() == kModeCombo || GOrbwalking->GetOrbwalkingMode() == kModeMixed))
        {
            return;
        }
        E->CastOnPlayer();
    }
    if(autoQ->Enabled() && Q->IsReady())
    {
        if(QTarget == nullptr || !QTarget->IsHero() || !QTarget->IsValidTarget() ||!Extensions::Validate(QTarget))
        {
            return;
        }
        CastQ(QTarget);
    }
}


void Karthus::eToggle()
{
    if(E->IsReady() && !Hero->HasBuff("KarthusDefile"))
    {
        E->CastOnPlayer();
    }
}

float Karthus::qWidthChange(IUnit* target)
{
    return 	std::max(30.f, (1.f - (Extensions::GetDistance(Hero, target->ServerPosition())) / Q->Range()) * 160.f);
}

float Karthus::wWidthChange(IUnit* target)
{
    return 	std::max(70.f, (1.f - (Extensions::GetDistance(Hero, target->ServerPosition())) / W->Range()) * 160.f);
}

void Karthus::zigzag()   //credits sn karthus
{
    if(QTarget == nullptr || !QTarget->IsHero() || !QTarget->IsValidTarget() || !Extensions::Validate(QTarget) ||  !(QTarget->IsVisible()))
    {
        return;
    }
    if(czx < czx2)
    {
        if(czx2 >= QTarget->ServerPosition().x)
        {
            cz = true;
        }
        else
        {
            cz = false;
        }
    }
    else if(czx == czx2)
    {
        cz = false;
        czx = czx2;
        czx2 = (QTarget->ServerPosition().x);
        return;
    }
    else
    {
        if(czx2 <= QTarget->ServerPosition().x)
        {
            cz = true;
        }
        else
        {
            cz = false;
        }
    }
    czx = czx2;
    czx2 = QTarget->ServerPosition().x;
    if(czy < czy2)
    {
        if(czy2 >= QTarget->ServerPosition().z)
        {
            cz = true;
        }
        else
        {
            cz = false;
        }
    }
    else if(czy == czy2)
    {
        cz = false;
    }
    else
    {
        if(czy2 <= QTarget->ServerPosition().z)
        {
            cz = true;
        }
        else
        {
            cz = false;
        }
    }
    czy = czy2;
    czy2 = QTarget->ServerPosition().z;
}

Vec3 Karthus::PredPos(IUnit* Hero, float Delay)     //credits sn karthus
{
    float value;
    if(Hero->IsFacing(Hero))
    {
        value = (50.f - Hero->BoundingRadius());
    }
    else
    {
        value = - (100.f - Hero->BoundingRadius());
    }
    auto distance = Delay * Hero->MovementSpeed() + value;
    auto path = Hero->GetWaypointList();
    for(auto i = 0; i < path.size() - 1; i++)
    {
        auto a = path[i];
        auto b = path[i + 1];
        auto d = Extensions::GetDistance(a, b);
        if(d < distance)
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
    return 700 + 100 * GEntityList->Player()->GetSpellLevel(kSlotW);
}

float Karthus::WMaxRangeSqr()
{
    float WHalfWidth = WWidth() / 2.0f;
    return W->Range() * W->Range() + WHalfWidth*WHalfWidth;
}

float Karthus::WMaxRange()
{
    return std::sqrt(WMaxRangeSqr());
}

bool Karthus::IsInWRange(Vec2 position)
{
    return Extensions::GetDistanceSqr(GEntityList->Player()->ServerPosition().To2D(), position) < WMaxRangeSqr();
}

//Divine Nader[Sl]
void  Karthus::CastW()
{
    auto target = GTargetSelector->FindTarget(QuickestKill, SpellDamage, WMaxRange());
    if(target == nullptr || !target->IsValidTarget() || target->IsDead())
    {
        return;
    }
    Vec3 futurePos;
    GPrediction->GetFutureUnitPosition(target, W->GetDelay(), true, futurePos);
    if(IsInWRange(futurePos.To2D()))
        if(Hero->IsValidTarget(target, W->Range()))
        {
            W->CastOnTarget(target);
            return;
        }
        else
        {
            auto myPos = Hero->ServerPosition();
            auto myPos2D = myPos.To2D();
            auto targetPos = futurePos;
            auto targetPos2D = targetPos.To2D();
            float x = W->Range();
            float y = std::sqrt(Extensions::GetDistanceSqr(myPos, targetPos) - x*x);
            float z = Extensions::GetDistance(myPos, targetPos);
            float angle = (std::acos((y*y + z*z - x*x) / (2.0f * y * z)));   // *PI / 180; //convert to radian
            auto direction = (myPos2D - targetPos2D).VectorNormalize().Rotated(angle);
            auto castPosition = Extensions::To3D(targetPos2D + y * direction);
            W->CastOnPosition(castPosition);
            return;
        }
}

Vec3 Karthus::FarmQ(Vec3 pos)
{
    auto posChecked = 0;
    auto maxPosChecked = 52;
    auto posRadius = 65;
    auto radiusIndex = 0;
    std::vector<std::pair<int, Vec2>> CloseQPositions;
    std::vector<std::pair<int, Vec2>> possibleQPositions;
    while(posChecked < maxPosChecked)
    {
        radiusIndex++;
        auto curRadius = radiusIndex * (0x2 * posRadius);
        auto curCurcleChecks = static_cast<int>(ceil((0x2 * M_PI * curRadius) / (0x2 * static_cast<double>(posRadius))));
        for(auto i = 1; i < curCurcleChecks; i++)
        {
            posChecked++;
            auto cRadians = (0x2 * M_PI / (curCurcleChecks - 1)) * i;
            auto xPos = static_cast<float>(floor(pos.x + curRadius * cos(cRadians)));
            auto zPos = static_cast<float>(floor(pos.z + curRadius * sin(cRadians)));
            auto posFor2D = Vec2(xPos, zPos);
            auto count = Extensions::CountMinionsInTargetRange(Extensions::To3D(posFor2D), 160.f);
            //GGame->PrintChat(std::to_string(count).c_str());
            if(GNavMesh->IsPointWall(Extensions::To3D(posFor2D)))
            {
                // dont push is wall
                continue;
            }
            if(Extensions::Dist2D(posFor2D, Hero->ServerPosition()) > Q->Range())
            {
                // dont push to far away to cast;
                continue;
            }
            if(Extensions::Dist2D(posFor2D, pos) <= Q->Radius())
            {
                //	GGame->ShowPing(kPingAssistMe, To3D(posFor2D), false);
                possibleQPositions.push_back(std::make_pair(count, posFor2D));
            }
        }
    }
    std::sort(possibleQPositions.begin(), possibleQPositions.end(), [](auto &left, auto &right)
    {
        return left.first > right.first;
    });
    for(auto entry : possibleQPositions)
    {
        return (Extensions::To3D(entry.second));
    }
}

void Karthus::Combo()
{
    if(ComboW->Enabled() && W->IsReady())
    {
        CastW();
    }
    auto target = GTargetSelector->FindTarget(QuickestKill, SpellDamage, Q->Range());
    if(target == nullptr || !target->IsHero() || target->IsDead() || !Extensions::Validate(QTarget) || !(QTarget->IsVisible()))
    {
        return;
    }
    if(ComboQ->Enabled() && Q->IsReady() && Hero->IsValidTarget(target, Q->Range()))
    {
        CastQ(QTarget);
    }
    if(Extensions::EnemiesInRange(Hero->ServerPosition(), E->Range()) && ComboE->Enabled())
    {
        eToggle();
    }
}

void Karthus::Harass()
{
    auto target = GTargetSelector->FindTarget(QuickestKill, SpellDamage, Q->Range());
    if(!Extensions::Validate(target) || !target->IsHero() || target->IsDead() || !Extensions::Validate(QTarget) || !(QTarget->IsVisible()))
    {
        return;
    }
    if(harassQ->Enabled() && Q->IsReady() && Hero->IsValidTarget(target, Q->Range()) && Hero->ManaPercent() >= harassQMana->GetFloat())
    {
        CastQ(QTarget);
    }
    if(harassW->Enabled() && W->IsReady() && Hero->IsValidTarget(target, W->Range()))
    {
        W->SetOverrideRadius(wWidthChange(target));
        W->CastOnTarget(target);
    }
    if(Extensions::EnemiesInRange(Hero->ServerPosition(), E->Range()) && harassE->Enabled() && Hero->ManaPercent() >= harassEMana->GetFloat())
    {
        eToggle();
    }
}


float Karthus::qDmg(IUnit* Target)
{
    float InitDamage = 0;
    float BonusStackDamage = ((0.3 * Hero->TotalMagicDamage()));
    if(Hero->GetSpellLevel(kSlotQ) == 1)
    {
        InitDamage += (40 + BonusStackDamage);
    }
    else if(Hero->GetSpellLevel(kSlotQ) == 2)
    {
        InitDamage += (60 + BonusStackDamage);
    }
    else if(Hero->GetSpellLevel(kSlotQ) == 3)
    {
        InitDamage += (80 + BonusStackDamage);
    }
    else if(Hero->GetSpellLevel(kSlotQ) == 4)
    {
        InitDamage += (100 + BonusStackDamage);
    }
    else if(Hero->GetSpellLevel(kSlotQ) == 5)
    {
        InitDamage += (120 + BonusStackDamage);
    }
    return GDamage->CalcMagicDamage(Hero, Target, InitDamage);
}

float Karthus::rDmg(IUnit* Target)
{
    float InitDamage = 0;
    float BonusStackDamage = (0.6 * Hero->TotalMagicDamage());
    if(Hero->GetSpellLevel(kSlotR) == 1)
    {
        InitDamage += 250 + BonusStackDamage;
    }
    else if(Hero->GetSpellLevel(kSlotR) == 2)
    {
        InitDamage += 400 + BonusStackDamage;
    }
    else if(Hero->GetSpellLevel(kSlotR) == 3)
    {
        InitDamage += 550 + BonusStackDamage;
    }
    return GDamage->CalcMagicDamage(Hero, Target, InitDamage);
}

void Karthus::dmgdraw()
{
    std::string killable = "Killable : ";
    if(R->IsReady() && !Hero->IsDead())
    {
        for(auto enemy : GEntityList->GetAllHeros(false, true))
        {
            if(!enemy->IsDead())
            {
                auto health = enemy->GetHealth() + enemy->HPRegenRate() * R->GetDelay();
                if(enemy->IsVisible())
                {
                    LastSeen = GGame->Time();
                }
                if(health < rDmg(enemy) && GGame->Time()-LastSeen<8)
                {
                    killable += enemy->ChampionName();
                    killable.append(" ");
                    if(GGame->TickCount() - LastPing > 1 && enemy->IsVisible() && Ping0->Enabled())
                    {
                        LastPing = GGame->TickCount() + PingDelay->GetInteger() *100;
                        GGame->ShowPing(PingOption->GetInteger()+1, enemy, true);
                    }
                }
            }
        }
    }
    if(killable != "Killable : ")
    {
        //Vec2 pos;
        static Vec2 Resoution = GRender->ScreenSize();
        static Vec2 ScreenCenter = Vec2(Resoution.x / 3.0f, (Resoution.y / 4.7f));
        //if (GGame->Projection(Hero->ServerPosition(), &pos)) {
        auto message = GRender->CreateFontW("Impact", 60.f, kFontWeightNormal);
        message->SetColor(Vec4(255, 0, 0, 255));
        message->SetOutline(true);
        message->Render(ScreenCenter.x, ScreenCenter.y, killable.c_str());
        //	message->Render(pos.x + 10 + 32, pos.y + 10, killable.c_str());
    }
}
bool myKarthfunctionHP(IUnit* i, IUnit* j) { return (i->GetHealth() >= j->GetHealth()); }




void Karthus::LaneClear()
{
    if(Extensions::CountMinionsInTargetRange(Hero->ServerPosition(), E->Range()) > 2 && E->IsReady() && laneClearE->Enabled() && Hero->ManaPercent() >= laneClearEMana->GetFloat())
    {
        eToggle();
    }
    Q->SetOverrideRadius(160.f);
    if(Q->IsReady() && laneClearQ->Enabled() && Hero->ManaPercent() >= laneClearQMana->GetFloat())
    {
        for(auto minion : GEntityList->GetAllMinions(false, true, false))
        {
            if(Extensions::Validate(minion) && !minion->IsWard() && minion->IsCreep() && Extensions::GetDistance(Hero, minion->ServerPosition()) <= 1000)
            {
                if(!minion->IsDead())
                {
                    auto health = minion->GetHealth();
                    auto hp = GHealthPrediction->GetPredictedHealth(minion, kLastHitPrediction, 1000 + GGame->Latency(), 0);
                    if(hp < qDmg(minion) && hp > health - hp * 2)
                    {
                        Q->CastOnPosition(minion->ServerPosition());
                        return;
                    }
                    Q->CastOnPosition(FarmQ(minion->ServerPosition()));
                    return;
                }
            }
        }
        std::vector<IUnit*> allMobs;
        for(auto mob : GEntityList->GetAllMinions(false, false, true))
        {
            if(mob != nullptr && mob->IsValidTarget() && mob->IsJungleCreep() && mob->IsVisible() && Extensions::GetDistance(Hero, mob->ServerPosition()) <= Q->Range())
            {
                if(!mob->IsDead() && mob->PhysicalDamage() > 1)
                {
                    allMobs.push_back(mob);
                }
            }
        }
        if(allMobs.size() >= 1 && Q->IsReady() && laneClearQ->Enabled() && Hero->ManaPercent() > laneClearQMana->GetFloat())
        {
            std::sort(allMobs.begin(), allMobs.end(), myKarthfunctionHP);
            if(!allMobs[0]->IsMoving())
            {
                Q->CastOnUnit(allMobs[0]);
            }
            else
            {
                Q->SetSkillshot(1.f, 140.f, 1800, 890.f);
                Vec3 castPos;
                if(GetCastPosition(Q, Hero, allMobs[0], castPos))
                {
                    Q->CastOnPosition(castPos);
                    Q->SetSkillshot(1.f, 140.f, FLT_MAX, 890.f);
                }
            }
        }
    }
}



void Karthus::LastHit()
{
    Q->SetOverrideRadius(160.f);
    if(Q->IsReady())
    {
        std::vector<IUnit*> allMinions;
        std::vector<IUnit*> enemyMinions;
        if(!lastHitQ->Enabled())
        {
            for(auto minion : GEntityList->GetAllMinions(false, true, true))
            {
                if(minion != nullptr && !minion->IsWard() && minion->IsCreep() && Extensions::GetDistance(GEntityList->Player(), minion->ServerPosition()) <= Q->Range())
                {
                    if(!minion->IsDead())
                    {
                        allMinions.push_back(minion);
                    }
                }
            }
        }
        else
        {
            for(auto minion : GEntityList->GetAllMinions(false, true, true))
            {
                if(minion != nullptr && !minion->IsWard() && minion->IsCreep() && Extensions::GetDistance(GEntityList->Player(), minion->ServerPosition()) <= Q->Range() && Extensions::GetDistance(GEntityList->Player(), minion->ServerPosition()) > Hero->AttackRange())
                {
                    if(!minion->IsDead())
                    {
                        allMinions.push_back(minion);
                    }
                }
            }
        }
        for(auto minion : GEntityList->GetAllMinions(false, true, false))
        {
            if(minion != nullptr && !minion->IsWard() && minion->IsCreep() && Extensions::GetDistance(GEntityList->Player(), minion->ServerPosition()) <= Q->Range())
            {
                if(!minion->IsDead())
                {
                    enemyMinions.push_back(minion);
                }
            }
        }
        for(auto killable : allMinions)
        {
            if(killable->GetHealth() > GDamage->GetSpellDamage(GEntityList->Player(), killable, kSlotQ))
            {
                allMinions.erase(std::remove(allMinions.begin(), allMinions.end(), killable), allMinions.end());
            }
        }
        auto i = std::vector<int> { -100, -70, 0, 70, 100 };
        auto j = std::vector<int> { -100, -70, 0, 70, 100 };
        for(auto lastMinion : allMinions)
        {
            for(auto xOffset:i)
            {
                for(auto zOffset :j)
                {
                    auto count = 0;
                    Vec3 pos1;
                    GPrediction->GetFutureUnitPosition(lastMinion, 0.25, false, pos1);
                    pos1 = Vec3(pos1.x + xOffset, pos1.y, pos1.z + zOffset);
                    for(auto futureMinions : enemyMinions)
                    {
                        Vec3 pos2;
                        GPrediction->GetFutureUnitPosition(futureMinions, 0.25, false, pos2);
                        if(Extensions::GetDistance(pos1,pos2) < 200)
                        {
                            count++;
                        }
                    }
                    if(count == 1 && GHealthPrediction->GetPredictedHealth(lastMinion,kLastHitPrediction,0.45,0) > 0 && GHealthPrediction->GetPredictedHealth(lastMinion, kLastHitPrediction, 0.25, 0.25) <= GDamage->GetSpellDamage(GEntityList->Player(), lastMinion, kSlotQ))
                    {
                        Q->CastOnPosition(pos1);
                        break;
                    }
                }
            }
        }
    }
}


void Karthus::Drawing()
{
    if(DrawReady->Enabled())
    {
        if(Q->IsReady() && DrawQ->Enabled())
        {
            GRender->DrawCircle(Hero->GetPosition(), Q->Range(), Vec4(0, 255, 0, 225));
        }
        if(W->IsReady() && DrawW->Enabled())
        {
            GRender->DrawCircle(Hero->GetPosition(), W->Range(), Vec4(0, 255, 0, 225));
        }
        if(E->IsReady() && DrawE->Enabled())
        {
            GRender->DrawCircle(Hero->GetPosition(), E->Range(), Vec4(0, 255, 0, 225));
        }
    }
    else
    {
        if(DrawQ->Enabled())
        {
            GRender->DrawCircle(Hero->GetPosition(), Q->Range(), Vec4(0, 255, 0, 225));
        }
        if(DrawW->Enabled())
        {
            GRender->DrawCircle(Hero->GetPosition(), W->Range(), Vec4(0, 255, 0, 225));
        }
        if(DrawE->Enabled())
        {
            GRender->DrawCircle(Hero->GetPosition(), E->Range(), Vec4(0, 255, 0, 225));
        }
    }
}