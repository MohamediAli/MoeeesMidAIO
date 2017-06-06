#include "Taliyah.h"
#include "Rembrandt.h"
#include "Extensions.h"

Taliyah::~Taliyah()
{
    TaliyahMenu->Remove();
}


Taliyah::Taliyah(IMenu* Parent, IUnit* Hero) :Champion(Parent, Hero)
{
    Q = GPluginSDK->CreateSpell2(kSlotQ, kLineCast, true, true, static_cast<eCollisionFlags>(kCollidesWithYasuoWall, kCollidesWithMinions));
    Q->SetSkillshot(0.275f, 100.f, 3600.f, 1000.f);
    W = GPluginSDK->CreateSpell2(kSlotW, kCircleCast, false, true, kCollidesWithNothing);
    W->SetSkillshot(1.0f, 200.f, FLT_MAX, 900.f);
    E = GPluginSDK->CreateSpell2(kSlotE, kConeCast, false, true, kCollidesWithWalls);
    E->SetSkillshot(0.30f, 450.f, FLT_MAX, 800.f);
    R = GPluginSDK->CreateSpell2(kSlotR, kTargetCast, false, false, kCollidesWithNothing);
    TaliyahMenu      = Parent->AddMenu("Taliyah Menu");
    ComboMenu        = Parent->AddMenu("Combo");
    qMenu            = Parent->AddMenu("Q Settings");
    wMenu            = Parent->AddMenu("W Settings");
    eMenu            = Parent->AddMenu("E Settings");
    Drawings         = Parent->AddMenu("Spell Drawings");
    MiscMenu         = Parent->AddMenu("Miscs");
    LockQ            = qMenu->CheckBox("Auto-Lock Q", true);
    killStealQ       = qMenu->CheckBox("Kill Steal with Q", true);
    ComboQ           = qMenu->CheckBox("Use Q in Combo", true);
    comboFullQ       = qMenu->CheckBox(":: Combo: Only with full Q", true);
    harassQ          = qMenu->CheckBox("Harass with Q", true);
    harassFullQ      = qMenu->CheckBox(":: Harass: Only with full Q", false);
    harassQMana      = qMenu->AddFloat(":: Only Harras Q if Mana >", 0, 100, 50);
    laneClearQ       = qMenu->CheckBox("Wave Clear with Q", true);
    laneFullQ        = qMenu->CheckBox(":: Wave Clear: Only with full Q", true);
    laneClearQMana   = qMenu->AddFloat(":: Only Wave Clear Q if Mana >", 0, 100, 70);
    JungleClearQ     = qMenu->CheckBox("Jungle Clear with Q", true);
    jungleFullQ      = qMenu->CheckBox(":: Jungle Clear: Only with full Q", true);
    JungleClearQMana = qMenu->AddFloat(":: Only Jungle Clear Q if Mana >", 0, 100, 50);
    ComboW           = wMenu->CheckBox("Use W in Combo", true);
    EonlyW           = wMenu->CheckBox("::Only W if E is Up", true);
    killStealW       = wMenu->CheckBox("Kill Steal with W", true);
    laneClearW       = wMenu->CheckBox("Wave Clear with W", true);
    laneClearWMana   = wMenu->AddFloat(":: Only Wave Clear W if Mana >", 0, 100, 70);
    JungleClearW     = wMenu->CheckBox("Jungle Clear with W", true);
    JungleClearWMana = wMenu->AddFloat(":: Only Jungle Clear W if Mana >", 0, 100, 50);
    seperator1       = wMenu->CheckBox("Seismic Shove Options:", false);
    gapCloserW       = wMenu->CheckBox("W on Gap Closer", true);
    interrupterW     = wMenu->CheckBox("W on Interruptable Spells", true);
    ComboE           = eMenu->CheckBox("Use E in Combo", true);
    killStealE       = eMenu->CheckBox("Kill Steal with E", true);
    harassE          = eMenu->CheckBox("Harass with E", true);
    harassEMana      = eMenu->AddFloat(":: Only Harras E if Mana >", 0, 100, 50);
    laneClearE       = eMenu->CheckBox("Wave Clear with E", true);
    laneClearEMana   = eMenu->AddFloat(":: Only Wave Clear E if Mana >", 0, 100, 70);
    JungleClearE     = eMenu->CheckBox("Jungle Clear with E", true);
    JungleClearEMana = eMenu->AddFloat(":: Only Jungle Clear E if Mana >", 0, 100, 50);
    gapCloserE       = eMenu->CheckBox("E on Gap Closer", true);
    RideR            = MiscMenu->CheckBox("Automatically Mount on R", true);
    DrawReady        = Drawings->CheckBox("Draw Ready Spells", true);
    drawDmg          = Drawings->CheckBox("Draw Damage", false);
    HPBar            = Drawings->AddColor("Change Health Bar", 125, 0, 250, 200);
    DrawQ            = Drawings->CheckBox("Draw Q", true);
    DrawW            = Drawings->CheckBox("Draw W", true);
    DrawE            = Drawings->CheckBox("Draw E", true);
    DrawR            = Drawings->CheckBox("Draw R", true);
}


