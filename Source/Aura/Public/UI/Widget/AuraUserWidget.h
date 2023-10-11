// yyyyyxxxxxx

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AuraUserWidget.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API UAuraUserWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UObject> WidgetController;

	// 蓝图触发c++的函数
	UFUNCTION(BlueprintCallable)
	void SetWidgetController(UObject* InWidgetController);
protected:
	// c++触发蓝图的函数
	UFUNCTION(BlueprintImplementableEvent)
	void WidgetControllerSet();
};
