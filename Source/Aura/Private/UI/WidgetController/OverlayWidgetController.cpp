// yyyyyxxxxxx


#include "UI/WidgetController/OverlayWidgetController.h"

#include "AuraGameplayTags.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AbilitySystem/Data/AbilityInfo.h"
#include "AbilitySystem/Data/LevelUpInfo.h"
#include "Player/AuraPlayerState.h"

void UOverlayWidgetController::BroadcastInitialValues()
{
	OnHealtChanged.Broadcast(GetAuraAS()->GetHealth());
	OnMaxHealtChanged.Broadcast(GetAuraAS()->GetMaxHealth());

	OnManaChanged.Broadcast(GetAuraAS()->GetMana());
	OnMaxManaChanged.Broadcast(GetAuraAS()->GetMaxMana());
}

void UOverlayWidgetController::BindCallbackToDependencies()
{
	// 1  绑定  经验值变化
	GetAuraPS()->OnXPChangedDelegate.AddUObject(this, &UOverlayWidgetController::OnXPChanged);
	// 升级
	GetAuraPS()->OnLevelChangedDelegate.AddLambda(
		[this](int32 InNewLevel)
		{
			OnPlayerLevelChangedDelegate.Broadcast(InNewLevel);
		}
	);
	
	// 绑定建通属性变化
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		AuraAttributeSet->GetHealthAttribute()
	).AddLambda(
		[this](const FOnAttributeChangeData& Data)
		{
			OnHealtChanged.Broadcast(Data.NewValue);
		}
	);
	
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		GetAuraAS()->GetMaxHealthAttribute()
	).AddLambda(
		[this](const FOnAttributeChangeData& Data)
		{
			OnMaxHealtChanged.Broadcast(Data.NewValue);
		}
	);

	
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		GetAuraAS()->GetManaAttribute()
	).AddLambda(
		[this](const FOnAttributeChangeData& Data)
		{
			OnManaChanged.Broadcast(Data.NewValue);
		}
	);
	
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		GetAuraAS()->GetMaxManaAttribute()
	).AddLambda(
		[this](const FOnAttributeChangeData& Data)
		{
			OnMaxManaChanged.Broadcast(Data.NewValue);
		}
	);

	if (GetAuraASC())
	{
		GetAuraASC()->AbilityEquipped.AddUObject(this, &UOverlayWidgetController::OnAbilityEquipped);
		
		// 说明已经给予能力后，才进行Widget绑定
		if (GetAuraASC()->bStartupAbilitiesGiven)
		{
			BroadcastAbilityInfo();
		} else
		{
			// 说明能力 还未给予，进行Widget绑定
			//** 代理绑定 ： 就是绑定，不是给值，绑定上对应参数列表函数。激活广播传参
			GetAuraASC()->AbilitiesGivenDelegate.AddUObject(this, &UOverlayWidgetController::BroadcastAbilityInfo);
		}

		// 绑定自定义在GAS的代理
		// Lamba 是匿名函数，没有名字
		GetAuraASC()->EffectAssetTags.AddLambda(
			[this](const FGameplayTagContainer& AssetTags)
			{
				for (const FGameplayTag& Tag : AssetTags)
				{
					// 请求一个名为"Message"的`GameplayTag`的
					FGameplayTag MessageTag = FGameplayTag::RequestGameplayTag(FName("Message"));
					// "Message.HealthPotion" .MatchesTag("Message") will return true;
					// "Message" .MatchesTag("Message.HealthPotion") will return false;
					// ****** 看看Tag是不是属于Message
					if (Tag.MatchesTag(MessageTag))
					{
						const FUIWidgetRow* Row = GetDataTableRowByTag<FUIWidgetRow>(MessageWidgetDataTable, Tag);
						MessageWidgetRowDelegate.Broadcast(*Row);
					}
				}
			}
		);
	}
}

void UOverlayWidgetController::OnAbilityEquipped(const FGameplayTag& AbilityTag, const FGameplayTag& Status,
	const FGameplayTag& Slot, const FGameplayTag& PreviousSlot) const
{
	const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();
	
	FAuraAbilityInfo LastSlotInfo;
	LastSlotInfo.StatusTag = GameplayTags.Abilities_Status_UnLocked;
	LastSlotInfo.InputTag = PreviousSlot;
	LastSlotInfo.AbilityTag = GameplayTags.Abilities_None;
	// Broadcast empty info if PreviousSlot is a valid slot. Only if equipping an already-equipped spell
	AbilityInfoSignature.Broadcast(LastSlotInfo);

	FAuraAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);
	Info.StatusTag = Status;
	Info.InputTag = Slot;
	AbilityInfoSignature.Broadcast(Info);
}

void UOverlayWidgetController::OnXPChanged(int32 NewXP)
{
	// 3 广播刀UI
	ULevelUpInfo* LevelUpInfo = GetAuraPS()->LevelInfo;
	checkf(LevelUpInfo, TEXT("没找到，请指定 LevelUpInfo "));

	// 当前等级
	const int32 Level = LevelUpInfo->FindLevelForXP(NewXP);
	const int32 MaxLevel = LevelUpInfo->LevelUpInformation.Num();
	if (Level <= MaxLevel && Level > 0)
	{
		//2600
		const int32 LevelUpRequirement = LevelUpInfo->LevelUpInformation[Level].LevelUpRequirement;
		//1000
		const int32 PreviousLevelUpRequirement = LevelUpInfo->LevelUpInformation[Level - 1].LevelUpRequirement;

		//1600
		const int32 DeltaLevelRequirement = LevelUpRequirement - PreviousLevelUpRequirement;
		// 2500 - 1000 = 1500
		const int32 XPForThisLevel = NewXP - PreviousLevelUpRequirement;

		// 1500/1600
		const float XPBarPercent = static_cast<float>(XPForThisLevel) / static_cast<float>(DeltaLevelRequirement);
		OnXPPercentChangedDelegate.Broadcast(XPBarPercent);
	}
}