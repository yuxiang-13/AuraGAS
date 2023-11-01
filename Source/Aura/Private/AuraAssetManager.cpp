// yyyyyxxxxxx


#include "AuraAssetManager.h"

#include "AbilitySystemGlobals.h"
#include "AuraGameplayTags.h"

UAuraAssetManager& UAuraAssetManager::Get()
{
	check(GEngine);
	
	UAuraAssetManager* AuraAssetManager = Cast<UAuraAssetManager>(GEngine->AssetManager);
	return *AuraAssetManager;
}

void UAuraAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();
	// 加载c++定义GameplayTag
	FAuraGameplayTags::InitializeNativeGameplayTags();

	// 初始化GAS全局数据
	UAbilitySystemGlobals::Get().InitGlobalData();
}
