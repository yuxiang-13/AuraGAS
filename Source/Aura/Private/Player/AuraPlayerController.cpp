// yyyyyxxxxxx


#include "Player/AuraPlayerController.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameplayTagContainer.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "Input/AuraInputComponent.h"
#include "Interacton/EnemyInterface.h"

AAuraPlayerController::AAuraPlayerController()
{
	// 服务器发生改变，下发复制到所有客户端
	bReplicates = true;
}

void AAuraPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
	CursorTrace();
}


void AAuraPlayerController::CursorTrace()
{
	// 检测鼠标命中
	FHitResult CursorHit;
	// Under 下面
	GetHitResultUnderCursor(ECC_Visibility, false, CursorHit);
	if (!CursorHit.bBlockingHit) return;

	LastActor = ThisActor;
	ThisActor = CursorHit.GetActor();

	/*
	Line trace from cursor. There are several scenarios:
	*A LastActor is null && ThisActor is null
		- Do nothing
	B. LastActor is null && ThisActor is valid*
		-Highlight ThisActor
	C. LastActor is valid && ThisActor is null*
		- UnHighlight LastActor
	D. Both actors are valid,but LastActor != ThisActor*
		- UnHighlight LastActor, and Highlight ThisActor*
	E. Both actors are valid, and are the same actor*
		- Do nothing
	*/
	if (LastActor == nullptr)
	{
		if (ThisActor != nullptr)
		{
			// Case B
			if (Cast<IEnemyInterface>(ThisActor))
			{
				Cast<IEnemyInterface>(ThisActor)->HighLightActor();
			}
		} else
		{
			// Case A
		}
	} else
	{
		if (ThisActor == nullptr)
		{
			// Case C
			if (Cast<IEnemyInterface>(LastActor))
			{
				Cast<IEnemyInterface>(LastActor)->UnHighLightActor();
			}
		} else // same
		{
			if (LastActor != ThisActor)
			{
				// Case D
				if (Cast<IEnemyInterface>(LastActor))
				{
					Cast<IEnemyInterface>(LastActor)->UnHighLightActor();
				}
				if (Cast<IEnemyInterface>(ThisActor))
				{
					Cast<IEnemyInterface>(ThisActor)->HighLightActor();
				}
			} else
			{
				// Case E
			}
		}
	}
}



void AAuraPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// 断言检测，无效指针直接崩溃
	check(AuraContext)

	UEnhancedInputLocalPlayerSubsystem* SubSystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());

	// ************  GetLocalPlayer()  走这块逻辑，返回的是真实有效的本地玩家，所以需要去掉断言,因为网络游戏下，都会走这个 BeginPlay逻辑
	if (SubSystem)
	{
		// 添加上下文
		SubSystem->AddMappingContext(AuraContext, 0);
	}

	//光标
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;

	// 输入模式
	FInputModeGameAndUI InputModeData;
	/** Do not lock the mouse cursor to the viewport */
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	// 不隐藏光标
	InputModeData.SetHideCursorDuringCapture(false);
	SetInputMode(InputModeData);
}

void AAuraPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// 转成增强输入
	// *** CastChecked 检测强转是否成功，不成功就像 check(xx) 直接断言 崩溃
	UAuraInputComponent* AuraInputComponent = CastChecked<UAuraInputComponent>(InputComponent);
	
	// 关联输入动作InputAction以及绑定对应事件
	AuraInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAuraPlayerController::Move);

	// ***********  ThisClass 表示这个类文件   this 表示这个类
	AuraInputComponent->BindAbilityActions(InputConfig, this, &ThisClass::AbilityInputTagPressed, &AAuraPlayerController::AbilityInputTagReleased, &AAuraPlayerController::AbilityInputTagTagHeld);
}

void AAuraPlayerController::AbilityInputTagPressed(FGameplayTag InputTag)
{
	// GEngine->AddOnScreenDebugMessage(1, 3.f, FColor::Red, *InputTag.ToString());
}

void AAuraPlayerController::AbilityInputTagReleased(FGameplayTag InputTag)
{
	GEngine->AddOnScreenDebugMessage(2, 3.f, FColor::Blue, *InputTag.ToString());

	if (GetASC() == nullptr) { return; }
	GetASC()->AbilityInputTagReleased(InputTag);
}

void AAuraPlayerController::AbilityInputTagTagHeld(FGameplayTag InputTag)
{
	// 。。。 一直触发
	GEngine->AddOnScreenDebugMessage(3, 3.f, FColor::Green, *InputTag.ToString());

	if (GetASC() == nullptr) { return; }
	GetASC()->AbilityInputTagHeld(InputTag);
}

UAuraAbilitySystemComponent* AAuraPlayerController::GetASC()
{
	if (AuraAbilitySystemComponent == nullptr)
	{
		// 获取能力组件
		UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn<APawn>());
		AuraAbilitySystemComponent = Cast<UAuraAbilitySystemComponent>(AbilitySystemComponent);
	}
	return AuraAbilitySystemComponent;
}

void AAuraPlayerController::Move(const FInputActionValue& InputActionValue)
{
	const FVector2D InputAxisVector = InputActionValue.Get<FVector2D>();
	// 控制器yaw 左右就够用了
	const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw ,0.f);

	// *** 获取前向向量 （矩阵的单位长度 向量自然带方向，实际就是把FRotator转成FVector） Unit=单位
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	// 设置Pawn移动
	if (APawn* ControlledPawn = GetPawn<APawn>())
	{
		// 前后移动  == 对映2D 的Y
		ControlledPawn->AddMovementInput(ForwardDirection, InputAxisVector.Y);
		
		// 左右移动  == 对映2D 的X
		ControlledPawn->AddMovementInput(RightDirection, InputAxisVector.X);
	}
}