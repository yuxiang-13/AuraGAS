// yyyyyxxxxxx

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"


// GameplayTags的单例
struct FAuraGameplayTags
{
public:
	static const FAuraGameplayTags& Get() { return GameplayTags; };
	static void InitializeNativeGameplayTags();

	// 力量
	FGameplayTag Attributes_Primary_Strength;
	// 智力
	FGameplayTag Attributes_Primary_Intelligence;
	// 韧性
	FGameplayTag Attributes_Primary_Resilience;
	// 活力
	FGameplayTag Attributes_Primary_Vigor;
	
	
	// 护甲
	FGameplayTag Attributes_Secondary_Armor;
	// 护甲穿透
	FGameplayTag Attributes_Secondary_ArmorPenetration;
	// 格挡
	FGameplayTag Attributes_Secondary_BlockChance;
	// 暴击率
	FGameplayTag Attributes_Secondary_CriticalHitChance;
	// 暴击伤害
	FGameplayTag Attributes_Secondary_CriticalHitDamage;
	// 暴击抗性
	FGameplayTag Attributes_Secondary_CriticalHitResistance;
	// 生命恢复
	FGameplayTag Attributes_Secondary_HealthRegeneration;
	FGameplayTag Attributes_Secondary_ManaRegeneration;
	FGameplayTag Attributes_Secondary_MaxHealth;
	FGameplayTag Attributes_Secondary_MaxMana;

	// 输入
	// 鼠标左
	FGameplayTag InputTag_LMB;
	// 鼠标右
	FGameplayTag InputTag_RMB;
	// 输入按键ID（可定义任意）
	FGameplayTag InputTag_1;
	FGameplayTag InputTag_2;
	FGameplayTag InputTag_3;
	FGameplayTag InputTag_4;

	FGameplayTag Damage;
	FGameplayTag Effects_HitReact;
private:
	static FAuraGameplayTags GameplayTags;
};

