// yyyyyxxxxxx


#include "Player/AuraPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
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
	ThisActor = Cast<IEnemyInterface>(CursorHit.GetActor());

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
			ThisActor->HighLightActor();
		} else
		{
			// Case A
		}
	} else
	{
		if (ThisActor == nullptr)
		{
			// Case C
			LastActor->UnHighLightActor();
		} else // same
		{
			if (LastActor != ThisActor)
			{
				// Case D
				LastActor->UnHighLightActor();
				ThisActor->HighLightActor();
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
	check(SubSystem)
	// 添加上下文
	SubSystem->AddMappingContext(AuraContext, 0);

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
	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);
	
	// 关联输入动作InputAction以及绑定对应事件
	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAuraPlayerController::Move);
	
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