#pragma once

#include "PluginSDK.h"

class Praedictio
{
        static float GetImpactTime(Vec3 TargetDirection);
        static void SetOutput();
        static void SetInput(IUnit* Source, ISpell2* Spell, IUnit* Target, Vec3 ballPos);
    public:
        static void Cast(IUnit* Source, ISpell2* Spell, IUnit* Target, Vec3 ballPos = Vec3(0,0,0));
};

