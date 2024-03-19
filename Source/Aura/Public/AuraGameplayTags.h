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

	
	FGameplayTag Attributes_Meta_IncomingXP; //  经验
	
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
	
	FGameplayTag InputTag_Passive_1;
	FGameplayTag InputTag_Passive_2;

	FGameplayTag Damage;
	// 火焰伤害类型
	FGameplayTag Damage_Fire; // 火
	FGameplayTag Damage_Lightning; // 雷电
	FGameplayTag Damage_Arcane; // 法术
	FGameplayTag Damage_Physical; // 物理
	
	// 抗性
	FGameplayTag Attributes_Resistance_Fire;
	FGameplayTag Attributes_Resistance_Lightning; // 雷电抗性
	FGameplayTag Attributes_Resistance_Arcane; // 法术抗性
	FGameplayTag Attributes_Resistance_Physical; // 物理抗性
	// 伤害类型+伤害抗性 Map
	TMap<FGameplayTag, FGameplayTag> DamageTpesToResistance;

	// debuff
	FGameplayTag Debuff_Burn; // 燃烧（火球GA）
	FGameplayTag Debuff_Stun; // 眩晕（雷电GA）
	FGameplayTag Debuff_Arcane; // 法术增益（法术GA）
	FGameplayTag Debuff_Physical; // 眩晕增益（物理GA）
	// 伤害类型+debuff Map
	TMap<FGameplayTag, FGameplayTag> DamageTpesToDebuffs;
	
	
	FGameplayTag Debuff_Chance; // 减益触发几率
	FGameplayTag Debuff_Damage; // 减益伤害
	FGameplayTag Debuff_Frequency; // 减益伤害 每1秒频率
	FGameplayTag Debuff_Duration; // 减益 持续时间

	FGameplayTag Abilities_None;
	FGameplayTag Abilities_Attack;
	FGameplayTag Abilities_Summon;

	
	// 技能点击状态
	FGameplayTag Abilities_HitReact;
	// 锁定
	FGameplayTag Abilities_Status_Locked;
	// 符合资格
	FGameplayTag Abilities_Status_Eligible;
	// 解锁
	FGameplayTag Abilities_Status_UnLocked;
	// 装备
	FGameplayTag Abilities_Status_Equipped;
	// 技能种类-攻击
	FGameplayTag Abilities_Type_Offensive;
	// 技能种类-被动
	FGameplayTag Abilities_Type_Passive;
	// 技能种类-无
	FGameplayTag Abilities_Type_None;

	// 光环的保护
	FGameplayTag Abilities_Passive_HaloOfProtection;
	// 吸取生命
	FGameplayTag Abilities_Passive_LifeSiphon;
	// 吸取魔法
	FGameplayTag Abilities_Passive_ManaSiphon;
	
	
	FGameplayTag Abilities_Fire_FireBolt;
	FGameplayTag Abilities_Fire_FireBlast;
	FGameplayTag Abilities_Lightning_Electrocute;
	FGameplayTag Abilities_Arcane_ArcaneShards;

	// 冷却标签
	FGameplayTag Cooldown_Fire_FireBolt;
	

	//蒙太奇动画攻击方式 武器攻击
	FGameplayTag CombatSocket_Weapon;
	FGameplayTag CombatSocket_RightHand;
	FGameplayTag CombatSocket_LeftHand;
	FGameplayTag CombatSocket_Tail;

	FGameplayTag Montage_Attack_1;
	FGameplayTag Montage_Attack_2;
	FGameplayTag Montage_Attack_3;
	FGameplayTag Montage_Attack_4;
	
	FGameplayTag Effects_HitReact;

	FGameplayTag Player_Block_InputPressed;
	FGameplayTag Player_Block_InputHeld;
	FGameplayTag Player_Block_InputReleased;
	FGameplayTag Player_Block_CursorTrace;
	
private:
	static FAuraGameplayTags GameplayTags;
};

