// yyyyyxxxxxx

#pragma once

#include "CoreMinimal.h"
#include "AuraAbilityTypes.h"
#include "AbilitySystem/Abilities/AuraGameplayAbility.h"
#include "AuraDamageGameplayAbility.generated.h"

struct FTaggedMontage;
/**
 * 
 */
UCLASS()
class AURA_API UAuraDamageGameplayAbility : public UAuraGameplayAbility
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	void CauseDamage(AActor* TargetActor);

	UFUNCTION(BlueprintPure)
	FDamageEffectParams MakeDamageEffectParamsFromClassDefaults(
		AActor* TargetActor = nullptr,
		FVector InRadialDamageOrigin = FVector::ZeroVector,
		bool bOverrideKnockbackDirection = false, // 覆盖击退方向
		FVector KnockbackDirectionOveride = FVector::ZeroVector, // 击退方向
		bool bOverrideDeathImpluse = false, // 死亡冲击力
		FVector DeathImpluseDirectionOveride = FVector::ZeroVector, // 击退方向
		bool bOverridePitch = false,
		float PitchOverride = 0.f
	) const;

	UFUNCTION(BlueprintPure)
	float GetDamageAtLevel() const;
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	// 伤害类型Map---> 不同伤害曲线
	UPROPERTY(EditDefaultsOnly, Category="Damage")
	FGameplayTag DamageType;
	UPROPERTY(EditDefaultsOnly, Category="Damage")
	FScalableFloat Damage;

	UPROPERTY(EditDefaultsOnly, Category="Damage")
	float DebuffChance = 20.f; // 减益触发几率
	UPROPERTY(EditDefaultsOnly, Category="Damage")
	float DebuffDamage = 5.f; // 减益伤害
	UPROPERTY(EditDefaultsOnly, Category="Damage")
	float DebuffFrequency = 1.f; // 减益伤害 每1秒频率
	UPROPERTY(EditDefaultsOnly, Category="Damage")
	float DebuffDuration = 5.f; // 减益 持续时间
	
	UPROPERTY(EditDefaultsOnly, Category="Damage")
	float DeathImpulseMagnitude = 1000.f; // 死亡布娃娃冲击力
	
	UPROPERTY(EditDefaultsOnly, Category="Damage")
	float KnockbackImpulseMagnitude = 1000.f;
	
	UPROPERTY(EditDefaultsOnly, Category="Damage")
	float KnockbackChance = 0.f;

	// 是否开启范围径向伤害
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Damage")
	bool bIsRadialDamage = false;
	// 内径
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Damage")
	float RadialDamageInnerRadius = 0.f;
	// 外径
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Damage")
	float RadialDamageOuterRadius = 0.f;
	
	UFUNCTION(BlueprintPure)
	FTaggedMontage GetRandomTaggedMontageFromArray(const TArray<FTaggedMontage>& TaggedMontages);

};
