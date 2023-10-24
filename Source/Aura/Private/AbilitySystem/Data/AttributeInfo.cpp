// yyyyyxxxxxx


#include "AbilitySystem/Data/AttributeInfo.h"

FAuraAttributeInfo UAttributeInfo::FindAttributeInfoForTag(const FGameplayTag& AttributeTag, bool bLogNotFound) const
{
	for (const FAuraAttributeInfo& Info : AttributeInformation)
	{
		// Matches 匹配  Exact 精确
		if (Info.AttributeTag.MatchesTagExact(AttributeTag))
		{
			return Info;
		}
	}

	if (bLogNotFound)
	{
		UE_LOG(LogTemp, Error, TEXT("没找到 tag [%s]  on  AttributeInfo [%s] "), *AttributeTag.ToString(), *GetNameSafe(this));
	}
	return FAuraAttributeInfo();
}
