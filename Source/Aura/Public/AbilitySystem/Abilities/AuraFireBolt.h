// yyyyyxxxxxx

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/AuraProjectileSpell.h"
#include "AuraFireBolt.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API UAuraFireBolt : public UAuraProjectileSpell
{
	GENERATED_BODY()
public:
	virtual FString GetDescription(int32 Level) override;
	virtual FString GetNextLevelDescription(int32 Level) override;

	UFUNCTION(BlueprintCallable)
	void SpawnProjectiles(const FVector& ProjectileTargetLocation, const FGameplayTag& SocketTag, bool bOverridePitch = false, float PitchOverride = 0.f, AActor* HomingTarget = nullptr);

protected:
	// 弹丸扇形扩散 角
	UPROPERTY(EditDefaultsOnly, Category="FireBolt")	
	float ProjectileSpread = 90.f;

	// 弹丸数
	UPROPERTY(EditDefaultsOnly, Category="FireBolt")	
	int32 MaxNumProjectiles = 5;

	// 巡航最小加速度
	UPROPERTY(EditDefaultsOnly, Category="FireBolt")	
	float HomingAccelerationMin = 1600.f;

	// 巡航最大加速度
	UPROPERTY(EditDefaultsOnly, Category="FireBolt")	
	float HomingAccelerationMax = 3200.f;

	// 是否开启巡航导弹
	UPROPERTY(EditDefaultsOnly, Category="FireBolt")
	bool bLaunchHomingProjectile = true;
	
};
