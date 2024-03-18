// yyyyyxxxxxx


#include "AbilitySystem/Abilities/AuraDamageGameplayAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AuraGameplayTags.h"
#include "Interacton/CombatInterface.h"

void UAuraDamageGameplayAbility::CauseDamage(AActor* TargetActor)
{
	// 创建GEHandle
	FGameplayEffectSpecHandle DamageSpecHandle = MakeOutgoingGameplayEffectSpec(DamageEffectClass, 1.f);
	
	const float ScaleDamage = Damage.GetValueAtLevel(GetAbilityLevel());
	// Spec->SetSetByCallerMagnitude(DataTag, Magnitude); 分配Tag具体伤害
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(DamageSpecHandle, DamageType, ScaleDamage);

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	GetAbilitySystemComponentFromActorInfo()->ApplyGameplayEffectSpecToTarget(*DamageSpecHandle.Data.Get(), TargetASC);
}

FDamageEffectParams UAuraDamageGameplayAbility::MakeDamageEffectParamsFromClassDefaults(AActor* TargetActor,
	FVector InRadialDamageOrigin, bool bOverrideKnockbackDirection, FVector KnockbackDirectionOveride,
	bool bOverrideDeathImpluse, FVector DeathImpluseDirectionOveride, bool bOverridePitch,
	float PitchOverride) const
{
	FDamageEffectParams Params;
	Params.WorldContextObject = GetAvatarActorFromActorInfo();
	Params.DamageGameplayEffectClass = DamageEffectClass;
	Params.SourceAbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	Params.TargetAbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);

	// 获取曲线伤害
	Params.BaseDamage = Damage.GetValueAtLevel(GetAbilityLevel());
	Params.AbilityLevel = GetAbilityLevel();
	Params.DamageType = DamageType;
	
	Params.DebuffChance = DebuffChance; // 减益触发几率
	Params.DebuffDamage = DebuffDamage; // 减益伤害
	Params.DebuffFrequency = DebuffFrequency; // 减益伤害 每1秒频率
	Params.DebuffDuration = DebuffDuration; // 减益 持续时间

	Params.DeathImpulseMagnitude = DeathImpulseMagnitude;
	Params.KnockbackForceMagnitude = KnockbackImpulseMagnitude;
	Params.KnockbackChance = KnockbackChance;

	if (IsValid(TargetActor))
	{
		FRotator Rotation = (TargetActor->GetActorLocation() - GetAvatarActorFromActorInfo()->GetActorLocation()).Rotation();
		if (bOverridePitch)
		{
			Rotation.Pitch = PitchOverride;
		}
		const FVector ToTarget = Rotation.Vector();

		if (!bOverrideKnockbackDirection)
		{
			Params.KnockbackForce = ToTarget * KnockbackImpulseMagnitude;
		}
		if (!bOverrideDeathImpluse)
		{
			Params.KnockbackForce = ToTarget * DeathImpulseMagnitude;
		}
	}
	
	
	
	// 覆盖击退方向
	if (bOverrideKnockbackDirection)
	{
		KnockbackDirectionOveride.Normalize();
		// 击飞方向
		Params.KnockbackForce = KnockbackDirectionOveride * KnockbackImpulseMagnitude;
		
		if (bOverridePitch)
		{
			// 击退方向的 旋转
			FRotator KnockbackRotation = KnockbackDirectionOveride.Rotation();
			KnockbackRotation.Pitch = PitchOverride;
			// 击飞方向
			Params.KnockbackForce = KnockbackRotation.Vector() * KnockbackImpulseMagnitude;
		}
	}

	// 击退方向
	if (bOverrideDeathImpluse)
	{
		DeathImpluseDirectionOveride.Normalize();
		Params.DeathImpulse = DeathImpluseDirectionOveride * DeathImpulseMagnitude;
		if (bOverridePitch)
		{
			FRotator DeathImpulseRotation = DeathImpluseDirectionOveride.Rotation();
			DeathImpulseRotation.Pitch = PitchOverride;
			Params.DeathImpulse = DeathImpulseRotation.Vector() * DeathImpulseMagnitude;
		}
	}
	
	// 存在径向伤害
	if (bIsRadialDamage)
	{
		Params.bIsRadialDamage = bIsRadialDamage;
		Params.RadialDamageInnerRadius = RadialDamageInnerRadius;
		Params.RadialDamageOuterRadius = RadialDamageOuterRadius;
		Params.RadialDamageOrigin = InRadialDamageOrigin;
	}
	return Params;
}

float UAuraDamageGameplayAbility::GetDamageAtLevel() const
{
	return Damage.GetValueAtLevel(GetAbilityLevel());
}

FTaggedMontage UAuraDamageGameplayAbility::GetRandomTaggedMontageFromArray(const TArray<FTaggedMontage>& TaggedMontages)
{
	if (TaggedMontages.Num() > 0)
	{
		const int32 Selection = FMath::RandRange(0, TaggedMontages.Num() - 1);
		return TaggedMontages[Selection];
	}
	return FTaggedMontage();
}
