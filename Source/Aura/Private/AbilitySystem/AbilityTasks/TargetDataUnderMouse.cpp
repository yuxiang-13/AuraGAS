// yyyyyxxxxxx


#include "AbilitySystem/AbilityTasks/TargetDataUnderMouse.h"
#include "Kismet/KismetSystemLibrary.h"
#include "AbilitySystemComponent.h"

UTargetDataUnderMouse* UTargetDataUnderMouse::CreateTargetDataUnderMouse(UGameplayAbility* OwningAbility)
{
	// 将能力GA绑定给任务，当GA激活时 触发下面的 Activate()
	UTargetDataUnderMouse* MyObj = NewAbilityTask<UTargetDataUnderMouse>(OwningAbility);
	return MyObj;
}

void UTargetDataUnderMouse::Activate()
{
	const bool bIsServer = Ability->GetAvatarActorFromActorInfo()->HasAuthority();
	const bool bIsLocallyControlled = Ability->GetCurrentActorInfo()->IsLocallyControlled();
	// 判断是不是本地客户端
	if (bIsLocallyControlled)
	{
		SendMouseCursorData();
	} else
	{
		const FGameplayAbilitySpecHandle SpecHandle = GetAbilitySpecHandle();
		const FPredictionKey ActivationPredictionKey = GetActivationPredictionKey();
		//TODO: 服务器下，监听数据
		//1 AbilityTargetDataMap中如果没有才进行，绑定到 AbilityTargetDataMap 属性图谱中
		AbilitySystemComponent.Get()->AbilityTargetDataSetDelegate(SpecHandle, ActivationPredictionKey).AddUObject(
			this,
			&ThisClass::OnTargetDataReplicatedCallback
		);
		//2 服务器进行远端接收TargetData并绑定对应函数,也就是 C 数据上传到 S 上时，触发委托
		//2.1 此函数会在 AbilityTargetDataMap 中找出 TargetSetDelegate 并触发广播，成功就返回true
		const bool bCalledDelegate = AbilitySystemComponent.Get()->CallReplicatedTargetDataDelegatesIfSet(SpecHandle, ActivationPredictionKey);
		// 没触发成功
		if (!bCalledDelegate)
		{
			// 让服务器等待 玩家数据
			SetWaitingOnRemotePlayerData();
		}
	}
	
}

void UTargetDataUnderMouse::SendMouseCursorData()
{
	// 创建 范作用域预测
	//创建一个网络同步点, 服务器必须等待预测的客户发送该事件的预测密钥。
	FScopedPredictionWindow ScopedPrediction(AbilitySystemComponent.Get());
	
	APlayerController* PC = Ability->GetCurrentActorInfo()->PlayerController.Get();
	FHitResult CursorHit;
	PC->GetHitResultUnderCursor(ECC_Visibility, false, CursorHit);
	// 不进行广播
	// ValidData.Broadcast(CursorHit.Location);

	// 数据Data信息
	FGameplayAbilityTargetData_SingleTargetHit* Data = new FGameplayAbilityTargetData_SingleTargetHit();
	Data->HitResult = CursorHit;
	// 数据DataHandle
	FGameplayAbilityTargetDataHandle DataHandle;
	DataHandle.Add(Data);
	
	// 发送到服务器
	AbilitySystemComponent.Get()->ServerSetReplicatedTargetData(
		GetAbilitySpecHandle(),
		GetActivationPredictionKey(),
		DataHandle,
		FGameplayTag(),
		AbilitySystemComponent.Get()->ScopedPredictionKey
	);

	// 判断GA是否依然是活跃状态，活跃就广播
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		// 客户端函数直接广播
		ValidData.Broadcast(DataHandle);
	}
}

void UTargetDataUnderMouse::OnTargetDataReplicatedCallback(const FGameplayAbilityTargetDataHandle& DataHandle,
	FGameplayTag ActivationTag)
{
	const FGameplayAbilitySpecHandle SpecHandle = GetAbilitySpecHandle();
	const FPredictionKey ActivationPredictionKey = GetActivationPredictionKey();
	// 3 能力数据已经收到，把这个 AbilityTargetDataMap 图谱中对应的变量清除 (目标数据已经服务器收到，不用保存在map中了)
	AbilitySystemComponent.Get()->ConsumeClientReplicatedTargetData(SpecHandle, ActivationPredictionKey);

	
	// 判断GA是否依然是活跃状态，活跃就广播
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		// 客户端函数直接广播
		ValidData.Broadcast(DataHandle);
	}
}
