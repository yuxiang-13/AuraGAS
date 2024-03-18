#pragma once
#include "GameplayEffect.h"
#include "GameplayEffectTypes.h"
#include "AuraAbilityTypes.generated.h"

USTRUCT(BlueprintType)
struct FDamageEffectParams
{
	GENERATED_BODY()

	FDamageEffectParams(){};

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UObject> WorldContextObject = nullptr;

	UPROPERTY(BlueprintReadWrite)
	TSubclassOf<UGameplayEffect> DamageGameplayEffectClass = nullptr;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UAbilitySystemComponent> SourceAbilitySystemComponent;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UAbilitySystemComponent> TargetAbilitySystemComponent;

	// 基础伤害
	UPROPERTY(BlueprintReadWrite)
	float BaseDamage = 0.f;
	// GA等级
	UPROPERTY(BlueprintReadWrite)
	float AbilityLevel = 1.f;
	// 伤害标签
	UPROPERTY(BlueprintReadWrite)
	FGameplayTag DamageType = FGameplayTag();
	
	UPROPERTY(BlueprintReadWrite)
	float DebuffChance = 0.f; // 减益触发几率
	UPROPERTY(BlueprintReadWrite)
	float DebuffDamage = 0.f; // 减益伤害
	UPROPERTY(BlueprintReadWrite)
	float DebuffFrequency = 0.f; // 减益伤害 每1秒频率
	UPROPERTY(BlueprintReadWrite)
	float DebuffDuration = 0.f; // 减益 持续时间
	
	UPROPERTY(BlueprintReadWrite)
	float DeathImpulseMagnitude = 0.f; // 死亡布娃娃冲击力

	UPROPERTY(BlueprintReadWrite)
	FVector DeathImpulse = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite)  
	float KnockbackForceMagnitude = 0.f;
	
	UPROPERTY(BlueprintReadWrite)
	float KnockbackChance = 0.f;

	UPROPERTY(BlueprintReadWrite)
	FVector KnockbackForce = FVector::ZeroVector;

	// 是否开启范围径向伤害
	UPROPERTY(BlueprintReadWrite)
	bool bIsRadialDamage = false;
	// 内径
	UPROPERTY(BlueprintReadWrite)
	float RadialDamageInnerRadius = 0.f;
	// 外径
	UPROPERTY(BlueprintReadWrite)
	float RadialDamageOuterRadius = 0.f;
	// 辐射原点
	UPROPERTY(BlueprintReadWrite)
	FVector RadialDamageOrigin = FVector::ZeroVector;
};


// **** 扩展 GE_Context 结构体
USTRUCT(BlueprintType)
struct FAuraGameplayEffectContext : public FGameplayEffectContext
{
	GENERATED_BODY()

public:
	bool IsCriticalHit() const { return bIsCriticalHit; }
	bool IsBlockedHit() const { return bIsBlockedHit; }
	
	bool IsSuccessfulDebuff() const;
	float GetDebuffDamage() const { return Debuff_Damage ; }
	float GetDebuffFrequency() const { return Debuff_Frequency ; }
	float GetDebuffDuration() const { return Debuff_Duration ; }
	TSharedPtr<FGameplayTag> GetDamageType() const { return DamageType ; }
	FVector GetDeathImpulse() const { return DeathImpulse ; }
	FVector GetKnockbackForce() const { return KnockbackForce ; }
	
	bool IsRadialDamage() const { return bIsRadialDamage; }
	float GetRadialDamageInnerRadius() const { return RadialDamageInnerRadius ; }
	float GetRadialDamageOuterRadius() const { return RadialDamageOuterRadius ; }
	FVector GetRadialDamageOrigin() const { return RadialDamageOrigin ; }
	

	void SetIsCriticalHit(bool bInIsCriticalHit ) { bIsCriticalHit = bInIsCriticalHit; }
	void SetIsBlockedHit(bool bInIsBlockedHit ) { bIsBlockedHit = bInIsBlockedHit; }
	void SetIsSuccessfulDebuff(bool bInIsDebuff );
	void SetDebuffDamage(float InDamage) { Debuff_Damage = InDamage ; }
	void SetDebuffFrequency(float InFrequency) { Debuff_Frequency = InFrequency ; }
	void SetDebuffDuration(float InDuration) { Debuff_Duration = InDuration ; }
	void SetDamageType(TSharedPtr<FGameplayTag> InDamageType) { DamageType = InDamageType; };
	void SetDeathImpulse(const FVector& InImpulse) { DeathImpulse = InImpulse ; }
	void SetKnockbackForce(const FVector& InKnockbackForce) { KnockbackForce = InKnockbackForce ; }

	void SetIsRadialDamage(bool bInIsRadialDamage) { bIsRadialDamage = bInIsRadialDamage ; }
	void SetRadialDamageInnerRadius(float InRadialDamageInnerRadius) { RadialDamageInnerRadius = InRadialDamageInnerRadius ; }
	void SetRadialDamageOuterRadius(float InRadialDamageOuterRadius) { RadialDamageOuterRadius = InRadialDamageOuterRadius ; }
	void SetRadialDamageOrigin(const FVector& InRadialDamageOrigin) { RadialDamageOrigin = InRadialDamageOrigin ; }
	
	/** Returns the actual struct used for serialization, subclasses must override this! */
	virtual UScriptStruct* GetScriptStruct() const
	{
		return FGameplayEffectContext::StaticStruct();
	}
	/** Creates a copy of this context, used to duplicate for later modifications */
	virtual FGameplayEffectContext* Duplicate() const
	{
		FGameplayEffectContext* NewContext = new FGameplayEffectContext();
		*NewContext = *this;
		if (GetHitResult())
		{
			// Does a deep copy of the hit result
			NewContext->AddHitResult(*GetHitResult(), true);
		}
		return NewContext;
	}

	/** Custom serialization, subclasses must override this */
	virtual bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);
protected:
	// 是否暴击
	UPROPERTY()
	bool bIsCriticalHit = false;
	// 格挡
	UPROPERTY()
	bool bIsBlockedHit = false;

	UPROPERTY()
	bool bIsSuccessfulDebuff = false;
	UPROPERTY()
	float Debuff_Damage = 0.f; // 减益伤害
	UPROPERTY()
	float Debuff_Frequency = 0.f; // 减益伤害 每1秒频率
	UPROPERTY()
	float Debuff_Duration = 0.f; // 减益 持续时间

	// 共享指针自己处理垃圾回收，不用UPROPERTY()保护
	TSharedPtr<FGameplayTag> DamageType;

	UPROPERTY()
	FVector DeathImpulse = FVector::ZeroVector;
	
	UPROPERTY()
	FVector KnockbackForce = FVector::ZeroVector;
	
	// 是否开启范围径向伤害
	UPROPERTY()
	bool bIsRadialDamage = false;
	// 内径
	UPROPERTY()
	float RadialDamageInnerRadius = 0.f;
	// 外径
	UPROPERTY()
	float RadialDamageOuterRadius = 0.f;
	// 辐射原点
	UPROPERTY()
	FVector RadialDamageOrigin = FVector::ZeroVector;
};

// 序列化模板
template<>
struct TStructOpsTypeTraits< FAuraGameplayEffectContext > : public TStructOpsTypeTraitsBase2< FAuraGameplayEffectContext >
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true		// Necessary so that TSharedPtr<FHitResult> Data is copied around
	};
};
