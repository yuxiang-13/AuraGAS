// yyyyyxxxxxx

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "MVVM_LoadSlot.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSetWidgetSwitcherIndex, int32, WidgetSwitcherIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEnableSelectSlotButton, bool, bEnable);

UCLASS()
class AURA_API UMVVM_LoadSlot : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FSetWidgetSwitcherIndex SetWidgetSwitcherIndex;
	
	UPROPERTY(BlueprintAssignable)
	FEnableSelectSlotButton EnableSelectSlotButton;

	void InitializeSlot();

	TEnumAsByte<ESaveSlotStates> SlotStatus;
	
	UPROPERTY()
	int32 SlotIndex;

	UPROPERTY()
	FName PlayerStartTag;
	
	// FieldNotify, 标记此变量是一个字段通知， Setter, Getter 意味着需要实现 GetSet方法
	/* Field Notifies */
	void SetPlayerName(FString InPlayerName);
	FString GetPlayerName() const { return PlayerName; };
	/* Field Notifies */
	void SetLoadSlotName(FString InLoadSlotName);
	FString GetLoadSlotName() const { return LoadSlotName; };
	/* Field Notifies */
	void SetMapName(FString InMapName);
	FString GetMapName() const { return MapName; };
	/* Field Notifies */
	void SetPlayerLevel(int32 InPlayerLevel);
	int32 GetPlayerLevel() const { return PlayerLevel; };

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, FieldNotify, Setter, Getter, meta=(AllowPrivateAccess = "true"))
	FString LoadSlotName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, FieldNotify, Setter, Getter, meta=(AllowPrivateAccess = "true"))
	FString PlayerName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, FieldNotify, Setter, Getter, meta=(AllowPrivateAccess = "true"))
	FString MapName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, FieldNotify, Setter, Getter, meta=(AllowPrivateAccess = "true"))
	int32 PlayerLevel;
	
};
