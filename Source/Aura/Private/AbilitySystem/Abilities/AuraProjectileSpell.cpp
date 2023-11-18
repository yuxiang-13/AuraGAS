// yyyyyxxxxxx


#include "AbilitySystem/Abilities/AuraProjectileSpell.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AuraGameplayTags.h"
#include "Actor/AuraProjectile.h"
#include "Interacton/CombatInterface.h"

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
		// 1 下面这个报错
		// const FVector SocketLocation = CombatInterface->GetCombatSocketLocation();
		// 2 参数代表着强转的类
		const FVector SocketLocation = ICombatInterface::Execute_GetCombatSocketLocation(
			GetAvatarActorFromActorInfo(),
			FAuraGameplayTags::Get().Montage_Attack_Weapon
		);
		
		
		// 1 A-B = B看向A  获取武器插槽位置到目标点的 向量的旋转
		FRotator Rotatior = (ProjectileTargetLocation - SocketLocation).Rotation();

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

		
		//  SourceASC->MakeEffectContext()
		//  这种就会默认设置 EffectCauser 和 Instigator相关
		FGameplayEffectContextHandle EffectContextHandle = SourceASC->MakeEffectContext();
		
		EffectContextHandle.SetAbility(this);
		
		EffectContextHandle.AddSourceObject(Projectile);
		
		TArray<TWeakObjectPtr<AActor>> Actors;
		Actors.Add(Projectile);
		EffectContextHandle.AddActors(Actors);

		FHitResult HitResult;
		HitResult.Location = ProjectileTargetLocation;
		EffectContextHandle.AddHitResult(HitResult);
		
		
		const  FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(), EffectContextHandle);

		// 查看伤害类型  给定伤害值
		for (auto & Pair : DamageTypes)
		{
			const float ScaledDamage = Pair.Value.GetValueAtLevel(GetAbilityLevel());
			// 分配 Tag SetByCaller
			// 您可以实现具有动态标签逻辑的游戏行为，而无需硬编码固定的标签分配条件
			UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, Pair.Key, ScaledDamage);
		}
		
		
		// 指定积累导弹的 SpecHandle
		Projectile->DamageEffectSpecHandle = SpecHandle;
		
		Projectile->FinishSpawning(SpawnTransform);
	}
}
