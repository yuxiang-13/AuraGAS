// yyyyyxxxxxx


#include "AbilitySystem/AsyncTasks/WaitColldownChange.h"

#include "AbilitySystemComponent.h"

UWaitColldownChange* UWaitColldownChange::WaitForCooldownChange(UAbilitySystemComponent* AbilitySystemComponent,
                                                                const FGameplayTag& InCooldownTag)
{
	// 1 实例化Object
	UWaitColldownChange* WaitColldownChange = NewObject<UWaitColldownChange>();
	// 2 设置属性
	WaitColldownChange->ASC = AbilitySystemComponent;
	WaitColldownChange->CooldownTag = InCooldownTag;

	if (!IsValid(AbilitySystemComponent) || !InCooldownTag.IsValid())
	{
		WaitColldownChange->EndTask();
		return nullptr;
	}
	// 绑定 标签赋予时的 代理
	AbilitySystemComponent->RegisterGameplayTagEvent(InCooldownTag, EGameplayTagEventType::NewOrRemoved).AddUObject(
		WaitColldownChange, // ****** 注意，这里是 WaitColldownChange 不是 This
		&UWaitColldownChange::CooldownTagChanged
	);
	// 绑定GE效果施加自己 [无论何时添加基于   duraton的GE，  都会在客户端和服务器上调用（例如，即时GE不会触发此操作）]
	AbilitySystemComponent->OnActiveGameplayEffectAddedDelegateToSelf.AddUObject(
		WaitColldownChange, // ****** 注意，这里是 WaitColldownChange 不是 This
		&UWaitColldownChange::OnActiveEffectAdded
	);
	

	return WaitColldownChange;
}

void UWaitColldownChange::EndTask()
{
	if (IsValid(ASC))
	{
		// 任务结束时
		ASC->RegisterGameplayTagEvent(CooldownTag, EGameplayTagEventType::NewOrRemoved).RemoveAll(this);

	}
	// 准备销毁
	SetReadyToDestroy();
	// 标记垃圾
	MarkAsGarbage();
}

void UWaitColldownChange::CooldownTagChanged(const FGameplayTag InCooldownTag, int32 NewCount)
{
	if (NewCount == 0)
	{
		CooldownEnd.Broadcast(0.f);
	}
}

void UWaitColldownChange::OnActiveEffectAdded(UAbilitySystemComponent* TargetASC,
	const FGameplayEffectSpec& SpecApplied, FActiveGameplayEffectHandle ActiveEffectHandle)
{
	// 获取冷却时间
	FGameplayTagContainer AssetTags;
	SpecApplied.GetAllAssetTags(AssetTags);

	FGameplayTagContainer GrantedTags;
	SpecApplied.GetAllGrantedTags(GrantedTags);

	if (AssetTags.HasTagExact(CooldownTag) || GrantedTags.HasTagExact(CooldownTag))
	{
		FGameplayTagContainer InTags = CooldownTag.GetSingleTagContainer();
		// 检测包含此 Tag容器内所有Tag【这就表示要匹配数组内所有的才行，也就是说我们的冷却，只能是一个单一的标签GE】
		FGameplayEffectQuery GameplayEffectQuery = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(CooldownTag.GetSingleTagContainer());
		// 获取所有的满足 筛选条件的GE 的剩余时间数组
		TArray<float> TimesRemaining = ASC->GetActiveEffectsTimeRemaining(GameplayEffectQuery);

		// 遍历数组，找出最大的剩余时间【剩余时间Tag 是根据 GA 对应起来的，一对一】
		if (TimesRemaining.Num() > 0) {
			float TimeRemaining = TimesRemaining[0];
			for (int32 i = 0; i < TimesRemaining.Num(); i++)
			{
				if (TimesRemaining[i] > TimeRemaining)
				{
					TimeRemaining = TimesRemaining[i];
				}
			}

			CooldownStart.Broadcast(TimeRemaining);
		}
		
	}
}
