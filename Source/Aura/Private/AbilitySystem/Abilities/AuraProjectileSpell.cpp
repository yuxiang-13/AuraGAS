// yyyyyxxxxxx


#include "AbilitySystem/Abilities/AuraProjectileSpell.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Actor/AuraProjectile.h"
#include "Interacton/CombatInterface.h"

void UAuraProjectileSpell::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                           const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                           const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	// UKismetSystemLibrary::PrintString(this, FString("ActivateAbility (c++)"), true, true, FLinearColor::Red, 3);

}

void UAuraProjectileSpell::SpawnProjectile(const FVector& ProjectileTargetLocation, const FGameplayTag& SocketTag, bool bOverridePitch, float PitchOverride)
{
	const bool bIsServer = GetAvatarActorFromActorInfo()->HasAuthority();
	if (!bIsServer) return;

	// 获取接口位置
	ICombatInterface* CombatInterface = Cast<ICombatInterface>(GetAvatarActorFromActorInfo());
	if (CombatInterface)
	{
		// 1 下面这个报错
		// const FVector SocketLocation = CombatInterface->GetCombatSocketLocation();
		// 2 参数代表着强转的类
		const FVector SocketLocation = ICombatInterface::Execute_GetCombatSocketLocation(
			GetAvatarActorFromActorInfo(),
			SocketTag
		);
		// 1 A-B = B看向A  获取武器插槽位置到目标点的 向量的旋转
		FRotator Rotatior = (ProjectileTargetLocation - SocketLocation).Rotation();
		if (bOverridePitch) {
			Rotatior.Pitch = PitchOverride;
		}

		FTransform SpawnTransform;
		SpawnTransform.SetLocation(SocketLocation);
		// TODO: 给导弹 添加 Rotation （Rotatior -> Rotation  用 Rotatior.Quaternion()）
		SpawnTransform.SetRotation(Rotatior.Quaternion());

		// SpawnActor 直接就生成了
		// SpawnActorDeferred  Deferred 延迟 （就可以设置一些参数）
		AAuraProjectile* Projectile = GetWorld()->SpawnActorDeferred<AAuraProjectile>(ProjectileClass, SpawnTransform,
			GetOwningActorFromActorInfo(), // 参数 Owner
			Cast<APawn>(GetOwningActorFromActorInfo()), //  参数 Instigator 触发者
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn
		);

		// 把伤害信息传递给 弹丸
		Projectile->DamageEffectParams = MakeDamageEffectParamsFromClassDefaults();
		
		Projectile->FinishSpawning(SpawnTransform);
	}
}
