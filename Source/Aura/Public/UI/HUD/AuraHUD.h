// yyyyyxxxxxx

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "UI/Widget/AuraUserWidget.h"
#include "AuraHUD.generated.h"

class USpellMenuWidgetController;
struct FWidgetControllerParams;
class UOverlayWidgetController;
class UAbilitySystemComponent;
class UAttributeWidgetController;
class UAttributeSet;
/**
 * 
 */
UCLASS()
class AURA_API AAuraHUD : public AHUD
{
	GENERATED_BODY()
public:
	void InitOverlay(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS);
	UOverlayWidgetController* GetOverlayWidgetController(const FWidgetControllerParams& WCParams);
	UAttributeWidgetController* GetAttributeMenuWidgetController(const FWidgetControllerParams& WCParams);
	USpellMenuWidgetController* GetSpellMenuWidgetController(const FWidgetControllerParams& WCParams);
private:
	// 最底层UI  UserWidget 类
	UPROPERTY(EditAnywhere)
	TSubclassOf<UAuraUserWidget> OverlayWidgetClass;
	// 最底层UI  WidgetControlled UObject 类
	UPROPERTY(EditAnywhere)
	TSubclassOf<UOverlayWidgetController> OverlayWidgetControllerClass;

	// 最底层UI  WidgetControlled UObject 实例
	UPROPERTY()
	TObjectPtr<UOverlayWidgetController> OverlayWidgetController;
	// 最底层UI  UserWidget 实例
	UPROPERTY()
	TObjectPtr<UAuraUserWidget> OverlayWidget;
	
	// 属性菜单  UserWidget 实例
	UPROPERTY()
	TObjectPtr<UAttributeWidgetController> AttributeMenuWidgetController;
	// 属性菜单   WidgetControlled UObject 类
	UPROPERTY(EditAnywhere)
	TSubclassOf<UAttributeWidgetController> AttributeMenuWidgetControllerClass;


	
	// 技能树  UserWidget 实例
	UPROPERTY()
	TObjectPtr<USpellMenuWidgetController> SpellMenuWidgetController;
	// 技能树   WidgetControlled UObject 类
	UPROPERTY(EditAnywhere)
	TSubclassOf<USpellMenuWidgetController> SpellMenuWidgetControllerClass;
};
