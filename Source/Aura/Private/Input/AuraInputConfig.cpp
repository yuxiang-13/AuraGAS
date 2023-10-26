// yyyyyxxxxxx


#include "Input/AuraInputConfig.h"

const UInputAction* UAuraInputConfig::FindAbilityInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound)
{
	// for (auto& xx : StartupAbilities)
	for (const FAuraInputAction& Action : AbilityInputActions)
	{
		if (Action.InputAction && Action.InputTag == InputTag)
		{
			return Action.InputAction;
		}
	}
	if (bLogNotFound)
	{
		UE_LOG(LogTemp, Error, TEXT(" cant't find inputTag [%s],  on inputconfig [%s]"), *InputTag.ToString(), *GetNameSafe(this))
	}
	return nullptr;
}
