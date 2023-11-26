// yyyyyxxxxxx


#include "AbilitySystem/ExecCalc/ExecCalc_Damage.h"
#include "AbilitySystemComponent.h"
#include "AuraAbilityTypes.h"
#include "AuraGameplayTags.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AbilitySystem/Data/CharacterClassInfo.h"
#include "Interacton/CombatInterface.h"
#include "Kismet/KismetSystemLibrary.h"

// 1 原C++ 结构体。没必要加F 因为是原生c++
struct AuraDamageStatics
{
	// ① 使用 （声明属性捕获）宏定义
	// 这个宏，作用是创建属性捕获变量，P只是变量名
	DECLARE_ATTRIBUTE_CAPTUREDEF(Armor) // Armor 护甲
	DECLARE_ATTRIBUTE_CAPTUREDEF(ArmorPenetration) // Armor 护甲穿透
	DECLARE_ATTRIBUTE_CAPTUREDEF(BlockChance) // 格挡几率
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitChance) // 爆率
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitResistance) // 爆抗
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitDamage) // 爆击伤害
	
	DECLARE_ATTRIBUTE_CAPTUREDEF(FireResistance) // 火抗性
	DECLARE_ATTRIBUTE_CAPTUREDEF(LightningResistance) // 雷电抗性
	DECLARE_ATTRIBUTE_CAPTUREDEF(ArcaneResistance) // 魔法抗性
	DECLARE_ATTRIBUTE_CAPTUREDEF(PhysicalResistance) // 物理抗性

	// 创建关于捕获属性的Map
	// 游戏标签---捕获 的 Map
	TMap<FGameplayTag, FGameplayEffectAttributeCaptureDefinition> TagsToCaptureDefs;
	
	AuraDamageStatics()
	{
		// ② 定义 （定义属性捕获）宏定义
		// 第三个参数 EGameplayEffectAttributeCaptureSource::T  只包含两种类型，Target和Source,
		/** Source (caster) of the gameplay effect.  【激活GE】  */ 
		//Source,	
		/** Target (recipient) of the gameplay effect. 【受害者GE】*/
		//Target
		// 我们现在是 捕获护甲，去计算最终伤害，伤害谁？？---> 那肯定是 Target 是受到伤害的

		// ③ false 是 InSnapshot 快照
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, Armor, Target, false)
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, BlockChance, Target, false)
		// 护甲穿透是攻击者的属性，护甲和格挡是受击者 属性
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, ArmorPenetration, Source, false)
		// 攻击者 属性
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, CriticalHitChance, Source, false)
		// 受击者 属性
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, CriticalHitResistance, Target, false)
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, CriticalHitDamage, Target, false)

		
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, FireResistance, Target, false)
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, LightningResistance, Target, false)
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, ArcaneResistance, Target, false)
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, PhysicalResistance, Target, false)

		// 添加Map
		const FAuraGameplayTags& Tags = FAuraGameplayTags::Get();
		TagsToCaptureDefs.Add(Tags.Attributes_Secondary_Armor, ArmorDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Secondary_ArmorPenetration, ArmorPenetrationDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Secondary_BlockChance, BlockChanceDef);

		TagsToCaptureDefs.Add(Tags.Attributes_Secondary_CriticalHitChance, CriticalHitChanceDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Secondary_CriticalHitResistance, CriticalHitResistanceDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Secondary_CriticalHitDamage, CriticalHitDamageDef);

		TagsToCaptureDefs.Add(Tags.Attributes_Resistance_Fire, FireResistanceDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Resistance_Lightning, LightningResistanceDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Resistance_Arcane, ArcaneResistanceDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Resistance_Physical, PhysicalResistanceDef);
	}
};

// 2 声明静态函数，对外返回 结构体引用
static const AuraDamageStatics& DamageStatics()
{
	// 3 创建 静态结构体类
	/*
	 * 类似 声明一个变量
	 * int a;
	 * float a;
	 * MySpecialStruct S；
	 */
	static AuraDamageStatics DStatics;

	// ******* 当在静态函数内部 创建静态变量时，每次调用该函数，都会返回同一个
	// 静态对象，在整个静态对象存储期间，哪怕函数结束，他也不会消失，即使
	// 他没有作为指针动态new分配在堆上 , 但他就是静态，会继续存在
	// 所以哪怕 我们一遍又一遍调用这个函数，也只会返回 相同且唯一的 DStatics 静态对象
	return DStatics;

	// 这样就保证了，会一直存储我们的结果，并且每次都会得到相同的结果
}