void Taliyah::OnGameUpdate()
{
    auto walkPos = CalculateReturnPos();
    QTarget = GTargetSelector->FindTarget(QuickestKill, SpellDamage, W->Range());
    if(!GUtility->IsLeagueWindowFocused() || GGame->IsChatOpen() || GGame->IsShopOpen())
    {
        return;
    }
    KillSteal();
    Automatic();
    if(GOrbwalking->GetOrbwalkingMode() == kModeCombo)
    {
        Combo();
        if(Extensions::GetDistance(Hero, walkPos) < 500 && LockQ->Enabled() && !GNavMesh->IsPointWall(walkPos) && Extensions::GetDistance(Hero, walkPos) > 30)
        {
            GGame->IssueOrder(Hero, kMoveTo, walkPos);
        }
    }
    else if(GOrbwalking->GetOrbwalkingMode() == kModeMixed)
    {
        Harass();
        if(Extensions::GetDistance(Hero, walkPos) < 500 && LockQ->Enabled() && !GNavMesh->IsPointWall(walkPos) && Extensions::GetDistance(Hero, walkPos) > 30)
        {
            GGame->IssueOrder(Hero, kMoveTo, walkPos);
        }
    }
    else if(GOrbwalking->GetOrbwalkingMode() == kModeLaneClear)
    {
        LaneClear();
        JungleClear();
    }
}
void Taliyah::OnRender()
{
    if(Extensions::Validate(Qing) && LockQ->Enabled())
    {
        Extensions::DrawLineRectangle(Hero->GetPosition(), GMissileData->GetEndPosition(Qing), Q->Radius(), laneClearWMana->GetFloat(), Vec4(255, 255, 100, 255));
    }
    Drawing();
    dmgdraw();
    GRender->DrawCircle(Hero->GetPosition(), 30, Vec4(255, 255, 0, 255), 2);
    if(CalculateReturnPos() != Vec3(0, 0, 0))
    {
        GRender->DrawCircle(CalculateReturnPos(), 100, Vec4(255, 0, 255, 255), 5, false);
    }
}



Vec3 Taliyah::CalculateReturnPos() //credits 2 my nigga sebby
{
    auto target = GTargetSelector->FindTarget(QuickestKill, SpellDamage, W->Range());
    if(Extensions::Validate(Qing) && Extensions::Validate(target))
    {
        auto finishPosition = GMissileData->GetEndPosition(Qing);
        /*if (GMissileData->GetName (missileSource) == MissileName)
        {
        finishPosition = MissileEndPos;
        }*/
        auto misToPlayer = Extensions::GetDistance(Hero, finishPosition);
        auto tarToPlayer = Extensions::GetDistance(Hero, target);
        if(misToPlayer > tarToPlayer)
        {
            auto misToTarget = Extensions::GetDistance(target, finishPosition);
            if(misToTarget < Q->Range() && misToTarget > 50)
            {
                auto cursorToTarget = Extensions::GetDistance(target, (Hero->ServerPosition().Extend(GGame->CursorPosition(), 100)));
                auto ext = finishPosition.Extend(target->ServerPosition(), cursorToTarget + misToTarget);
                if(Extensions::GetDistance(ext, Hero->ServerPosition()) < 1000 && Extensions::EnemiesInRange(ext, 400) <2)    // CountEnemiesInRange (400) < 2)
                {
                    return ext;
                }
            }
        }
    }
    return Vec3(0, 0, 0);
}



