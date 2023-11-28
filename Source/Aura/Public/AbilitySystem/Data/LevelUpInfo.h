// yyyyyxxxxxx

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LevelUpInfo.generated.h"

USTRUCT(BlueprintType)
struct FAuraLevelUpInfo
{
	GENERATED_BODY()

	// 升级要求
	UPROPERTY(EditDefaultsOnly)
	int32 LevelUpRequirement = 0;

	// 奖励点
	UPROPERTY(EditDefaultsOnly)
	int32 AttributePointAward = 1;

	// 惊喜奖励？
	UPROPERTY(EditDefaultsOnly)
	int32 SpellPointAward = 1;
};


UCLASS()
class AURA_API ULevelUpInfo : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly)
	TArray<FAuraLevelUpInfo> LevelUpInformation;

	int32 FindLevelForXP(int32 XP) const;
};
