// yyyyyxxxxxx

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystem/Data/CharacterClassInfo.h"
#include "GameFramework/Character.h"
#include "Interacton/CombatInterface.h"
#include "AuraCharacterBase.generated.h"


class UPassiveNiagaraComponent;
class UNiagaraSystem;
class UGameplayAbility;
class UAbilitySystemComponent;
class UAttributeSet;
class UGameplayEffect;
class UAnimMontage;
class UDebuffNiagaraComponent;

// æbstrækt 抽象（就不会UE关卡中被显示使用）
UCLASS(Abstract)
class AURA_API AAuraCharacterBase : public ACharacter, public IAbilitySystemInterface, public ICombatInterface
{
	GENERATED_BODY()
public:
	AAuraCharacterBase();
	virtual void Tick(float DeltaSeconds) override;
	
	UAbilitySystemComponent* GetAbilitySystemComponent() const;
	UAttributeSet* GetAttributeSet() const { return AttributeSet; };

	virtual UAnimMontage* GetHitReactMontage_Implementation() override;


	/* Combat Interface */
	virtual void Die(const FVector& DeathImpulse) override;
	virtual bool IsDead_Implementation() const override;
	virtual AActor* GetAvator_Implementation() override;
	virtual TArray<FTaggedMontage> GetAttackMontags_Implementation() override;
	virtual UNiagaraSystem* GetBloodEffect_Implementation() override;
	virtual FTaggedMontage GetTaggedMontageByTag_Implementation(const FGameplayTag& Montage) override;
	virtual int32 GetMinionCount_Implementation() override;
	virtual void IncremenetMinionCount_Implementation(int32 Amount) override;
	virtual ECharacterClass GetCharacterClass_Implementation() override;
	virtual FOnASCRegistered& GetOnASCRefisteredDelegate() override;
	virtual FOnDeath& GetOnDeathDelegate() override;
	virtual USkeletalMeshComponent* GetWeapon_Implementation() override;
	bool IsBeingShocked_Implementation() const override;
	void SetBeingShocked_Implementation(bool bInShock) override;
	/* Combat Interface */

	FOnASCRegistered OnASCRegistered;
	FOnDeath OnOnDeath;

	
	UFUNCTION(NetMulticast, Reliable)
	virtual void MulticastHandleDeath(const FVector& DeathImpulse);

	// 攻击蒙太奇
	UPROPERTY(EditAnywhere, Category="Combat")
	TArray<FTaggedMontage> AttackMontages;

	// 眩晕
	UPROPERTY(ReplicatedUsing=OnRep_Stunned, BlueprintReadOnly)
	bool bIsStunned = false;
	UFUNCTION()
	virtual void OnRep_Stunned();
	// 灼烧
	UPROPERTY(ReplicatedUsing=OnRep_Burned, BlueprintReadOnly)
	bool bIsBurned = false;
	UFUNCTION()
	virtual void OnRep_Burned();
	// 眩晕循环
	UPROPERTY(Replicated, BlueprintReadOnly)
	bool bInBeingShocked = false;
protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat")
	TObjectPtr<USkeletalMeshComponent> Weapon;

	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat")
	TObjectPtr<UDebuffNiagaraComponent> BurnDebuffComponent;

	// 眩晕特效
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat")
	TObjectPtr<UDebuffNiagaraComponent> StunDebuffComponent;
	
	// 武器释放技能插槽位置
	UPROPERTY(EditAnywhere, Category="Combat")
	FName WeaponTipSocketName;
	UPROPERTY(EditAnywhere, Category="Combat")
	FName LeftHandSocketName;
	UPROPERTY(EditAnywhere, Category="Combat")
	FName RightHandSocketName;
	UPROPERTY(EditAnywhere, Category="Combat")
	FName TailSocketName;
	
	// 基础移动速度
	UPROPERTY(EditDefaultsOnly, Category="Combat")
	float BaseWalkSpeed = 600.f;
	
	bool bDead = false;

	virtual void StunTagChanged(const FGameplayTag CallbackTag, int32 NewCount);

	virtual FVector GetCombatSocketLocation_Implementation(const FGameplayTag& MontageTag) override;

	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;

	virtual void InitAbilityActorInfo();

	// 用于初始化属性的GE
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Attributes")
	TSubclassOf<UGameplayEffect> DefaultPrimaryAttributes;
	
	// 用于初始化次要属性的GE
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Attributes")
	TSubclassOf<UGameplayEffect> DefaultSecondaryAttributes;

	// 用于初始化重要属性的GE
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Attributes")
	TSubclassOf<UGameplayEffect> DefaultVitalAttributes;

	void ApplyEffectToSelf(TSubclassOf<UGameplayEffect> GameplayEffectClass, float Level) const;
	virtual void InitializeDefaultAttributes() const;

	//2 赋予能力
	void AddCharacterAbilities();

	/* 溶解特效 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UMaterialInstance> DissolveMateraialInstance;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UMaterialInstance> WeaponDissolveMateraialInstance;

	// 创建动态材质 替换当前材质
	void Dissolve();

	// 蓝图事件
	UFUNCTION(BlueprintImplementableEvent)
	void StartDissolveTimeLine(UMaterialInstanceDynamic* DynamicMaterialInstance);

	
	// 蓝图事件
	UFUNCTION(BlueprintImplementableEvent)
	void StartWeaponDissolveTimeLine(UMaterialInstanceDynamic* DynamicMaterialInstance);

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UNiagaraSystem* BloodEffect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	USoundBase* DeathSound;

	int32 MinionCount = 0;
	
	// 指定敌人攻击种类
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character Class Defaults")
	ECharacterClass CharacterClass = ECharacterClass::Warrior;

	
private:
	// 1 指定 GA能力蓝图
	UPROPERTY(EditAnywhere, Category="Abilities")
	TArray<TSubclassOf<UGameplayAbility>> StartupAbilities;
	
	// 1 指定 GA 被动 能力蓝图
	UPROPERTY(EditAnywhere, Category="Abilities")
	TArray<TSubclassOf<UGameplayAbility>> StartupPassiveAbilities;

	UPROPERTY(EditAnywhere, Category="Abilities")
	TObjectPtr<UAnimMontage> HitReactMontage;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UPassiveNiagaraComponent> HaloOfProtectionNiagaraComponent;
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UPassiveNiagaraComponent> LifeSiphonNiagaraComponent;
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UPassiveNiagaraComponent> ManaSiphonNiagaraComponent;

	// 上面三个特效组件的 根，这样，这个根可以自由控制旋转而不会影响角色
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> EffectAttachComponent;
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
