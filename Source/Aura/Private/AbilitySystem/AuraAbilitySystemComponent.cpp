// yyyyyxxxxxx


#include "AbilitySystem/AuraAbilitySystemComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AuraGameplayTags.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "AbilitySystem/Abilities/AuraDamageGameplayAbility.h"
#include "AbilitySystem/Abilities/AuraGameplayAbility.h"
#include "AbilitySystem/Data/AbilityInfo.h"
#include "Aura/AuraLogChannels.h"
#include "Interacton/PlayerInterface.h"


void UAuraAbilitySystemComponent::AbilityActorInfoSet()
{
	if (GetWorld()->IsServer())
	{
		// 每当GE应用于自身时在服务器上调用。这包括基于即时和持续时间的GE
		OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &UAuraAbilitySystemComponent::ClientEffectApplied);
	} else
	{
		// 客户端 绑定不绑定的，没用。因为只在服务器上的生效
	}
}

void UAuraAbilitySystemComponent::ClientEffectApplied_Implementation(UAbilitySystemComponent* AbilitySystemComponent,
                                                const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveEffectHandle)
{
	FGameplayTagContainer TagContainer;
	EffectSpec.GetAllAssetTags(TagContainer);

	EffectAssetTags.Broadcast(TagContainer);
}


void UAuraAbilitySystemComponent::AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities)
{
	// for (auto& xx : StartupAbilities)

	for (TSubclassOf<UGameplayAbility> AbilityClass : StartupAbilities)
	{
		FGameplayAbilitySpec  AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);

		// 把 激活GA的标签 添加到 动态能力Tag
		if (const UAuraGameplayAbility* AuraAbility = Cast<UAuraGameplayAbility>(AbilitySpec.Ability))
		{
			AbilitySpec.DynamicAbilityTags.AddTag(AuraAbility->StartupInputTag);
			AbilitySpec.DynamicAbilityTags.AddTag(FAuraGameplayTags::Get().Abilities_Status_Equipped);
			GiveAbility(AbilitySpec);
		}
	}

	bStartupAbilitiesGiven = true;
	// 该赋予的能力都赋予完了，广播
	AbilitiesGivenDelegate.Broadcast();
}

void UAuraAbilitySystemComponent::AddCharacterPassiveAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupPassiveAbilities)
{
	for (TSubclassOf<UGameplayAbility> AbilityClass : StartupPassiveAbilities)
	{
		FGameplayAbilitySpec  AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);

		GiveAbilityAndActivateOnce(AbilitySpec);
	}
}

bool UAuraAbilitySystemComponent::GetDescriptionsByAbilityTag(const FGameplayTag& AbilityTag, FString& OutDescription,
	FString& OutNextLevelDescription)
{
	// 获取被激活的GA
	if (const FGameplayAbilitySpec* AbilitySpec = GetSpecFromAbilityTag(AbilityTag))
	{
		if (UAuraGameplayAbility* AuraAbility = Cast<UAuraGameplayAbility>(AbilitySpec->Ability))
		{
			OutDescription = AuraAbility->GetDescription(AbilitySpec->Level);
			OutNextLevelDescription = AuraAbility->GetNextLevelDescription(AbilitySpec->Level + 1);
			return true;
		}
	}

	// 获取全部GA
	const UAbilityInfo* AbilityInfo = UAuraAbilitySystemLibrary::GetAbilityInfo(GetAvatarActor());
	// 获取描述
	int32 Level = AbilityInfo->FindAbilityInfoForTag(AbilityTag).LevelRequirement;
	OutDescription = UAuraGameplayAbility::GetLockedDescription(Level);
	OutNextLevelDescription = FString();
	return false;
}

FGameplayAbilitySpec* UAuraAbilitySystemComponent::GetSpecFromAbilityTag(const FGameplayTag& AbilityTag)
{
	// *** 这个函数将 循环更新GA能力，需要加锁
	FScopedAbilityListLock ActiveScopeLoc(*this);
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		for (FGameplayTag Tag : AbilitySpec.Ability.Get()->AbilityTags)
		{
			if (AbilityTag.MatchesTag(Tag))
			{
				return &AbilitySpec;
			}
		}
	}
	return nullptr;
}

