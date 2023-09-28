// yyyyyxxxxxx


#include "Character/AuraCharacter.h"

#include "AbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Player/AuraPlayerState.h"

AAuraCharacter::AAuraCharacter()
{
	GetCharacterMovement()->bOrientRotationToMovement = true;
	// *****   转向速率
	GetCharacterMovement()->RotationRate = FRotator(0.f, 400.f, 0.f);

	// 如果为true，则移动将被约束到一个平面
	GetCharacterMovement()->bConstrainToPlane = true;
	// 尝试将其位置与地面或平面对齐，以确保角色始终在地面上或特定平面上移动
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	// 上下低头
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw  = false;
}

void AAuraCharacter::PossessedBy(AController* NewController)
{
	// 这个函数只Server触发
	Super::PossessedBy(NewController);

	InitAbilityActorInfo();
}

void AAuraCharacter::OnRep_PlayerState()
{
	// 这个函数只客户端触发
	Super::OnRep_PlayerState();
	
	InitAbilityActorInfo();
}

// 初始化技能组件函数
void AAuraCharacter::InitAbilityActorInfo()
{
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	AuraPlayerState->GetAbilitySystemComponent()->InitAbilityActorInfo(AuraPlayerState, this);

	// Character记录自己的 成员变量进行初始化
	AbilitySystemComponent = AuraPlayerState->GetAbilitySystemComponent();
	AttributeSet = AuraPlayerState->GetAttributeSet();
}
