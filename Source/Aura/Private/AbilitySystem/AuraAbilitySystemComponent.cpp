// yyyyyxxxxxx


#include "AbilitySystem/AuraAbilitySystemComponent.h"

#include "AuraGameplayTags.h"
#include "AbilitySystem/Abilities/AuraGameplayAbility.h"
#include "Kismet/KismetSystemLibrary.h"

void UAuraAbilitySystemComponent::AbilityActorInfoSet()
{
	// 每当GE应用于自身时在服务器上调用。这包括基于即时和持续时间的GE
	OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &UAuraAbilitySystemComponent::EffectApplied);

}

void UAuraAbilitySystemComponent::EffectApplied(UAbilitySystemComponent* AbilitySystemComponent,
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
}

void UAuraAbilitySystemComponent::AbilityInputTagHeld(const FGameplayTag& InputTag)
{
	if ( ! InputTag.IsValid()) return;

	UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT(" ------->>> is    ")), true, true, FLinearColor::Red, 10.f);

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