void Taliyah::zigzag()   //credits sn karthus
{
    if(QTarget == nullptr || !QTarget->IsHero() || !QTarget->IsValidTarget())
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

Vec3 Taliyah::PredPos(IUnit* Hero, float Delay)   //credits sn karthus
{
    float value = 0.f;
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

void  Taliyah::AntiGapclose(GapCloserSpell const& args)
{
    if(E->IsReady() && gapCloserE->Enabled() && Extensions::GetDistance(GEntityList->Player(), args.Source) <150 && args.Source->IsValidTarget() && args.Source->IsEnemy(Hero))
    {
        E->CastOnPosition(args.Source->ServerPosition());
    }
    if(W->IsReady() && gapCloserW->Enabled() && Extensions::GetDistance(GEntityList->Player(), args.Source) <150 && args.Source->IsEnemy(Hero))
    {
        AdvPredictionOutput outputfam;
        W->RunPrediction(args.Source, false, kCollidesWithMinions, &outputfam);
        if(outputfam.HitChance == kHitChanceDashing)
        {
            W->CastOnTarget(args.Source, kHitChanceDashing);
        }
    }
}

void Taliyah::OnInterrupt(InterruptibleSpell const& args)
{
    if(args.Source == nullptr || args.Source->IsDead()) { return; }
    if(W->IsReady() && interrupterW->Enabled() && Hero->IsValidTarget(args.Source, W->Range()) && args.Source != nullptr && args.Source != Hero && args.Source->IsEnemy(Hero))
    {
        if(!cz)
        {
            W->CastFrom(PredPos(QTarget, 0.75f + (GGame->Latency() / 1000) / 2), Hero->ServerPosition());
        }
    }
}

void Taliyah::OnCreate(IUnit* object)
{
    auto objectName = object->GetObjectName();
    if(object->IsMissile() && GMissileData->GetCaster(object) == GEntityList->Player())
    {
        if(strcmp(GMissileData->GetName(object), "TaliyahQMis") == 0)
        {
            Qing = object;
        }
    }
    std::vector<IUnit*> result;
    for(auto qaoe : GEntityList->GetAllUnits())
    {
        if(strcmp(objectName, "Taliyah_Base_Q_aoe_bright.troy") == 0 && object->IsVisible() && object->IsValidObject())
        {
            qFive = false;
            result.push_back(qaoe);
        }
    }
    std::vector<IUnit*> result1;
    for(auto qaoe1 : GEntityList->GetAllUnits())
    {
        if(strcmp(objectName, "Taliyah_Base_E_Mines.troy") == 0 && object->IsVisible() && object->IsValidObject())
        {
            eOnGround = true;
            result1.push_back(qaoe1);
        }
    }
}

void Taliyah::OnDelete(IUnit* object)
{
    auto objectName = object->GetObjectName();
    {
        if(strcmp(objectName, "Taliyah_Base_Q_aoe_bright.troy") == 0 && object->IsVisible() && object->IsValidObject())   //!soldier->IsEnemy(Hero)
        {
            qFive = true;
            /*	GPluginSDK->DelayFunctionCall(300,
            []() { for (auto ground : GEntityList->GetAllUnits())
            if ((Hero->ServerPosition() - ground->ServerPosition()).Length() < 412.5f && strcmp(ground->GetObjectName(), "Taliyah_Base_Q_aoe_bright.troy") == 0 && ground->IsValidObject())

            }); */
        }
    }
    if(strcmp(objectName, "Taliyah_Base_E_Timeout.troy") == 0 && object->IsVisible() && object->IsValidObject())   //!soldier->IsEnemy(Hero)
    {
        eOnGround = false;
    }
}

void Taliyah::CastE()
{
    auto target = GTargetSelector->GetFocusedTarget() != nullptr
                  ? GTargetSelector->GetFocusedTarget()
                  : GTargetSelector->FindTarget(QuickestKill, SpellDamage, W->Range() + 50);
    if(target == nullptr || !target->IsHero())
    {
        return;
    }
    if(ComboE->Enabled() && Extensions::GetDistanceSqr(Hero->ServerPosition(),target->ServerPosition()) < (E->Range()+100) * (E->Range() + 100))
    {
        E->CastOnPosition(target->ServerPosition());
    }
}

void Taliyah::Combo()
{
    auto target = GTargetSelector->FindTarget(QuickestKill, SpellDamage, W->Range());
    if(target == nullptr || !target->IsHero() || !Extensions::Validate(target) || !target->IsVisible())
    {
        return;
    }
    if(ComboW->Enabled() && W->IsReady() && Hero->IsValidTarget(target, W->Range()))
    {
        if(EonlyW->Enabled())
        {
            if(E->IsReady() && (Hero->ServerPosition() - target->ServerPosition()).Length() > 420)
            {
                if(!cz)
                {
                    W->CastFrom(PredPos(QTarget, 0.75f + (GGame->Latency() / 1000) / 2), Hero->ServerPosition());
                }
            }
            if(E->IsReady() && W->IsReady() && Extensions::GetDistance(Hero, target) < 420)
            {
                if(!cz)
                {
                    W->CastOnPosition(PredPos(QTarget, 0.75f + (GGame->Latency() / 1000) / 2));
                }
            }
        }
        if(eOnGround)
        {
            if((Hero->ServerPosition() - target->ServerPosition()).Length() > 420)
            {
                if(!cz)
                {
                    W->CastFrom(PredPos(QTarget, 0.75f + (GGame->Latency() / 1000) / 2), Hero->ServerPosition());
                }
            }
            if((Hero->ServerPosition() - target->ServerPosition()).Length() < 420)
            {
                if(!cz)
                {
                    W->CastOnPosition(PredPos(QTarget, 0.75f + (GGame->Latency() / 1000) / 2));
                }
            }
        }
        if(!EonlyW->Enabled())
        {
            if(!cz)
            {
                W->CastFrom(PredPos(QTarget, 0.75f + (GGame->Latency() / 1000) / 2), Hero->ServerPosition());
            }
        }
    }
    if(ComboE->Enabled() && E->IsReady() && !W->IsReady() && Hero->IsValidTarget(target, E->Range()))
    {
        //GPluginSDK->DelayFunctionCall(400, []() { CastE(); });
        CastE();
    }
    if(ComboQ->Enabled() && Q->IsReady() && Hero->IsValidTarget(target, Q->Range() + 50))
    {
        if(comboFullQ->Enabled() && qFive == true)
        {
            Q->CastOnTarget(target, kHitChanceMedium);
        }
        if(!comboFullQ->Enabled())
        {
            Q->CastOnTarget(target, kHitChanceMedium);
        }
    }
}





void Taliyah::Harass()
{
    auto target = GTargetSelector->FindTarget(QuickestKill, SpellDamage, W->Range());
    if(harassE->Enabled() && E->IsReady() && Hero->IsValidTarget(target, E->Range()) && Hero->ManaPercent() >= harassEMana->GetFloat())
    {
        E->CastOnTarget(target);
    }
    if(harassQ->Enabled() && Q->IsReady() && Hero->IsValidTarget(target, Q->Range() + 50) && Hero->ManaPercent() >= harassQMana->GetFloat())
    {
        if(harassFullQ->Enabled() && qFive == true)
        {
            Q->CastOnTarget(target, kHitChanceHigh);
        }
        if(!harassFullQ->Enabled())
        {
            Q->CastOnTarget(target, kHitChanceHigh);
        }
    }
}

void Taliyah::LaneClear()
{
    for(auto minions : GEntityList->GetAllMinions(false, true, false))
    {
        if(laneClearQ->Enabled() && Q->IsReady() && Hero->ManaPercent() > laneClearQMana->GetFloat() && Hero->IsValidTarget(minions, W->Range()))
        {
            if(laneFullQ->Enabled() && qFive == true)
            {
                Q->CastOnPosition(minions->ServerPosition());
            }
            if(!laneFullQ->Enabled())
            {
                Q->CastOnPosition(minions->ServerPosition());
            }
        }
        if(laneClearE->Enabled() && Hero->ManaPercent() > laneClearEMana->GetFloat() && E->IsReady() && Hero->IsValidTarget(minions, E->Range()))
        {
            E->CastOnUnit(minions);
        }
        if(laneClearW->Enabled() && Hero->ManaPercent() > laneClearWMana->GetFloat() && W->IsReady() && Hero->IsValidTarget(minions, W->Range()))
        {
            W->CastFrom(minions->ServerPosition(), Hero->ServerPosition());
        }
    }
}

void Taliyah::JungleClear()
{
    for(auto minions : GEntityList->GetAllMinions(false, false, true))
    {
        if(JungleClearQ->Enabled() && Q->IsReady() && Hero->ManaPercent() > JungleClearQMana->GetFloat() && Hero->IsValidTarget(minions, W->Range()))
        {
            if(jungleFullQ->Enabled() && qFive == true)
            {
                Q->CastOnPosition(minions->ServerPosition());
            }
            if(!jungleFullQ->Enabled())
            {
                Q->CastOnPosition(minions->ServerPosition());
            }
        }
        if(JungleClearE->Enabled() && Hero->ManaPercent() > JungleClearEMana->GetFloat() && E->IsReady() && Hero->IsValidTarget(minions, W->Range()))
        {
            E->CastOnUnit(minions);
        }
        if(JungleClearW->Enabled() && Hero->ManaPercent() > JungleClearWMana->GetFloat() && W->IsReady() && Hero->IsValidTarget(minions, W->Range()))
        {
            W->CastOnUnit(minions);
        }
    }
}

void Taliyah::autoE()
{
    for(auto target : GEntityList->GetAllHeros(false, true))
        if(E->IsReady() && target->IsValidTarget())
        {
            if(target->IsValidTarget(Hero, E->Range()))
            {
                AdvPredictionOutput outputfam;
                E->RunPrediction(target, false, kCollidesWithNothing, &outputfam);
                if(outputfam.HitChance == kHitChanceDashing)
                {
                    E->CastOnPosition(target->ServerPosition());
                }
            }
        }
}

void Taliyah::Automatic()
{
    for(auto target : GEntityList->GetAllHeros(false, true))
    {
        if(Hero->IsValidTarget(target, W->Range()) && target != nullptr && !target->IsDead())
        {
            AdvPredictionOutput prediction_output;
            W->RunPrediction(target, true, kCollidesWithNothing, &prediction_output);
            if(prediction_output.HitChance == kHitChanceImmobile)
            {
                W->CastFrom(prediction_output.CastPosition, Hero->ServerPosition());
            }
        }
    }
}

void Taliyah::OnSpellCast(CastedSpell const& args)
{
    auto fix = (E->IsReady()); //ape code pls ignore
    if(std::string(args.Name_) == "TaliyahR" && RideR->Enabled() && args.Caster_ == Hero)
    {
        R->CastOnPlayer();
    }
    auto target = GTargetSelector->FindTarget(QuickestKill, SpellDamage, W->Range());
    if(std::string(args.Name_) == "TaliyahWVC" && RideR->Enabled() && args.Caster_ == Hero)
    {
        if(ComboE->Enabled() && E->IsReady() && Hero->IsValidTarget(target, W->Range()))
        {
            CastE();
        }
    }
}

float Taliyah::qDmg(IUnit* Target)
{
    float InitDamage = 0;
    float BonusStackDamage = ((0.45 * Hero->TotalMagicDamage()));
    if(qFive == true)
    {
        if(Hero->GetSpellLevel(kSlotQ) == 1)
        {
            InitDamage += (70 + BonusStackDamage) * 3;
        }
        else if(Hero->GetSpellLevel(kSlotQ) == 2)
        {
            InitDamage += (95 + BonusStackDamage) * 3;
        }
        else if(Hero->GetSpellLevel(kSlotQ) == 3)
        {
            InitDamage += (120 + BonusStackDamage) * 3;
        }
        else if(Hero->GetSpellLevel(kSlotQ) == 4)
        {
            InitDamage += (145 + BonusStackDamage) * 3;
        }
        else if(Hero->GetSpellLevel(kSlotQ) == 5)
        {
            InitDamage += (170 + BonusStackDamage) * 3;
        }
    }
    if(qFive == false)
    {
        if(Hero->GetSpellLevel(kSlotQ) == 1)
        {
            InitDamage += (70 + BonusStackDamage);
        }
        else if(Hero->GetSpellLevel(kSlotQ) == 2)
        {
            InitDamage += (95 + BonusStackDamage);
        }
        else if(Hero->GetSpellLevel(kSlotQ) == 3)
        {
            InitDamage += (120 + BonusStackDamage);
        }
        else if(Hero->GetSpellLevel(kSlotQ) == 4)
        {
            InitDamage += (145 + BonusStackDamage);
        }
        else if(Hero->GetSpellLevel(kSlotQ) == 5)
        {
            InitDamage += (170 + BonusStackDamage);
        }
    }
    return GDamage->CalcMagicDamage(Hero, Target, InitDamage);
}

float Taliyah::wDmg(IUnit* Target)
{
    float InitDamage = 0;
    float BonusStackDamage = (0.4 * Hero->TotalMagicDamage());
    if(Hero->GetSpellLevel(kSlotW) == 1)
    {
        InitDamage += 60 + BonusStackDamage;
    }
    else if(Hero->GetSpellLevel(kSlotW) == 2)
    {
        InitDamage += 80 + BonusStackDamage;
    }
    else if(Hero->GetSpellLevel(kSlotW) == 3)
    {
        InitDamage += 100 + BonusStackDamage;
    }
    else if(Hero->GetSpellLevel(kSlotW) == 4)
    {
        InitDamage += 120 + BonusStackDamage;
    }
    else if(Hero->GetSpellLevel(kSlotW) == 5)
    {
        InitDamage += 140 + BonusStackDamage;
    }
    return GDamage->CalcMagicDamage(Hero, Target, InitDamage);
}

float Taliyah::eDmg(IUnit* Target)
{
    float InitDamage = 0;
    float BonusStackDamage = (0.4 * Hero->TotalMagicDamage());
    if(Hero->GetSpellLevel(kSlotE) == 1)
    {
        InitDamage += 70 + BonusStackDamage;
    }
    else if(Hero->GetSpellLevel(kSlotE) == 2)
    {
        InitDamage += 90 + BonusStackDamage;
    }
    else if(Hero->GetSpellLevel(kSlotE) == 3)
    {
        InitDamage += 110 + BonusStackDamage;
    }
    else if(Hero->GetSpellLevel(kSlotE) == 4)
    {
        InitDamage += 130 + BonusStackDamage;
    }
    else if(Hero->GetSpellLevel(kSlotE) == 5)
    {
        InitDamage += 150 + BonusStackDamage;
    }
    return GDamage->CalcMagicDamage(Hero, Target, InitDamage);
}

void Taliyah::dmgdraw()
{
    if(drawDmg->Enabled())
    {
        {
            for(auto hero : GEntityList->GetAllHeros(false, true))
            {
                Vec2 barPos = Vec2();
                if(hero->GetHPBarPosition(barPos) && !hero->IsDead())
                {
                    float QDamage = 0;
                    float WDamage = 0;
                    float EDamage = 0;
                    float RDamage = 0;
                    if(W->IsReady())
                    {
                        WDamage = wDmg(hero);
                    }
                    if(Q->IsReady())
                    {
                        QDamage = qDmg(hero) + GDamage->GetAutoAttackDamage(Hero, hero, true);
                    }
                    if(E->IsReady())
                    {
                        EDamage = eDmg(hero);
                    }
                    if(R->IsReady())
                    {
                        RDamage = GDamage->GetSpellDamage(Hero, hero, kSlotR);
                    }
                    Vec4 BarColor;
                    HPBar->GetColor(&BarColor);
                    float totalDamage = QDamage + WDamage + EDamage + RDamage;
                    Rembrandt::DrawDamageOnChampionHPBar(hero, totalDamage, BarColor);
                }
            }
        }
    }
}


void Taliyah::KillSteal()
{
    auto target = GTargetSelector->FindTarget(QuickestKill, SpellDamage, W->Range());
    if(target == nullptr || !target->IsHero())
    {
        return;
    }
    if(killStealE->Enabled() && E->IsReady() && Hero->IsValidTarget(target, E->Range()) && eDmg(target) > target->GetHealth())
    {
        E->CastOnTarget(target, kHitChanceVeryHigh);
    }
    if(killStealQ->Enabled() && Q->IsReady() && Hero->IsValidTarget(target, Q->Range()) && qDmg(target) > target->GetHealth())
    {
        Q->CastOnTarget(target, kHitChanceVeryHigh);
    }
    if(killStealW->Enabled() && W->IsReady() && Hero->IsValidTarget(target, W->Range()) && wDmg(target) > target->GetHealth())
    {
        W->CastOnTarget(target, kHitChanceHigh);
    }
}

void Taliyah::Drawing()
{
    if(DrawReady->Enabled())
    {
        if(Q->IsReady() && DrawQ->Enabled())
        {
            GRender->DrawCircle(Hero->GetPosition(), Q->Range(), Vec4(0, 225, 0, 225));
        }
        if(W->IsReady() && DrawW->Enabled())
        {
            GRender->DrawCircle(Hero->GetPosition(), W->Range(), Vec4(0, 225, 0, 225));
        }
        if(E->IsReady() && DrawE->Enabled())
        {
            GRender->DrawCircle(Hero->GetPosition(), E->Range(), Vec4(0, 225, 0, 225));
        }
    }
    else
    {
        if(DrawQ->Enabled())
        {
            GRender->DrawCircle(Hero->GetPosition(), Q->Range(), Vec4(0, 225, 0, 225));
        }
        if(DrawW->Enabled())
        {
            GRender->DrawCircle(Hero->GetPosition(), W->Range(), Vec4(0, 225, 0, 225));
        }
        if(DrawE->Enabled())
        {
            GRender->DrawCircle(Hero->GetPosition(), E->Range(), Vec4(0, 225, 0, 225));
        }
    }
}