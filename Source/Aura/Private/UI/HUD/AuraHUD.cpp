// yyyyyxxxxxx


#include "UI/HUD/AuraHUD.h"

#include "UI/WidgetController/AttributeWidgetController.h"
#include "UI/WidgetController/OverlayWidgetController.h"

void AAuraHUD::InitOverlay(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS)
{
	// *********** 相对于check() 下面checkf()可以写入一句话到崩溃日志
	checkf(OverlayWidgetClass, TEXT("Overlay Widget Class UnInitialized, please fill out BP_AuraHUD"));
	checkf(OverlayWidgetControllerClass, TEXT("Overlay Widget Controller Class UnInitialized, please fill out BP_AuraHUD"));

	//1 创建 最底层(Overlay) WidgetController
	const FWidgetControllerParams WidgetControllerParams(PC, PS, ASC, AS);
	UOverlayWidgetController* WidgetController = GetOverlayWidgetController(WidgetControllerParams);
	
	//2 创建 Widget
	UUserWidget* Widget = CreateWidget<UUserWidget>(GetWorld(), OverlayWidgetClass);
	OverlayWidget = Cast<UAuraUserWidget>(Widget);

	// 3 关联到一起
	OverlayWidget->SetWidgetController(WidgetController);
	// 广播血量
	WidgetController->BroadcastInitialValues();
	
	Widget->AddToViewport();
}

UOverlayWidgetController* AAuraHUD::GetOverlayWidgetController(const FWidgetControllerParams& WCParams)
{
	if (OverlayWidgetController == nullptr)
	{
		// *************** 第二个参数是 UClass，所以类型 TSubclassof
		OverlayWidgetController = NewObject<UOverlayWidgetController>(this, OverlayWidgetControllerClass);
		OverlayWidgetController->SetWidgetControllerParams(WCParams);

		
		// Control初始化时就绑定上事件
		OverlayWidgetController->BindCallbackToDependencies();
	}
	return OverlayWidgetController;
}

UAttributeWidgetController* AAuraHUD::GetAttributeMenuWidgetController(const FWidgetControllerParams& WCParams)
{
	if (AttributeMenuWidgetController == nullptr)
	{
		AttributeMenuWidgetController = NewObject<UAttributeWidgetController>(this, AttributeMenuWidgetControllerClass);
		AttributeMenuWidgetController->SetWidgetControllerParams(WCParams);

		// Control初始化时就绑定上事件
		AttributeMenuWidgetController->BindCallbackToDependencies();
	}
	return AttributeMenuWidgetController;
}
