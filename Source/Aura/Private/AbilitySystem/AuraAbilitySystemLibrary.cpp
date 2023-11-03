// yyyyyxxxxxx


#include "AbilitySystem/AuraAbilitySystemLibrary.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/Data/CharacterClassInfo.h"
#include "Aura/AuraGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Player/AuraPlayerState.h"
#include "UI/HUD/AuraHUD.h"
#include "UI/WidgetController/AuraWidgetController.h"

UOverlayWidgetController* UAuraAbilitySystemLibrary::GetOverlayWidgetController(const UObject* WorldContextObject)
{
    if (APlayerController* PC =	UGameplayStatics::GetPlayerController(WorldContextObject, 0))
    {
        if (AAuraHUD* AuraHUD = Cast<AAuraHUD>(PC->GetHUD()))
        {
            AAuraPlayerState* PS = PC->GetPlayerState<AAuraPlayerState>();
            UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
            UAttributeSet* AS = PS->GetAttributeSet();
            const FWidgetControllerParams WidgetControllerParams(PC, PS, ASC, AS);

            return AuraHUD->GetOverlayWidgetController(WidgetControllerParams);
        }
    }

    return nullptr;
}

UAttributeWidgetController* UAuraAbilitySystemLibrary::GetAttributeMenuWidgetController(
    const UObject* WorldContextObject)
{
    if (APlayerController* PC =	UGameplayStatics::GetPlayerController(WorldContextObject, 0))
    {
        if (AAuraHUD* AuraHUD = Cast<AAuraHUD>(PC->GetHUD()))
        {
            AAuraPlayerState* PS = PC->GetPlayerState<AAuraPlayerState>();
            UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
            UAttributeSet* AS = PS->GetAttributeSet();
            const FWidgetControllerParams WidgetControllerParams(PC, PS, ASC, AS);

            return AuraHUD->GetAttributeMenuWidgetController(WidgetControllerParams);
        }
    }

    return nullptr;
}

void UAuraAbilitySystemLibrary::InitizeDefaultAttributes(const UObject* WorldContextObject, ECharacterClass CharacterClass, float Level, UAbilitySystemComponent* ASC)
{
    // 获取数据资产
    AAuraGameModeBase* AuraGameMode = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));
    if (AuraGameMode == nullptr) return;
    UCharacterClassInfo* ClassInfo = AuraGameMode->CharacterClassInfo;

    AActor* AvatarActor = ASC->GetAvatarActor();
    // 根据敌人类型获取PrimaryAttributes
    FCharacterClassDefaultInfo ClassDefaultInfo = ClassInfo->GetClassDefaultInfo(CharacterClass);
    // 应用 主要属性 GE
    FGameplayEffectContextHandle GEHandle = ASC->MakeEffectContext();
    GEHandle.AddSourceObject(AvatarActor);
    const FGameplayEffectSpecHandle GEPrimaryAttributesSpecHandle = ASC->MakeOutgoingSpec(ClassDefaultInfo.PrimaryAttributes, Level, GEHandle);
    ASC->ApplyGameplayEffectSpecToSelf(*GEPrimaryAttributesSpecHandle.Data.Get());

    // 应用 次要属性 GE
    FGameplayEffectContextHandle GESecondaryAttributesHandle = ASC->MakeEffectContext();
    GESecondaryAttributesHandle.AddSourceObject(AvatarActor);
    const FGameplayEffectSpecHandle GESecondaryAttributesSpecHandle = ASC->MakeOutgoingSpec(ClassInfo->SecondaryAttributes, Level, GESecondaryAttributesHandle);
    ASC->ApplyGameplayEffectSpecToSelf(*GESecondaryAttributesSpecHandle.Data.Get());
    // 应用 重要属性 GE
    FGameplayEffectContextHandle GEVitalAttributesHandle = ASC->MakeEffectContext();
    GEVitalAttributesHandle.AddSourceObject(AvatarActor);
    const FGameplayEffectSpecHandle GEVitalAttributesSpecHandle = ASC->MakeOutgoingSpec(ClassInfo->VitalAttributes, Level, GEVitalAttributesHandle);
    ASC->ApplyGameplayEffectSpecToSelf(*GEVitalAttributesSpecHandle.Data.Get());
}

void UAuraAbilitySystemLibrary::GiveStartupAbilities(const UObject* WorldContextObject, UAbilitySystemComponent* ASC)
{
    // 获取数据资产
    AAuraGameModeBase* AuraGameMode = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));
    if (AuraGameMode == nullptr) return;
    // 角色初始化默认属性GE+公共GA
    UCharacterClassInfo* CharacterClassInfo = AuraGameMode->CharacterClassInfo;
    for (TSubclassOf<UGameplayAbility> AbilityClass : CharacterClassInfo->CommonAbilities)
    {
        // GA Spec 实例
        FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);

        // 给予能力GA
        ASC->GiveAbility(AbilitySpec); 
    }
    
}
