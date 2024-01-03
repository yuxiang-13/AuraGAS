// yyyyyxxxxxx


#include "AbilitySystem/Abilities/AuraGameplayAbility.h"

#include "AbilitySystem/AuraAttributeSet.h"

FString UAuraGameplayAbility::GetDescription(int32 Level)
{
	return FString::Printf(TEXT("<Default>%s, </><Level>%d</>"), L"Default Ability Name - LoremIpsum LoremIpsum LoremIpsum LoremIpsum LoremIpsum LoremIpsum LoremIpsum LoremIpsum LoremIpsum LoremIpsum LoremIpsum LoremIpsum LoremIpsum LoremIpsum LoremIpsum LoremIpsum", Level);
}

FString UAuraGameplayAbility::GetNextLevelDescription(int32 Level)
{
	return FString::Printf(TEXT("<Default>Next Level: </><Level>%d</> \n<Default>Causes much more damage. </>"), Level);
}

FString UAuraGameplayAbility::GetLockedDescription(int32 Level)
{
	return FString::Printf(TEXT("<Default>Spell Locked Until Level: %d</>"), Level);
}

float UAuraGameplayAbility::GetManaCost(float InLevel) const
{
	float ManaCost = 0.f;
	// 获取消耗GE
	if (const UGameplayEffect* CostEffect = GetCostGameplayEffect())
	{
		// 获取消耗成本
		for (FGameplayModifierInfo Mod : CostEffect->Modifiers)
		{
			// 对比属性
			if (Mod.Attribute == UAuraAttributeSet::GetManaAttribute())
			{
				// 获取具体值
				Mod.ModifierMagnitude.GetStaticMagnitudeIfPossible(InLevel, ManaCost);
				break;
			}
		}
	}

	return ManaCost;
}

// 获取冷却
float UAuraGameplayAbility::GetCooldown(float InLevel) const
{
	float Cooldown = 0.f;
	// 获取冷却GE
	if (const UGameplayEffect* CooldownEffect = GetCooldownGameplayEffect())
	{
		// 获取 持续时间修改器，获取振幅值
		CooldownEffect->DurationMagnitude.GetStaticMagnitudeIfPossible(InLevel, Cooldown);
	}
	
	return Cooldown;
}
