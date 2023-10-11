// yyyyyxxxxxx

#pragma once

#include "CoreMinimal.h"
#include "UI/WidgetController/AuraWidgetController.h"
#include "OverlayWidgetController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealtChangeSignature, float, NewHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMaxHealtChangeSignature, float, NewMaxHealth);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnManaChangeSignature, float, NewMana);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMaxManaChangeSignature, float, NewMaxMana);

struct FOnAttributeChangeData;

UCLASS()
class AURA_API UOverlayWidgetController : public UAuraWidgetController
{
	GENERATED_BODY()
public:
	virtual void BroadcastInitialValues() override;
	virtual void BindCallbackToDependencies() override;
	UPROPERTY(BlueprintAssignable, Category="GAS|Attributes")
	FOnHealtChangeSignature OnHealtChanged;
	UPROPERTY(BlueprintAssignable, Category="GAS|Attributes")
	FOnMaxHealtChangeSignature OnMaxHealtChanged;

	
	UPROPERTY(BlueprintAssignable, Category="GAS|Attributes")
	FOnManaChangeSignature OnManaChanged;
	
	UPROPERTY(BlueprintAssignable, Category="GAS|Attributes")
	FOnMaxManaChangeSignature OnMaxManaChanged;

protected:
	void HealthChanged(const FOnAttributeChangeData& Data) const;
	void MaxHealthChanged(const FOnAttributeChangeData& Data) const;
	
	void ManaChanged(const FOnAttributeChangeData& Data) const;
	void MaxManaChanged(const FOnAttributeChangeData& Data) const;
};
