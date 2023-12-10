// yyyyyxxxxxx

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "AbilityInfo.generated.h"


class UGameplayAbility;

USTRUCT(BlueprintType)
struct FAuraAbilityInfo
{
	GENERATED_BODY()
	// 能力
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag AbilityTag = FGameplayTag();
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag InputTag = FGameplayTag();
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag StatusTag = FGameplayTag();
	// 冷却标签
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag CooldownTag = FGameplayTag();
	// 图标
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<const UTexture2D> Icon = nullptr;
	// 材质
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<const UMaterialInterface> BackgroundMaterial = nullptr;
	
	// 级别要求
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 LevelRequirement = 1;

	// 赋予能力GA class
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayAbility> Ability;
};

/**
 * 
 */
UCLASS()
class AURA_API UAbilityInfo : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="AbilityInformation")
	TArray<FAuraAbilityInfo> AbilityInformation;
	
	FAuraAbilityInfo FindAbilityInfoForTag(const FGameplayTag& AbilityTag, bool bLogNotFound = false) const;
};
