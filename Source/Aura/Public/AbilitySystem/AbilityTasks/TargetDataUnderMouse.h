// yyyyyxxxxxx

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "TargetDataUnderMouse.generated.h"

class UGameplayAbility;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMouseTargetDataSignature, const FGameplayAbilityTargetDataHandle&, DataHandle);

UCLASS()
class AURA_API UTargetDataUnderMouse : public UAbilityTask
{
	GENERATED_BODY()
public:
	/*
	1. HidePin: 这个标记用于隐藏继承自"OwningAbility"的属性的连接线。在UE4中，当使用蓝图编辑器连接节点时，通常会显示一个连线，但是通过设置HidePin属性，你可以将该连线隐藏起来。
	2. DefaultToSelf: 这个标记用于告诉UE4在未明确指定目标时，默认将目标设置为"OwningAbility"
	3. 函数限制为仅内部使用，而不暴露给蓝图的使用者。通过设置BlueprintInternalUseOnly为"true"，这些成员将只在内部使用，而不会在蓝图编辑器的公开接口中显示出来
	*/
	UFUNCTION(BlueprintCallable, Category="Ability|Tasks", meta = (DispalyName = "TargetDataUnderMouse",  HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true"))
	static UTargetDataUnderMouse* CreateTargetDataUnderMouse(UGameplayAbility* OwningAbility);
	
	UPROPERTY(BlueprintAssignable)
	FMouseTargetDataSignature ValidData;

private:
	// 能力被激活时
	virtual void Activate() override;

	// 发送鼠标位置
	void SendMouseCursorData();

	void OnTargetDataReplicatedCallback(const FGameplayAbilityTargetDataHandle& DataHandle, FGameplayTag ActivationTag);
};
