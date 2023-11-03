// yyyyyxxxxxx


#include "AbilitySystem/Abilities/AuraProjectileSpell.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AuraGameplayTags.h"
#include "Actor/AuraProjectile.h"
#include "Interacton/CombatInterface.h"
#include "Kismet/KismetSystemLibrary.h"

void UAuraProjectileSpell::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                           const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                           const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	// UKismetSystemLibrary::PrintString(this, FString("ActivateAbility (c++)"), true, true, FLinearColor::Red, 3);

}

void UAuraProjectileSpell::SpawnProjectile(const FVector& ProjectileTargetLocation)
{
	const bool bIsServer = GetAvatarActorFromActorInfo()->HasAuthority();
	if (!bIsServer) return;

	// 获取接口位置
	ICombatInterface* CombatInterface = Cast<ICombatInterface>(GetAvatarActorFromActorInfo());
	if (CombatInterface)
	{
		const FVector SocketLocation = CombatInterface->GetCombatSocketLocation();
		// 1 A-B = B看向A  获取武器插槽位置到目标点的 向量的旋转
		FRotator Rotatior = (ProjectileTargetLocation - SocketLocation).Rotation();
		// (抬头低头)因为怪比较小，权杖高，肯定就向下发射了，改成水平的
		Rotatior.Pitch = 0.f;

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
		// TODO: 给导弹 添加 GE Spec 造成伤害
		const  UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetAvatarActorFromActorInfo());
		const  FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(), SourceASC->MakeEffectContext());


		
		// 获取伤害标签
		const FAuraGameplayTags GameplayTags = FAuraGameplayTags::Get();
		// 获取伤害（根据曲线）
		const float ScaleDamage = Damage.GetValueAtLevel(GetAbilityLevel());
		// GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString::Printf(TEXT("  FireBolt Damage : %f"), ScaleDamage));
		
		// 分配 Tag SetByCaller
		// 您可以实现具有动态标签逻辑的游戏行为，而无需硬编码固定的标签分配条件
		UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Damage, ScaleDamage);



		
		// 指定积累导弹的 SpecHandle
		Projectile->DamageEffectSpecHandle = SpecHandle;
		
		Projectile->FinishSpawning(SpawnTransform);
	}
}
