// yyyyyxxxxxx


#include "AbilitySystem/ModMagCalc/MMC_MaxHealth.h"

#include "AbilitySystem/AuraAttributeSet.h"
#include "Interacton/CombatInterface.h"

UMMC_MaxHealth::UMMC_MaxHealth()
{
	// 设置为 Vigor 活力属性
	VigorDef.AttributeToCapture = UAuraAttributeSet::GetVigorAttribute();
	VigorDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	/* Snapshot 概念： 跟踪属性值的变化
	* 属性捕获行为： 这对于实时监视和处理属性值变化非常有用，如生命值、法力值等。bSnapshot 用于控制是否应该为属性创建快照。
		1 当 bSnapshot 设置为 true 时，属性的当前值将在捕获时被记录下来。这个记录的值被称为 "快照"，它用于将属性的旧值与新值进行比较，以检测属性值的变化。
		这对于执行如生命值恢复、伤害计算等功能非常重要。
		2 变化检测： 快照允许游戏系统检测属性值的变化。如果某个属性在两次捕获之间的快照之间发生了变化，游戏系统可以触发相应的事件、效果或处理程序，以响应这些变化。
		3 节省计算资源： 如果你不需要跟踪属性的历史变化，可以将 bSnapshot 设置为 false，以减少不必要的性能开销。某些属性可能根本不需要快照，因为它们的值不会在捕获之间发生变化。
	*/
	VigorDef.bSnapshot = false;
	
	// Relevant  ˈreləvənt/ 相关
	RelevantAttributesToCapture.Add(VigorDef);
}

float UMMC_MaxHealth::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	//  Aggregated 总计 英/ˈæɡrɪɡeɪtɪd/
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	// Evaluate 评估 /ɪˈvæljueɪt/
	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = SourceTags;
	EvaluateParameters.TargetTags = TargetTags;

	float Vigor = 0.f;
	GetCapturedAttributeMagnitude(VigorDef, Spec, EvaluateParameters, Vigor);
	// 返回 Vigor 活力属性
	Vigor = FMath::Max<float>(Vigor, 0.f);

	ICombatInterface* CombatInterface = Cast<ICombatInterface>(Spec.GetContext().GetSourceObject());
	const int32 PlayerLevel = CombatInterface->GetPlayerLevel();
	
	return 80.f + (2.5 * Vigor) + (10.f * PlayerLevel);
}
