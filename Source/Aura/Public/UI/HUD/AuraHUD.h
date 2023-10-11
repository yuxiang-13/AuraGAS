// yyyyyxxxxxx

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "UI/Widget/AuraUserWidget.h"
#include "AuraHUD.generated.h"

struct FWidgetControllerParams;
class UOverlayWidgetController;
class UAbilitySystemComponent;
class UAttributeSet;
/**
 * 
 */
UCLASS()
class AURA_API AAuraHUD : public AHUD
{
	GENERATED_BODY()
public:
	// 最底层UI  UserWidget 实例
	UPROPERTY()
	TObjectPtr<UAuraUserWidget> OverlayWidget;


	void InitOverlay(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS);
	UOverlayWidgetController* GetOverlayWidgetController(const FWidgetControllerParams& WCParams);
	
private:
	// 最底层UI  UserWidget 类
	UPROPERTY(EditAnywhere)
	TSubclassOf<UAuraUserWidget> OverlayWidgetClass;

	// 最底层UI  WidgetControlled UObject 实例
	UPROPERTY()
	TObjectPtr<UOverlayWidgetController> OverlayWidgetController;
	// 最底层UI  WidgetControlled UObject 类
	UPROPERTY(EditAnywhere)
	TSubclassOf<UOverlayWidgetController> OverlayWidgetControllerClass;
};
