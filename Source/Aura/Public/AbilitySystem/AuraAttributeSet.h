// yyyyyxxxxxx

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "AuraAttributeSet.generated.h"



// set get init 属性宏 属性访问者
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)


USTRUCT()
struct FEffectProperties
{
	GENERATED_BODY()

	FEffectProperties() {}

	FGameplayEffectContextHandle EffectContextHandle;

	// Source
	UPROPERTY()
	UAbilitySystemComponent* SourceASC = nullptr;
	UPROPERTY()
	AActor* SourceAvatarActor  = nullptr;
	UPROPERTY()
	AController* SourceController = nullptr;
	UPROPERTY()
	ACharacter* SourceCharacter = nullptr;
	// Target
	UPROPERTY()
	UAbilitySystemComponent* TargetASC = nullptr;
	UPROPERTY()
	AActor* TargetAvatarActor  = nullptr;
	UPROPERTY()
	AController* TargetController = nullptr;
	UPROPERTY()
	ACharacter* TargetCharacter = nullptr;
};

#define ATTR_Delegate(AttrName); \
FAttributeSignature AttrName##Delegate; \
AttrName##Delegate.BindStatic(UAuraAttributeSet::Get##AttrName##Attribute); \
TagsToAttributes.Add(GameplayTags.Attributes_Primary_##AttrName, AttrName##Delegate);

// 定义Type
// 注意，FGameplayAttribute()带括号，1  FGameplayAttribute 是返回类型，2 ()里面是函数参数
typedef TBaseStaticDelegateInstance<FGameplayAttribute(), FDefaultDelegateUserPolicy>::FFuncPtr AttributeFunPtr;

// ************************ 绑定 函数指针
//1 using 创建Type别名，函数别名
//2 xxx ::FFuncPtr 属于依赖类型，需要前面加用typename ，告诉引擎是一个类型
template<class T>
using TStaticFuncPtr = typename TBaseStaticDelegateInstance<T, FDefaultDelegateUserPolicy>::FFuncPtr;

UCLASS()
class AURA_API UAuraAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
public:
	UAuraAttributeSet();

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	
	TBaseStaticDelegateInstance<FGameplayAttribute(), FDefaultDelegateUserPolicy>::FFuncPtr xxx;

	// 是模板。所以 < FGameplayAttribute() >
	TMap<FGameplayTag,  TStaticFuncPtr<FGameplayAttribute()>> TagsToAttributes;

	/*
	 *  Primary Attributes  主属性
	 */
//--------------------- a 力量-----------------------------
	UPROPERTY(ReplicatedUsing = OnRep_Strength, BlueprintReadOnly, Category=" Primary Attributes ")
	FGameplayAttributeData Strength;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Strength)
	UFUNCTION()
	void OnRep_Strength(const FGameplayAttributeData& OldStrength) const;


//--------------------- b 智力-----------------------------
	UPROPERTY(ReplicatedUsing = OnRep_Intelligence, BlueprintReadOnly, Category=" Primary Attributes ")
	FGameplayAttributeData Intelligence;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Intelligence)
	UFUNCTION()
	void OnRep_Intelligence(const FGameplayAttributeData& OldIntelligence) const;

//--------------------- c 恢复-----------------------------
	UPROPERTY(ReplicatedUsing = OnRep_Resilience, BlueprintReadOnly, Category=" Primary Attributes ")
	FGameplayAttributeData Resilience;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Resilience)
	UFUNCTION()
	void OnRep_Resilience(const FGameplayAttributeData& OldResilience) const;

//--------------------- d 活力-----------------------------
	UPROPERTY(ReplicatedUsing = OnRep_Vigor, BlueprintReadOnly, Category=" Primary Attributes ")
	FGameplayAttributeData Vigor;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Vigor)
	UFUNCTION()
	void OnRep_Vigor(const FGameplayAttributeData& OldVigor) const;
	
	/*
	 *  Vital Attributes  基本属性
	 */
//---------------------1-----------------------------
	// 属性 网络同步 ReplicatedUsing
	UPROPERTY(ReplicatedUsing = OnRep_Health, BlueprintReadOnly, Category=" Vital Attributes ")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Health)

	// 参数是上一次的旧值
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldHealth)  const;

//---------------------2-----------------------------
	UPROPERTY(ReplicatedUsing=OnRep_MaxHealth, BlueprintReadOnly, Category=" Vital Attributes ")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, MaxHealth)

	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const;
	
//---------------------3-----------------------------
	UPROPERTY(ReplicatedUsing=OnRep_Mana, BlueprintReadOnly, Category=" Vital Attributes ")
	FGameplayAttributeData Mana;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Mana)

	UFUNCTION()
	void OnRep_Mana(const FGameplayAttributeData& OldMana) const;

//---------------------4-----------------------------
	UPROPERTY(ReplicatedUsing=OnRep_MaxMana, BlueprintReadOnly, Category=" Vital Attributes ")
	FGameplayAttributeData MaxMana;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, MaxMana)

	UFUNCTION()
	void OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana) const;

	/*
	 *  Secondary Attributes  次要属性
	*/
//---------------------护甲-----------------------------
	UPROPERTY(ReplicatedUsing = OnRep_Armor, BlueprintReadOnly, Category=" Secondary Attributes ")
	FGameplayAttributeData Armor;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Armor)
	UFUNCTION()
	void OnRep_Armor(const FGameplayAttributeData& OldArmor) const;
	
	
