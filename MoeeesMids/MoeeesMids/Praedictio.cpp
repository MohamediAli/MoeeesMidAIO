#include "Praedictio.h"

AdvPredictionInput PredictionInput;
AdvPredictionOutput PredictionOutput;

void Praedictio::SetInput(IUnit* Source, ISpell2* Spell, IUnit* Target, Vec3 ballPos)
{
    PredictionInput.AddBoundingRadius = true;
    PredictionInput.RangeCheckFromPosition = Spell->GetRangeCheckFrom();
    PredictionInput.IsAoE = Spell->CastOnTargetAoE(Target, 1, kHitChanceMedium);
    Vec3 toTargetDirection;
    if(ballPos != Vec3(0,0,0))
    {
        toTargetDirection = (Target->ServerPosition() - ballPos).VectorNormalize();
        PredictionInput.FromPosition = ballPos - toTargetDirection * 65;
    }
    else
    {
        toTargetDirection = (Target->ServerPosition() - Source->ServerPosition()).VectorNormalize();
        PredictionInput.FromPosition = Source->ServerPosition() - toTargetDirection * Source->BoundingRadius();
    }
    PredictionInput.Type = Spell->GetType();
    PredictionInput.Delay = Spell->GetDelay();
    PredictionInput.Radius = Spell->Radius();
    PredictionInput.Speed = Spell->Speed() == FLT_MAX ? 3000 : Spell->Speed();
    PredictionInput.Range = Spell->Range();
    PredictionInput.CollisionFlags = Spell->GetCollisionFlags();
    PredictionInput.Target = Target;
}


float Praedictio::GetImpactTime(Vec3 TargetDirection)
{
    auto targetPosition = PredictionInput.Target->ServerPosition();
    auto delays = PredictionInput.Delay + GGame->Latency() / 1000;
    targetPosition = targetPosition + TargetDirection * (PredictionInput.Target->MovementSpeed() * delays);
    targetPosition = targetPosition - TargetDirection * PredictionInput.Target->BoundingRadius();
    auto toTargetDirection = (targetPosition - PredictionInput.FromPosition).VectorNormalize();
    auto theta = TargetDirection * toTargetDirection;
    auto castDirection = TargetDirection + toTargetDirection;
    targetPosition = targetPosition - castDirection * (theta * PredictionInput.Target->BoundingRadius());
    auto unitDistance = (targetPosition - PredictionInput.FromPosition).Length2D();
    auto a = pow(PredictionInput.Target->MovementSpeed(), 2) - pow(PredictionInput.Speed, 2);
    auto b = 2 * PredictionInput.Target->MovementSpeed() * unitDistance * theta;
    auto c = pow(unitDistance, 2);
    auto discriminant = b * b - 4 * a * c;
    if(discriminant < 0)
    {
        return -1;
    }
    auto impactTime = 2 * c / (sqrt(discriminant) - b);
    if(impactTime < 0)
    {
        return -1;
    }
    return impactTime;
}

void Praedictio::SetOutput()
{
    auto targetPosition = PredictionInput.Target->ServerPosition();
    auto paths = PredictionInput.Target->GetWaypointList();
    if(PredictionInput.Target->IsMoving() && paths.size() > 1)
    {
        for(size_t i = 1; i <= paths.size(); i++)
        {
            auto path = paths[i];
            auto targetDirection = (path - paths[i - 1]).VectorNormalize();
            auto impactTime = GetImpactTime(targetDirection);
            if(impactTime == -1)
            {
                return;
            }
            if((path - targetPosition).Length2D() / PredictionInput.Target->MovementSpeed() < impactTime)
            {
                targetPosition = path;
                if(i != paths.size())
                {
                    continue;
                }
            }
            else
            {
                targetPosition = targetPosition + targetDirection * (PredictionInput.Target->MovementSpeed() * impactTime);
            }
            PredictionOutput.TargetPosition = targetPosition;
            PredictionOutput.CastPosition = targetPosition;
        }
    }
    else
    {
        PredictionOutput.TargetPosition = PredictionInput.Target->ServerPosition();
        PredictionOutput.CastPosition = PredictionInput.Target->ServerPosition();
    }
    PredictionOutput.HitChance = kHitChanceLow;
    PredictionOutput.AoETargetsHit = { PredictionInput.Target };
}

void Praedictio::Cast(IUnit* Source, ISpell2* Spell, IUnit* Target, Vec3 ballPos)
{
    SetInput(Source, Spell, Target, ballPos);
    SetOutput();
    if(ballPos != Vec3(0,0,0))
    {
        if((PredictionOutput.TargetPosition - GEntityList->Player()->ServerPosition()).Length2D() > PredictionInput.Range)
        {
            return;
        }
    }
    else
    {
        if((PredictionOutput.TargetPosition - PredictionInput.FromPosition).Length2D() > PredictionInput.Range)
        {
            return;
        }
    }
    AdvPredictionOutput AdvPredictionOutput;
    Spell->RunPrediction(Target, false, Spell->GetCollisionFlags(), &AdvPredictionOutput);
    if(AdvPredictionOutput.HitChance >= kHitChanceMedium && AdvPredictionOutput.HitChance != kHitChanceCollision)
    {
        Spell->CastOnPosition(PredictionOutput.CastPosition);
    }
}