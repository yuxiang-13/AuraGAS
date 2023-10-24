// yyyyyxxxxxx


#include "UI/WidgetController/AttributeWidgetController.h"

#include "AbilitySystem/AuraAttributeSet.h"
#include "AbilitySystem/Data/AttributeInfo.h"


void UAttributeWidgetController::BindCallbackToDependencies()
{
	const UAuraAttributeSet* AuraAttributeSet = CastChecked<UAuraAttributeSet>(AttributeSet);
	check(AttributeInfo);
	
	for (auto& Pair : AuraAttributeSet->TagsToAttributes)
	{
		// 绑定建通属性变化
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
			Pair.Value() // 绑定函数指针
		).AddLambda(
			[this, Pair](const FOnAttributeChangeData& Data)
			{
				BroadcastAttributeInfo(Pair.Key,  Pair.Value());
			}
		);
		
	}
}

void UAttributeWidgetController::BroadcastInitialValues()
{
	UAuraAttributeSet* AS = CastChecked<UAuraAttributeSet>(AttributeSet);
	check(AttributeInfo);

	for (auto& Pair : AS->TagsToAttributes)
	{
		BroadcastAttributeInfo(Pair.Key,  Pair.Value());
	}
}


void UAttributeWidgetController::BroadcastAttributeInfo(const FGameplayTag& AttributeTag,
	const FGameplayAttribute Attribute) const
{
	FAuraAttributeInfo Info = AttributeInfo->FindAttributeInfoForTag(AttributeTag);
	Info.AttributeValue = Attribute.GetNumericValue(AttributeSet);
	AttributeInfoSignature.Broadcast(Info);
}