//---------------------护甲穿透-----------------------------
	UPROPERTY(ReplicatedUsing = OnRep_ArmorPenetration, BlueprintReadOnly, Category=" Secondary Attributes ")
	FGameplayAttributeData ArmorPenetration;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, ArmorPenetration)
	UFUNCTION()
	void OnRep_ArmorPenetration(const FGameplayAttributeData& OldArmorPenetration) const;

	
//---------------------格挡-----------------------------
	UPROPERTY(ReplicatedUsing = OnRep_BlockChance, BlueprintReadOnly, Category=" Secondary Attributes ")
	FGameplayAttributeData BlockChance;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, BlockChance)
	UFUNCTION()
	void OnRep_BlockChance(const FGameplayAttributeData& OldBlockChance) const;

	
//---------------------暴击率-----------------------------
	UPROPERTY(ReplicatedUsing = OnRep_CriticalHitChance, BlueprintReadOnly, Category=" Secondary Attributes ")
	FGameplayAttributeData CriticalHitChance;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, CriticalHitChance)
	UFUNCTION()
	void OnRep_CriticalHitChance(const FGameplayAttributeData& OldCriticalHitChance) const;


	
//---------------------暴击伤害-----------------------------
	UPROPERTY(ReplicatedUsing = OnRep_CriticalHitDamage, BlueprintReadOnly, Category=" Secondary Attributes ")
	FGameplayAttributeData CriticalHitDamage;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, CriticalHitDamage)
	UFUNCTION()
	void OnRep_CriticalHitDamage(const FGameplayAttributeData& OldCriticalHitDamage) const;

	
//---------------------暴击抗性-----------------------------
	UPROPERTY(ReplicatedUsing = OnRep_CriticalHitResistance, BlueprintReadOnly, Category=" Secondary Attributes ")
	FGameplayAttributeData CriticalHitResistance;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, CriticalHitResistance)
	UFUNCTION()
	void OnRep_CriticalHitResistance(const FGameplayAttributeData& OldCriticalHitResistance) const;


//---------------------生命恢复-----------------------------
	UPROPERTY(ReplicatedUsing = OnRep_HealthRegeneration, BlueprintReadOnly, Category=" Secondary Attributes ")
	FGameplayAttributeData HealthRegeneration;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, HealthRegeneration)
	UFUNCTION()
	void OnRep_HealthRegeneration(const FGameplayAttributeData& OldHealthRegeneration) const;



//---------------------魔法恢复-----------------------------
	UPROPERTY(ReplicatedUsing = OnRep_ManaRegeneration, BlueprintReadOnly, Category=" Secondary Attributes ")
	FGameplayAttributeData ManaRegeneration;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, ManaRegeneration)
	UFUNCTION()
	void OnRep_ManaRegeneration(const FGameplayAttributeData& OldManaRegeneration) const;

/*
 *  Resistance Attributes  伤害抗性
*/
//---------------------火伤害抗性-----------------------------
	UPROPERTY(ReplicatedUsing = OnRep_FireResistance, BlueprintReadOnly, Category=" Resistance Attributes ")
	FGameplayAttributeData FireResistance;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, FireResistance)
	UFUNCTION()
	void OnRep_FireResistance(const FGameplayAttributeData& OldFireResistance) const;
//---------------------雷电伤害抗性-----------------------------
	UPROPERTY(ReplicatedUsing = OnRep_LightningResistance, BlueprintReadOnly, Category=" Resistance Attributes ")
	FGameplayAttributeData LightningResistance;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, LightningResistance)
	UFUNCTION()
	void OnRep_LightningResistance(const FGameplayAttributeData& OldLightningResistance) const;
//---------------------魔法伤害抗性-----------------------------
	UPROPERTY(ReplicatedUsing = OnRep_ArcaneResistance, BlueprintReadOnly, Category=" Resistance Attributes ")
	FGameplayAttributeData ArcaneResistance;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, ArcaneResistance)
	UFUNCTION()
	void OnRep_ArcaneResistance(const FGameplayAttributeData& OldArcaneResistance) const;
//---------------------物理伤害抗性-----------------------------
	UPROPERTY(ReplicatedUsing = OnRep_PhysicalResistance, BlueprintReadOnly, Category=" Resistance Attributes ")
	FGameplayAttributeData PhysicalResistance;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, PhysicalResistance)
	UFUNCTION()
	void OnRep_PhysicalResistance(const FGameplayAttributeData& OldPhysicalResistance) const;


	
/*
 *  Meta Attributes  元属性
 *  只存在于服务器上，不参与复制
 */
	//---------------------元属性-----------------------------
	UPROPERTY(BlueprintReadOnly, Category=" Meta Attributes ")
	FGameplayAttributeData IncomingDamage;  // Incoming=接收
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, IncomingDamage)

	UPROPERTY(BlueprintReadOnly, Category=" Meta Attributes ")
	FGameplayAttributeData IncomingXP;  // Incoming=接收
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, IncomingXP)

	
	
private:
	void SetEffectProperties(const FGameplayEffectModCallbackData& Data, FEffectProperties& Props) const;
	void ShowFloatingText(const FEffectProperties& Props, float Damage, bool bBlockedHit, bool bCriticalHit) const;
	void SendXPEvent(const FEffectProperties& Props) const;
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
