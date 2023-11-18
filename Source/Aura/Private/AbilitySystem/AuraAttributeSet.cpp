// yyyyyxxxxxx


#include "AbilitySystem/AuraAttributeSet.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AuraGameplayTags.h"
#include "GameplayEffectExtension.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "GameFramework/Character.h"
#include "Interacton/CombatInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Player/AuraPlayerController.h"

UAuraAttributeSet::UAuraAttributeSet()
{
	const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();

	TagsToAttributes.Add(GameplayTags.Attributes_Primary_Strength, UAuraAttributeSet::GetStrengthAttribute);
	// 智力
	TagsToAttributes.Add(GameplayTags.Attributes_Primary_Intelligence, UAuraAttributeSet::GetIntelligenceAttribute);;
	// 韧性
	TagsToAttributes.Add(GameplayTags.Attributes_Primary_Resilience, UAuraAttributeSet::GetResilienceAttribute);;
	// 活力
	TagsToAttributes.Add(GameplayTags.Attributes_Primary_Vigor, UAuraAttributeSet::GetVigorAttribute);;
	// 护甲
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_Armor, UAuraAttributeSet::GetArmorAttribute);;
	// 护甲穿透
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_ArmorPenetration, UAuraAttributeSet::GetArmorPenetrationAttribute);;
	// 格挡
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_BlockChance, UAuraAttributeSet::GetBlockChanceAttribute);;
	// 暴击率
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_CriticalHitChance, UAuraAttributeSet::GetCriticalHitChanceAttribute);;
	// 暴击伤害
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_CriticalHitDamage, UAuraAttributeSet::GetCriticalHitDamageAttribute);;
	// 暴击抗性
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_CriticalHitResistance, UAuraAttributeSet::GetCriticalHitResistanceAttribute);;
	// 生命恢复
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_HealthRegeneration, UAuraAttributeSet::GetHealthRegenerationAttribute);;
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_ManaRegeneration, UAuraAttributeSet::GetManaRegenerationAttribute);;
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_MaxHealth, UAuraAttributeSet::GetMaxHealthAttribute);;
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_MaxMana, UAuraAttributeSet::GetMaxManaAttribute);;


	// 伤害抗性
	TagsToAttributes.Add(GameplayTags.Attributes_Resistance_Fire, UAuraAttributeSet::GetFireResistanceAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Resistance_Lightning, UAuraAttributeSet::GetLightningResistanceAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Resistance_Arcane, UAuraAttributeSet::GetArcaneResistanceAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Resistance_Physical, UAuraAttributeSet::GetPhysicalResistanceAttribute);
	
}


//1 属性变化前（PreAttributeChange）主要是响应Attribute中的CurrentValue的修改发生之前的调用。
//2 这里最好就是去做一些对输入的限制和调整，限制到某个合理的区间范围。
/*
 *注意Epic的对PreAttributeChange()的注释提到，不要去使用它来处理游玩相关的事件，而只是把它用作数值的修正和处理。
 *监听Attribute的变化而产生的和游玩相关的事件（译者注：比如说生命值、弹药数等属性的UI响应事件）
 *的推荐的处理方案是使用
 *UAbilitySystemComponent::GetGameplayAttributeValueChangeDelegate(FGameplayAttribute Attribute)
 */
void UAuraAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
	
	if (Attribute == GetManaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxMana());
	}
}
/*
 * 1 GameplayEffect对某个Attribute的BaseValue!!修改之后!!才会触发。
 * 2 当postgameplayeffectexecute调用的时候，只发生在服务器上。
 * 3 示例项目也在这个位置来应用受击动画，显示伤害飘字，并且为击杀者赋予经验和金币奖励。
 */
void UAuraAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	FEffectProperties Props;
	SetEffectProperties(Data, Props);

	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
		UE_LOG(LogTemp, Warning, TEXT(" Changed Health on %s, Health: %f "), *Props.TargetAvatarActor->GetName(), GetHealth());
	}
	if (Data.EvaluatedData.Attribute == GetManaAttribute())
	{
		SetMana(FMath::Clamp(GetMana(), 0.f, GetMaxMana()));
	}

	
	if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
	{
		// 套出来，然后重置0
		const float LocalIncomingDamage = GetIncomingDamage();
		SetIncomingDamage(0.f);

		if (LocalIncomingDamage > 0.f)
		{
			// 牛逼的意义--->
			const float NewHealth = GetHealth() - LocalIncomingDamage;
			SetHealth(FMath::Clamp(NewHealth, 0.f, GetMaxHealth()));
			// 意义是，当 NewHealth < 0，那就表示这是一哥致命伤害！
			// Fatal=致命的
			const bool bFatal = NewHealth <= 0.f;

			// 致命
			if (bFatal)
			{
				ICombatInterface* CombatInterface = Cast<ICombatInterface>(Props.TargetAvatarActor);
				if (CombatInterface)
				{
					CombatInterface->Die();
				}
				
			} else // 不致命，受击GA触发
			{
				FGameplayTagContainer TagContainer;
				TagContainer.AddTag(FAuraGameplayTags::Get().Effects_HitReact);
				Props.SourceASC; // 操，这个是 玩家(Name = "AbilitySystemComponent", Owner = 0x00000af5457bc000 (Name="BP_AuraPlayerState_C"_0))
				Props.TargetASC; // 操，这个是 哥布林
				Props.TargetASC->TryActivateAbilitiesByTag(TagContainer);
			}

			// 获取GEContext的自定义属性
			const bool bBlock = UAuraAbilitySystemLibrary::IsBlockedHit(Props.EffectContextHandle);
			const bool bCritical = UAuraAbilitySystemLibrary::IsCriticalHit(Props.EffectContextHandle);
			ShowFloatingText(Props, LocalIncomingDamage, bBlock, bCritical);
		}
	}
}

void UAuraAttributeSet::ShowFloatingText(const FEffectProperties& Props, float Damage, bool bBlockedHit, bool bCriticalHit) const
{
	// 拒绝自我伤害
	if (Props.SourceCharacter != Props.TargetCharacter)
	{
		// 获取PlayControll 哥布林身上肯定没有，只有玩家身上才有PlayerControl,那只能 是施法者玩家的
		// 错误的网络复制000--情况 APlayerController * PlayerController = UGameplayStatics::GetPlayerController(Props.SourceCharacter, 0);
		// Props.SourceCharacter->Controller = 发射火球的人
		if (AAuraPlayerController* PC = Cast<AAuraPlayerController>(Props.SourceCharacter->Controller))
		{
			// 哥布林身上创建 飘字组件
			PC->ShowDamageNumber(Damage, Props.TargetCharacter, bBlockedHit, bCriticalHit);
			return;
		}
		// 上面描述不准确，这里是服务器走的位置，那PlayerControll是都存在的，所以敌人也有PlayerController
		// 这里就是 哥布林对玩家造成伤害的 飘字
		if (AAuraPlayerController* PC = Cast<AAuraPlayerController>(Props.TargetCharacter->Controller))
		{
			// 哥布林身上创建 飘字组件
			PC->ShowDamageNumber(Damage, Props.TargetCharacter, bBlockedHit, bCriticalHit);
		}
	}
}


void UAuraAttributeSet::SetEffectProperties(const FGameplayEffectModCallbackData& Data, FEffectProperties& Props) const
{
	{ // Source
		Props.EffectContextHandle = Data.EffectSpec.GetContext();
		
		// Instigator 触发者  PlayState
		Props.SourceASC = Props.EffectContextHandle.GetOriginalInstigatorAbilitySystemComponent();

		if (IsValid(Props.SourceASC) && Props.SourceASC->AbilityActorInfo.IsValid() && Props.SourceASC->AbilityActorInfo->AvatarActor.IsValid())
		{
			Props.SourceAvatarActor = Props.SourceASC->AbilityActorInfo->AvatarActor.Get();
			Props.SourceController = Props.SourceASC->AbilityActorInfo->PlayerController.Get();
			
			if (Props.SourceController == nullptr && Props.SourceAvatarActor != nullptr)
			{
				if (const APawn* Pawn = Cast<APawn>(Props.SourceAvatarActor))
				{
					Props.SourceController = Pawn->GetController();
				}
			}
			// 获取Pawn
			if (Props.SourceController)
			{
				Props.SourceCharacter = Cast<ACharacter>(Props.SourceController->GetPawn());
			}
		}
	}

	{ // Target
		if (Data.Target.AbilityActorInfo.IsValid() && Data.Target.AbilityActorInfo->AvatarActor.IsValid())
		{
			Props.TargetAvatarActor = Data.Target.AbilityActorInfo->AvatarActor.Get();
			Props.TargetController = Data.Target.AbilityActorInfo->PlayerController.Get();
			Props.TargetCharacter = Cast<ACharacter>(Props.TargetAvatarActor);
			Props.TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Props.TargetAvatarActor);
		}
	}
}

void UAuraAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth) const
{
	// 它主要用于在属性变化时通知相关的客户端进行同步
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Health, OldHealth);
}

void UAuraAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const
{
	// 它主要用于在属性变化时通知相关的客户端进行同步
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, MaxHealth, OldMaxHealth);
}

void UAuraAttributeSet::OnRep_Mana(const FGameplayAttributeData& OldMana) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Mana, OldMana);
}

void UAuraAttributeSet::OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, MaxMana, OldMaxMana);
}

void UAuraAttributeSet::OnRep_Armor(const FGameplayAttributeData& OldArmor) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Armor, OldArmor);
}

void UAuraAttributeSet::OnRep_ArmorPenetration(const FGameplayAttributeData& OldArmorPenetration) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, ArmorPenetration, OldArmorPenetration);
}

void UAuraAttributeSet::OnRep_BlockChance(const FGameplayAttributeData& OldBlockChance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, BlockChance, OldBlockChance);
}

void UAuraAttributeSet::OnRep_CriticalHitChance(const FGameplayAttributeData& OldCriticalHitChance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, CriticalHitChance, OldCriticalHitChance);
}

void UAuraAttributeSet::OnRep_CriticalHitDamage(const FGameplayAttributeData& OldCriticalHitDamage) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, CriticalHitDamage, OldCriticalHitDamage);
}

void UAuraAttributeSet::OnRep_CriticalHitResistance(const FGameplayAttributeData& OldCriticalHitResistance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, CriticalHitResistance, OldCriticalHitResistance);
}

void UAuraAttributeSet::OnRep_HealthRegeneration(const FGameplayAttributeData& OldHealthRegeneration) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, HealthRegeneration, OldHealthRegeneration);
}

void UAuraAttributeSet::OnRep_ManaRegeneration(const FGameplayAttributeData& OldManaRegeneration) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, ManaRegeneration, OldManaRegeneration);
}

void UAuraAttributeSet::OnRep_FireResistance(const FGameplayAttributeData& OldFireResistance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, FireResistance, OldFireResistance);
}

void UAuraAttributeSet::OnRep_LightningResistance(const FGameplayAttributeData& OldLightningResistance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, LightningResistance, OldLightningResistance);
}

void UAuraAttributeSet::OnRep_ArcaneResistance(const FGameplayAttributeData& OldArcaneResistance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, ArcaneResistance, OldArcaneResistance);
}

void UAuraAttributeSet::OnRep_PhysicalResistance(const FGameplayAttributeData& OldPhysicalResistance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, PhysicalResistance, OldPhysicalResistance);
}


void UAuraAttributeSet::OnRep_Strength(const FGameplayAttributeData& OldStrength) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Strength, OldStrength);
}

void UAuraAttributeSet::OnRep_Intelligence(const FGameplayAttributeData& OldIntelligence) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Intelligence, OldIntelligence);
}

void UAuraAttributeSet::OnRep_Resilience(const FGameplayAttributeData& OldResilience) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Resilience, OldResilience);
}

void UAuraAttributeSet::OnRep_Vigor(const FGameplayAttributeData& OldVigor) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Vigor, OldVigor);
}


void UAuraAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// COND_None 表示不需要任何条件 就可网络复制
	// REPNOTIFY_Always 表示任何下都进行通知，哪怕值变化前后具体值都没有改变  通知策略
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Mana, COND_None, REPNOTIFY_Always);
	
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, MaxMana, COND_None, REPNOTIFY_Always);
	
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Strength, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Intelligence, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Resilience, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Vigor, COND_None, REPNOTIFY_Always);

	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, FireResistance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, LightningResistance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, ArcaneResistance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, PhysicalResistance, COND_None, REPNOTIFY_Always);
	
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, ArmorPenetration, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, BlockChance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, CriticalHitChance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, CriticalHitDamage, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, CriticalHitResistance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, HealthRegeneration, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, ManaRegeneration, COND_None, REPNOTIFY_Always);
	
}
