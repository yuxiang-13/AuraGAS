// yyyyyxxxxxx

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PlayerInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UPlayerInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class AURA_API IPlayerInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent)
	int32 FindLevelForXP(int32 InXP);
	
	UFUNCTION(BlueprintNativeEvent)
	int32 GetAttributePointsReward(int32 Level);
	
	UFUNCTION(BlueprintNativeEvent)
	int32 GetSpellPointsReward(int32 Level);

	UFUNCTION(BlueprintNativeEvent)
	void AddToAttributePoints(int32 InAttributePoints);

	UFUNCTION(BlueprintNativeEvent)
	void AddToPlayerLevel(int32 InPlayerLevel);

	UFUNCTION(BlueprintNativeEvent)
	void AddToSpellPoints(int32 InSPellPoints);

	
	UFUNCTION(BlueprintNativeEvent)
	int32 GetXP();
	UFUNCTION(BlueprintNativeEvent)
	void AddToXP(int32 InXP);
	UFUNCTION(BlueprintNativeEvent)
	void LevelUp();
};
