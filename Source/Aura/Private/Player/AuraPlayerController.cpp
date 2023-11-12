// yyyyyxxxxxx


#include "Player/AuraPlayerController.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AuraGameplayTags.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameplayTagContainer.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "Components/SplineComponent.h"
#include "GameFramework/Character.h"
#include "Input/AuraInputComponent.h"
#include "Interacton/EnemyInterface.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UI/Widget/DamageTextComponent.h"

AAuraPlayerController::AAuraPlayerController()
{
	// 服务器发生改变，下发复制到所有客户端
	bReplicates = true;
	// 样条线
	Spline = CreateDefaultSubobject<USplineComponent>("Spline");
}

void AAuraPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
	CursorTrace();

	AutoRun();
}

void AAuraPlayerController::AutoRun()
{
	if (!bAutoRunning) return;
	if (APawn* ControlledPawn = GetPawn())
	{
		// closest 接近
		// 函数会考虑到环境的阻碍物、碰撞等因素，以尽量准确地找到一个与指定世界坐标最接近的可用位置。如果有多个匹配的位置，函数将选择其中的一个返回
		const FVector LocationOnSpline = Spline->FindLocationClosestToWorldLocation(ControlledPawn->GetActorLocation(), ESplineCoordinateSpace::World);
		// 根据曲线位置，（曲线方向从0到1）获取移动方向
		const FVector Direction = Spline->FindDirectionClosestToWorldLocation(LocationOnSpline, ESplineCoordinateSpace::World);
		ControlledPawn->AddMovementInput(Direction);

		const float DistanceToDestination = (LocationOnSpline - CachedDestination).Length();
		if (DistanceToDestination <= AutoRunAcceptanceRadius)
		{
			// 关闭自动移动
			bAutoRunning = false;
		}
	}
}

void AAuraPlayerController::CursorTrace()
{
	// Under 下面
	GetHitResultUnderCursor(ECC_Visibility, false, CursorHit);
	if (!CursorHit.bBlockingHit) return;

	LastActor = ThisActor;
	ThisActor = CursorHit.GetActor();

	if (LastActor != ThisActor)
	{
		if (Cast<IEnemyInterface>(LastActor))
		{
			Cast<IEnemyInterface>(LastActor)->UnHighLightActor();
		}
		if (Cast<IEnemyInterface>(ThisActor))
		{
			Cast<IEnemyInterface>(ThisActor)->HighLightActor();
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

	AuraInputComponent->BindAction(ShiftAction, ETriggerEvent::Started, this, &AAuraPlayerController::ShiftPressed);
	AuraInputComponent->BindAction(ShiftAction, ETriggerEvent::Completed, this, &AAuraPlayerController::ShiftRelease);
	
	// ***********  ThisClass 表示这个类文件   this 表示这个类
	AuraInputComponent->BindAbilityActions(InputConfig, this, &ThisClass::AbilityInputTagPressed, &AAuraPlayerController::AbilityInputTagReleased, &AAuraPlayerController::AbilityInputTagTagHeld);
}

// 客户端RPC
void AAuraPlayerController::ShowDamageNumber_Implementation(float DamageAmount, ACharacter* TargetCharacter, bool bBlockedHit, bool bCriticalHit)
{
	//  
	if (IsValid(TargetCharacter) && DamageTextComponentClass && IsLocalController())
	{
		
		// 动态创建Object组件
		UDamageTextComponent* DamageText = NewObject<UDamageTextComponent>(TargetCharacter, DamageTextComponentClass);
		// ***** 必须先注册组件  (因为这一步是不是馋鬼的再默认构造函数中进行，而是动态创建的)
		DamageText->RegisterComponent();
		DamageText->AttachToComponent(TargetCharacter->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);

		// **** 花样来了。。。直接分离，因为Text想要脱离玩家自我往上飘，而不是随着玩家走动。。。。。
		DamageText->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		DamageText->SetDamageText(DamageAmount, bBlockedHit, bCriticalHit);
	}
}

void AAuraPlayerController::AbilityInputTagTagHeld(FGameplayTag InputTag)
{
	if (!InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_LMB))
	{
		if (GetASC()) GetASC()->AbilityInputTagHeld(InputTag);
		return;
	}

	if (bTargeting || bShiftKeyDown)
	{
		// // 在客户端调用TryActivateAbility触发GameplayAbility时，服务器会同时激活对应的GameplayAbility以保持游戏的同步性和一致性。
		if (GetASC()) GetASC()->AbilityInputTagHeld(InputTag);
	} else
	{
		FollowTime += GetWorld()->GetDeltaSeconds();
		if (CursorHit.bBlockingHit) CachedDestination = CursorHit.ImpactPoint;
		if (APawn* ControlledPawn = GetPawn())
		{
			const FVector WorldDirection = (CachedDestination - ControlledPawn->GetActorLocation()).GetSafeNormal();
			ControlledPawn->AddMovementInput(WorldDirection);
		}
	}
}


void AAuraPlayerController::AbilityInputTagPressed(FGameplayTag InputTag)
{
	// GEngine->AddOnScreenDebugMessage(1, 3.f, FColor::Red, *InputTag.ToString());
	if (InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_LMB))
	{
		if (Cast<IEnemyInterface>(LastActor))
		{
			bTargeting = true;
		} else
		{
			bTargeting = false;
		}
		bAutoRunning = false;
	}
}

void AAuraPlayerController::AbilityInputTagReleased(FGameplayTag InputTag)
{
	// GEngine->AddOnScreenDebugMessage(2, 3.f, FColor::Blue, *InputTag.ToString());

	if (!InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_LMB))
	{
		if (GetASC()) GetASC()->AbilityInputTagReleased(InputTag);
		return;
	}

	if (GetASC()) GetASC()->AbilityInputTagReleased(InputTag);
	
	if (!bTargeting && !bShiftKeyDown)
	{
		const APawn* ControlledPawn = GetPawn();
		if (FollowTime <= ShortPressThreshold && ControlledPawn)
		{
			if (UNavigationPath* NavPath = UNavigationSystemV1::FindPathToLocationSynchronously(this, ControlledPawn->GetActorLocation(), CachedDestination))
			{
				// 清除样条线点
				Spline->ClearSplinePoints();
				for (const FVector& PointLoc : NavPath->PathPoints)
				{
					Spline->AddSplinePoint(PointLoc, ESplineCoordinateSpace::World);
				}

				if (NavPath->PathPoints.Num() > 0)
				{
					// 刷新最后点击位置
					CachedDestination = NavPath->PathPoints[NavPath->PathPoints.Num() - 1];
					bAutoRunning = true;
				}
			}
		}

		FollowTime = 0.f;
		bTargeting = false;
	}
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