void UAuraAbilitySystemComponent::OnRep_ActivateAbilities()
{
	Super::OnRep_ActivateAbilities();

	if (!bStartupAbilitiesGiven)
	{
		bStartupAbilitiesGiven = true;
		// 该赋予的能力都赋予完了，广播
		AbilitiesGivenDelegate.Broadcast();
	}
}


void UAuraAbilitySystemComponent::AbilityInputTagHeld(const FGameplayTag& InputTag)
{
	if ( ! InputTag.IsValid()) return;

	// 返回所有   可激活   能力的列表
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		// Exact 精确匹配
		if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag))
		{
			// 标识这个技能 被按键按下 （内部函数）
			// 举个例子，假设你的游戏中有一个技能系统，某个角色在按下技能键后会发动一次攻击。
			// 你可以使用AbilitySpecInputPressed函数来监听玩家的技能键输入事件，并在触发时执行相应的攻击操作
			AbilitySpecInputPressed(AbilitySpec);
			
			// 检查激活
			if (!AbilitySpec.IsActive())
			{
				// ***** Try尝试激活能力（尝试 ---> 可能其他会阻止这个GA，所以是Try）
				// 在客户端调用TryActivateAbility触发GameplayAbility时，服务器会同时激活对应的GameplayAbility以保持游戏的同步性和一致性。
				TryActivateAbility(AbilitySpec.Handle);
			}
		}
	}
	
}

void UAuraAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if ( ! InputTag.IsValid()) return;

	// 返回所有   可激活   能力的列表
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		// Exact 精确匹配
		if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag))
		{
			// 标识这个技能 被按键释放 （内部函数）
			// 举个例子，假设你的游戏中有一个技能系统，某个角色在释放技能键后会发动一次攻击。
			// 你可以使用AbilitySpecInputPressed函数来监听玩家的技能键输入事件，并在触发时执行相应的攻击操作
			AbilitySpecInputReleased(AbilitySpec);

		}
	}
}


FGameplayTag UAuraAbilitySystemComponent::GetAbilityTagFromSpec(const FGameplayAbilitySpec& AbilitySpec)
{
	if (AbilitySpec.Ability)
	{
		for (FGameplayTag Tag : AbilitySpec.Ability.Get()->AbilityTags)
		{
			// 两个Tag之间的 不精确的 匹配
			if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Abilities"))))
			{
				return Tag;
			}
		}
	}
	return FGameplayTag();
}

FGameplayTag UAuraAbilitySystemComponent::GetInputTagFromSpec(const FGameplayAbilitySpec& AbilitySpec)
{
	for (FGameplayTag Tag : AbilitySpec.DynamicAbilityTags)
	{
		if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("InputTag"))))
		{
			return Tag;
		}
	}

	return FGameplayTag();
}

