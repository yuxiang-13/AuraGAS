// yyyyyxxxxxx


#include "Character/AuraCharacter.h"

#include "GameFramework/CharacterMovementComponent.h"

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
