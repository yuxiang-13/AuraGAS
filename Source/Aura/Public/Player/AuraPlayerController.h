// yyyyyxxxxxx

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Interacton/EnemyInterface.h"
#include "AuraPlayerController.generated.h"


class USplineComponent;
class UAuraAbilitySystemComponent;
struct FGameplayTag;
class UAuraInputConfig;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;



UCLASS()
class AURA_API AAuraPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	AAuraPlayerController();
protected:
	virtual void PlayerTick(float DeltaTime) override;
	
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
private:
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputMappingContext> AuraContext;
	// 移动 输入Action
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> MoveAction;
	void Move(const FInputActionValue& InputActionValue);
	
	// 发射火球 输入Shift
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> ShiftAction;
	void ShiftPressed() { bShiftKeyDown = true; };
	void ShiftRelease() { bShiftKeyDown = false; };
	bool bShiftKeyDown = false;

	// 鼠标跟踪
	void CursorTrace();
	TObjectPtr<AActor> LastActor;
	TObjectPtr<AActor> ThisActor;
	// 检测鼠标命中
	FHitResult CursorHit;

	void AbilityInputTagPressed(FGameplayTag InputTag);
	void AbilityInputTagReleased(FGameplayTag InputTag);
	void AbilityInputTagTagHeld(FGameplayTag InputTag);

	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UAuraInputConfig> InputConfig;

	
	UAuraAbilitySystemComponent* GetASC();
	UPROPERTY()
	TObjectPtr<UAuraAbilitySystemComponent> AuraAbilitySystemComponent;

	FVector CachedDestination = FVector::ZeroVector;
	float FollowTime = 0.f;
	float ShortPressThreshold = 0.5f;
	bool bAutoRunning = false;
	bool bTargeting = false;
	
	UPROPERTY(EditDefaultsOnly)
	float AutoRunAcceptanceRadius = 50.f;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<USplineComponent> Spline;

	void AutoRun();
};