UExecCalc_Damage::UExecCalc_Damage()
{
	// 底层写好的 捕获相关属性Map
	// 2-1 将捕获定义放到Map中
	RelevantAttributesToCapture.Add(DamageStatics().ArmorDef);
	RelevantAttributesToCapture.Add(DamageStatics().ArmorPenetrationDef);
	RelevantAttributesToCapture.Add(DamageStatics().BlockChanceDef);

	RelevantAttributesToCapture.Add(DamageStatics().CriticalHitChanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalHitResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalHitDamageDef);

	RelevantAttributesToCapture.Add(DamageStatics().FireResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().LightningResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().ArcaneResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().PhysicalResistanceDef);
}

void UExecCalc_Damage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();
	const UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();

	AActor* SourceAvatar = SourceASC ? SourceASC->GetAvatarActor() : nullptr;
	AActor* TargetAvatar = SourceASC ? TargetASC->GetAvatarActor() : nullptr;
	
	// 获取GESpec
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	
	// 创建评估参数集   
	FAggregatorEvaluateParameters EvaluateParameters;
	
	// Aggregated 聚合的   CapturedSourceTags 捕获Tag
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
	EvaluateParameters.SourceTags = SourceTags;
	EvaluateParameters.TargetTags = TargetTags;
	// 获取攻击者等级
	ICombatInterface* SourceCombatInterface = Cast<ICombatInterface>(SourceAvatar);
	ICombatInterface* TargetCombatInterface = Cast<ICombatInterface>(TargetAvatar);
	
	// Get Damage Set by Caller Magnitude 获取  c++ Set by Caller Magnitude 传递的参数
	// float Damage = Spec.GetSetByCallerMagnitude(FAuraGameplayTags::Get().Damage);
	// 便利伤害类型，进行伤害累加
	float Damage = 0.f;
	// 伤害类型 - 伤害抗性（被捕获了属性）
	for (const TTuple<FGameplayTag, FGameplayTag>& Pair : FAuraGameplayTags::Get().DamageTpesToResistance)
	{
		const FGameplayTag DamageTypeTag = Pair.Key;
		const FGameplayTag ResistanceTag = Pair.Value;

		
		checkf(AuraDamageStatics().TagsToCaptureDefs.Contains(ResistanceTag), TEXT("TagsToCaptureDefs doesn't contain Tag: [%s] in ExecCalc_Damage"), *ResistanceTag.ToString());
		//1    AuraDamageStatics aaa = AuraDamageStatics() 注意，这里 aaa 是直接创建一个结构体，开销大，而且出了for循环直接触发析构
		//2   const FGameplayEffect CaptureDef = AuraDamageStatics().TagsToCaptureDefs 注意，这里 AuraDamageStatics() 是个右值！！ 没有任何接收，那出了这一行代码后，直接析构
		const FGameplayEffectAttributeCaptureDefinition CaptureDef = AuraDamageStatics().TagsToCaptureDefs[ResistanceTag];

		if (Spec.SetByCallerTagMagnitudes.Find(DamageTypeTag))
		{
			// 获取GE指定伤害
			float DamageTypeValue = Spec.GetSetByCallerMagnitude(DamageTypeTag, false);

			// 捕获
			float Resistance = 0.f;
			ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(CaptureDef, EvaluateParameters, Resistance);
			Resistance = FMath::Clamp(Resistance, 0.f, 100.f);
		
			// 计算伤害
			DamageTypeValue *= (100.f - Resistance) / 100.f;
		
			Damage += DamageTypeValue;
		}
	}

	// 捕获格挡几率，并查看是否格挡成功
	// 格挡成功 伤害减半
	float TargetBlockChance = 0.f;
	// Attempt = 尝试    捕获
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().BlockChanceDef, EvaluateParameters, TargetBlockChance);
	TargetBlockChance = FMath::Max<float>(0.f, TargetBlockChance);

	// 格挡几率
	const bool bBlocked = FMath::RandRange(1, 100) < TargetBlockChance;

	// 传递格挡
	{
		FGameplayEffectContextHandle EffectContextHandle = Spec.GetContext();
		UAuraAbilitySystemLibrary::SetIsBlockedHit(EffectContextHandle, bBlocked);
	}
	
	if (bBlocked)
	{
		// 伤害减半
		Damage = Damage / 2.f;
	}
	
	// 捕获护甲
	float TargetArmor = 0.f;
	// Attempt = 尝试    捕获
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ArmorDef, EvaluateParameters, TargetArmor);
	TargetArmor = FMath::Max<float>(0.f, TargetArmor);

	// 捕获护甲穿透
	float SourceArmorPenetration = 0.f;
	// Attempt = 尝试    捕获
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ArmorPenetrationDef, EvaluateParameters, SourceArmorPenetration);
	SourceArmorPenetration = FMath::Max<float>(0.f, SourceArmorPenetration);
	
	// 使用曲线
	UCharacterClassInfo* CharacterClassInfo = UAuraAbilitySystemLibrary::GetCharacterClassInfo(SourceAvatar);
	UCurveTable* CurveTable = CharacterClassInfo->DamageCalculationCoefficients;
	// 寻找曲线
	const FRealCurve* ArmorPenetrationCurve = CurveTable->FindCurve(FName("ArmorPenetration"), FString(" ArmorPenetration 没找到 "));
	// 寻找曲线值  Eval = 评估
	const float ArmorPenetrationCoefficient = ArmorPenetrationCurve->Eval(SourceCombatInterface->GetPlayerLevel());

	
	// 护甲穿透会 减少敌人的护甲值
	// 护甲减免百分比 (比如敌人护甲100  而你护甲穿透1，那最后按照百分比减免，敌人护甲就是99) 【* 0.25f = 等级系数】
	const float ArmorPenetrationPercentage = (100 - SourceArmorPenetration * ArmorPenetrationCoefficient ) / 100;
	const float EffectiveArmor = TargetArmor * ArmorPenetrationPercentage;
	// 寻找曲线
	const FRealCurve* EffectiveArmorCurve = CurveTable->FindCurve(FName("EffectiveArmor"), FString(" EffectiveArmor 没找到 "));
	// 寻找曲线值  Eval = 评估
	const float EffectiveArmorCoefficient = EffectiveArmorCurve->Eval(TargetCombatInterface->GetPlayerLevel());
	// 【百分比减免伤害---  (100 - EffectiveArmor) / 100  表示最终折算比例】
	Damage = Damage * (100 - EffectiveArmor  * EffectiveArmorCoefficient) / 100;



	
	// 暴击相关
	float SourceCriticalHitChance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalHitChanceDef, EvaluateParameters, SourceCriticalHitChance);
	SourceCriticalHitChance = FMath::Max<float>(SourceCriticalHitChance, 0.f);
	
	float TargetCriticalHitResistance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalHitResistanceDef, EvaluateParameters, TargetCriticalHitResistance);
	TargetCriticalHitResistance = FMath::Max<float>(TargetCriticalHitResistance, 0.f);

	float SourceCriticalHitDamage = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalHitDamageDef, EvaluateParameters, SourceCriticalHitDamage);
	SourceCriticalHitDamage = FMath::Max<float>(SourceCriticalHitDamage, 0.f);

	
	// 寻找曲线
	const FRealCurve* CriticalHitResistanceCurve = CurveTable->FindCurve(FName("CriticalHitResistance"), FString(" CriticalHitResistance 没找到 "));
	// 寻找曲线值  Eval = 评估
	const float CriticalHitResistanceCoefficient = CriticalHitResistanceCurve->Eval(TargetCombatInterface->GetPlayerLevel());
	
	// 爆抗 减少 暴击几率
	const float EffectiveCriticalHitChance = SourceCriticalHitChance - TargetCriticalHitResistance * CriticalHitResistanceCoefficient;
	const bool bCriticalHit = FMath::RandRange(1, 100) < EffectiveCriticalHitChance;
	
	// 传递暴击
	{
		FGameplayEffectContextHandle EffectContextHandle = Spec.GetContext();
		UAuraAbilitySystemLibrary::SetIsCriticalHit(EffectContextHandle, bCriticalHit);
	}
	
	// 暴击伤害
	Damage = bCriticalHit ? 2.f * Damage + SourceCriticalHitDamage : Damage;
	
	// 修改输出属性修改后的值
	// 目的修改： 修改 元属性 伤害
	const FGameplayModifierEvaluatedData EvaluatedData(UAuraAttributeSet::GetIncomingDamageAttribute(), EGameplayModOp::Additive, Damage);
	OutExecutionOutput.AddOutputModifier(EvaluatedData);
}
