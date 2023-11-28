// yyyyyxxxxxx


#include "AbilitySystem/AuraAbilitySystemComponent.h"

#include "AuraGameplayTags.h"
#include "AbilitySystem/Abilities/AuraGameplayAbility.h"
#include "Aura/AuraLogChannels.h"

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
			GiveAbility(AbilitySpec);
		}
	}

	bStartupAbilitiesGiven = true;
	// 该赋予的能力都赋予完了，广播
	AbilitiesGivenDelegate.Broadcast(this);
}

void UAuraAbilitySystemComponent::AddCharacterPassiveAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupPassiveAbilities)
{
	for (TSubclassOf<UGameplayAbility> AbilityClass : StartupPassiveAbilities)
	{
		FGameplayAbilitySpec  AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);

		GiveAbilityAndActivateOnce(AbilitySpec);
	}
}

void UAuraAbilitySystemComponent::OnRep_ActivateAbilities()
{
	Super::OnRep_ActivateAbilities();

	if (!bStartupAbilitiesGiven)
	{
		bStartupAbilitiesGiven = true;
		// 该赋予的能力都赋予完了，广播
		AbilitiesGivenDelegate.Broadcast(this);
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