FGameplayTag UAuraAbilitySystemComponent::GetStatusFromSpec(const FGameplayAbilitySpec& AbilitySpec)
{
	// 返回包含的第一个带Status的Tag
	for (FGameplayTag StatusTag : AbilitySpec.DynamicAbilityTags)
	{
		if (StatusTag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Abilities.Status"))))
		{
			return StatusTag;
		}
	}
	
	return FGameplayTag();
}

void UAuraAbilitySystemComponent::UpdateAbilityStatuses(int32 Level)
{
	UAbilityInfo* AbilityInfo = UAuraAbilitySystemLibrary::GetAbilityInfo(GetAvatarActor());
	for (const FAuraAbilityInfo& Info : AbilityInfo->AbilityInformation)
	{
		if (!Info.AbilityTag.IsValid()) continue;
		
		// 检查等级是否满足
		if (Level < Info.LevelRequirement) continue;
		
		// 判断 数据资产 中的GA是否 已经被激活了
		if (GetSpecFromAbilityTag(Info.AbilityTag) == nullptr)
		{
			// 未被激活,开始创建并赋予能了GA
			FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(Info.Ability, 1);
			AbilitySpec.DynamicAbilityTags.AddTag(FAuraGameplayTags::Get().Abilities_Status_Eligible);
			
			GiveAbility(AbilitySpec);
			// **** 立即刷新并网络同步GA,以便引擎知道该能力规格的状态已经改变，需要进行更新或重绘,不必等到下一帧
			// MarkAbilitySpecDirty = 调用以标记能力规范已被修改
			MarkAbilitySpecDirty(AbilitySpec);

			// 服务器上执行Client RPC 两端各执行 一遍
			ClientUpdateAbilityStatus(Info.AbilityTag, FAuraGameplayTags::Get().Abilities_Status_Eligible, 1);
		}
	}
}

void UAuraAbilitySystemComponent::ServerSpendSpellPoint_Implementation(const FGameplayTag& AbilityTag)
{
	// 获取GA实例
	if (FGameplayAbilitySpec* AbilitySpec = GetSpecFromAbilityTag(AbilityTag))
	{
		// 技能点-1
		if (GetAvatarActor()->Implements<UPlayerInterface>())
		{
			IPlayerInterface::Execute_AddToSpellPoints(GetAvatarActor(), -1);
		}
		
		const FAuraGameplayTags GameplayTags = FAuraGameplayTags::Get();

		FGameplayTag Status = GetStatusFromSpec(*AbilitySpec);
		if (Status.MatchesTagExact(GameplayTags.Abilities_Status_Eligible))
		{
			// 符合解锁条件
			AbilitySpec->DynamicAbilityTags.RemoveTag(GameplayTags.Abilities_Status_Eligible);
			AbilitySpec->DynamicAbilityTags.AddTag(GameplayTags.Abilities_Status_UnLocked);

			Status = GameplayTags.Abilities_Status_UnLocked;
		}
		else if (Status.MatchesTagExact(GameplayTags.Abilities_Status_Equipped) || Status.MatchesTagExact(GameplayTags.Abilities_Status_UnLocked))
		{
			// 提升能力
			// 可在不取消GA下，直接提升能力，加等级
			AbilitySpec->Level += 1;
		}
		
		ClientUpdateAbilityStatus(AbilityTag, Status, AbilitySpec->Level);
		// 更新GA
		MarkAbilitySpecDirty(*AbilitySpec);
	}
}

void UAuraAbilitySystemComponent::ClientUpdateAbilityStatus_Implementation(const FGameplayTag& AbilityTag,const FGameplayTag& StatusTag, int32 AbilityLevel)
{
	AbilityStatusChanged.Broadcast(AbilityTag, StatusTag, AbilityLevel);
}

void UAuraAbilitySystemComponent::UpgradeAttribute(const FGameplayTag& AttributeTag)
{
	if (GetAvatarActor()->Implements<UPlayerInterface>())
	{
		// ***  1 节省RPC的判断
		if (IPlayerInterface::Execute_GetAttributePoints(GetAvatarActor()) > 0)
		{
			// *** 2 修改应该只发生在服务器
			ServerUpgradeAttribute(AttributeTag);
		}
	}
}

void UAuraAbilitySystemComponent::ServerUpgradeAttribute_Implementation(const FGameplayTag& AttributeTag)
{
	// *** 3 我们的被动能力，已经监听了任何属性下的 数值变化
	FGameplayEventData Payload;
	Payload.EventTag = AttributeTag;
	Payload.EventMagnitude = 1.f;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetAvatarActor(), AttributeTag, Payload);

	// 点数减一
	if (GetAvatarActor()->Implements<UPlayerInterface>())
	{
		IPlayerInterface::Execute_AddToAttributePoints(GetAvatarActor(), -1);
	}
}

void UAuraAbilitySystemComponent::ForEachAbility(const FForEachAbility& Delegate)
{
	// 锁定已激活GA表：含义就是---下面GetActivatableAbilities()表被锁定，并且跟踪记录锁定期间的 能力的 "增删改查" 操作 ，直到函数结束时，应用  锁定期间的操作
	FScopedAbilityListLock ActiveScopeLock(*this);
	
	// 遍历已激活GA
	for (const FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		// 执行代理，对代理的用法--->类似返回值操作
		if (!Delegate.ExecuteIfBound(AbilitySpec))
		{
			// %hs 打印函数名
			UE_LOG(LogAura, Error, TEXT("Failed to execute delegate in %hs "), __FUNCTION__);
		}
	}
}
