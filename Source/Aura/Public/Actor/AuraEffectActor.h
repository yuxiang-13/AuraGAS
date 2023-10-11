// yyyyyxxxxxx

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h"
#include "AuraEffectActor.generated.h"

class UAbilitySystemComponent;
struct FActiveGameplayEffectHandle;
class UGameplayEffect;

UENUM(BlueprintType)
enum class EEffectApplicationPolicy
{
	ApplyOnOverlap, // 碰撞时应用
	ApplyOnEndOverlap, // 碰撞结束时应用
	DoNotApply // 不触发应用
};

// 移除政策，只适用于无限GE,因为即时和持续GE都会自动消失
UENUM(BlueprintType)
enum class EEffectRemovalPolicy
{
	RemoveOnEndOverlap, // GE不碰撞接触时删除
	DoNotRemove // 永不删除
};

UCLASS()
class AURA_API AAuraEffectActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AAuraEffectActor();
	
protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	void ApplyEffectToTarget(AActor* Target, TSubclassOf<UGameplayEffect> GameplayEffectClass);
	
	UFUNCTION(BlueprintCallable)
	void OnOverlap(AActor* TargetActor);
	UFUNCTION(BlueprintCallable)
	void OnEndOverlap(AActor* TargetActor);
	
	
protected:
	// Instant 及时
	UPROPERTY(EditAnywhere, BlueprintReadOnly , Category= "Applied Effects")
	TSubclassOf<UGameplayEffect> InstantGameplayEffectClass;
	// GE应用策略---> 瞬时GE (没有策略)
	UPROPERTY(EditAnywhere, BlueprintReadOnly , Category= "Applied Effects")
	EEffectApplicationPolicy InstantEffectApplicationPolicy = EEffectApplicationPolicy::DoNotApply;

	
	// 持续一段时间 及时
	UPROPERTY(EditAnywhere, BlueprintReadOnly , Category= "Applied Effects")
	TSubclassOf<UGameplayEffect> DurationGameplayEffectClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly , Category= "Applied Effects")
	EEffectApplicationPolicy DurationEffectApplicationPolicy = EEffectApplicationPolicy::DoNotApply;;

	
	// 永久 及时
	UPROPERTY(EditAnywhere, BlueprintReadOnly , Category= "Applied Effects")
	TSubclassOf<UGameplayEffect> InfiniteGameplayEffectClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly , Category= "Applied Effects")
	EEffectApplicationPolicy InfiniteEffectApplicationPolicy = EEffectApplicationPolicy::DoNotApply;;
	// 只有这个永久GE --->  清除时机（结束碰撞）
	UPROPERTY(EditAnywhere, BlueprintReadOnly , Category= "Applied Effects")
	EEffectRemovalPolicy InfiniteEffectRemovalPolicy = EEffectRemovalPolicy::RemoveOnEndOverlap;

private:
	// 存储永久策略GE的Map
	TMap<FActiveGameplayEffectHandle, UAbilitySystemComponent*> ActiveEffectHandles;
};
