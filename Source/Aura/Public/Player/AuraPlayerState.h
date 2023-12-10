// yyyyyxxxxxx

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "AuraPlayerState.generated.h"

class ULevelUpInfo;
class UAbilitySystemComponent;
class UAttributeSet;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerStateChanged, int32 /*StateValue*/)
/**
 * 
 */
UCLASS()
class AURA_API AAuraPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()
public:
	AAuraPlayerState();
	UAbilitySystemComponent* GetAbilitySystemComponent() const;
	UAttributeSet* GetAttributeSet() const { return AttributeSet; };

	// 升级数据资产
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<ULevelUpInfo> LevelInfo;

	FORCEINLINE int32 GetLevel() const { return Level; };
	void SetLevel(int32 InLevel);
	void AddLevel(int32 InLevel);
	FOnPlayerStateChanged OnLevelChangedDelegate;

	FORCEINLINE int32 GetXP() const { return XP; };
	void AddToXP(int32 InXP);
	void SetXP(int32 InXP);
	FOnPlayerStateChanged OnXPChangedDelegate;
	
	FORCEINLINE int32 GetAttributePoints() const { return AttributePoints; };
	void AddToAttributePoints(int32 InAttributePoints);
	void SetAttributePoints(int32 InAttributePoints);
	FOnPlayerStateChanged OnAttributePointsChangedDelegate;
	
	FORCEINLINE int32 GetSpellPoints() const { return SpellPoints; };
	void AddToSpellPoints(int32 InSpellPoints);
	void SetSpellPoints(int32 InSpellPoints);
	FOnPlayerStateChanged OnSpellPointsChangedDelegate;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;

private:
	UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_Level)
	int32 Level = 1;
	UFUNCTION()
	void OnRep_Level(int32 OldLevel);

	UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_XP)
	int32 XP = 1;
	UFUNCTION()
	void OnRep_XP(int32 OldXP);
	
	UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_AttributePoints)
	int32 AttributePoints = 0;
	UFUNCTION()
	void OnRep_AttributePoints(int32 OldAttributePoints);

	UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_SpellPoints)
	int32 SpellPoints = 0;
	UFUNCTION()
	void OnRep_SpellPoints(int32 OldSpellPoints);
};
