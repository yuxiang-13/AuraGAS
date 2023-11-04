// yyyyyxxxxxx


#include "AbilitySystem/ExecCalc/ExecCalc_Damage.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"

// 1 原C++ 结构体。没必要加F 因为是原生c++
struct AuraDamageStatics
{
	// ① 使用 （声明属性捕获）宏定义
	// 这个宏，作用是创建属性捕获变量，P只是变量名
	DECLARE_ATTRIBUTE_CAPTUREDEF(Armor) // Armor 护甲

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
	
}

void UExecCalc_Damage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();
	const UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();

	const AActor* SourceAvatar = SourceASC ? SourceASC->GetAvatarActor() : nullptr;
	const AActor* TargetAvatar = SourceASC ? TargetASC->GetAvatarActor() : nullptr;
	
	// 获取GESpec
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	
	// 创建评估参数集   
	FAggregatorEvaluateParameters EvaluateParameters;
	
	// Aggregated 聚合的   CapturedSourceTags 捕获Tag
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
	EvaluateParameters.SourceTags = SourceTags;
	EvaluateParameters.TargetTags = TargetTags;

	// Attempt = 尝试    第一次捕获 10.25 + 1 = 11.25   第二次捕获  21.5+1=22.5  也就是说，第一次在10.25基础上+11.25，所以第二次是21.5
	// 因为使用了 EGameplayModOp::Additive 做加法
	float ArmorNum = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ArmorDef, EvaluateParameters, ArmorNum);
	ArmorNum = FMath::Max<float>(0.f, ArmorNum);

	++ArmorNum;

	// 修改输出属性修改后的值
	// 目的修改： +- 护甲   float ArmorNum = 0.f;
	const FGameplayModifierEvaluatedData EvaluatedData(DamageStatics().ArmorProperty, EGameplayModOp::Additive, ArmorNum);
	OutExecutionOutput.AddOutputModifier(EvaluatedData);
}
