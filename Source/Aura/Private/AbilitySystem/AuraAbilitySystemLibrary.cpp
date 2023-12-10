// yyyyyxxxxxx


#include "AbilitySystem/AuraAbilitySystemLibrary.h"

#include "AbilitySystemComponent.h"
#include "AuraAbilityTypes.h"
#include "AbilitySystem/Data/CharacterClassInfo.h"
#include "Aura/AuraGameModeBase.h"
#include "Interacton/CombatInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Player/AuraPlayerState.h"
#include "UI/HUD/AuraHUD.h"
#include "UI/WidgetController/AuraWidgetController.h"


bool UAuraAbilitySystemLibrary::MakeWidgetControllerParams(const UObject* WorldContextObject,
    FWidgetControllerParams& OutWCParams, AAuraHUD*& OutAuraHUD)
{
    if (APlayerController* PC =	UGameplayStatics::GetPlayerController(WorldContextObject, 0))
    {
        OutAuraHUD = Cast<AAuraHUD>(PC->GetHUD());
        if (OutAuraHUD)
        {
            AAuraPlayerState* PS = PC->GetPlayerState<AAuraPlayerState>();
            UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
            UAttributeSet* AS = PS->GetAttributeSet();
            
            OutWCParams.AttributeSet = AS;
            OutWCParams.AbilitySystemComponent = ASC;
            OutWCParams.PlayerState = PS;
            OutWCParams.PlayerController = PC;
            return true;
        }
    }
    return false;
}

UOverlayWidgetController* UAuraAbilitySystemLibrary::GetOverlayWidgetController(const UObject* WorldContextObject)
{
    FWidgetControllerParams WCParams;
    AAuraHUD* AuraHUD = nullptr;
    const bool bSuccessfulParams = MakeWidgetControllerParams(WorldContextObject, WCParams, AuraHUD);
    if (bSuccessfulParams)
    {
        return AuraHUD->GetOverlayWidgetController(WCParams);
    }

    return nullptr;
}

UAttributeWidgetController* UAuraAbilitySystemLibrary::GetAttributeMenuWidgetController(const UObject* WorldContextObject)
{
    FWidgetControllerParams WCParams;
    AAuraHUD* AuraHUD = nullptr;
    const bool bSuccessfulParams = MakeWidgetControllerParams(WorldContextObject, WCParams, AuraHUD);
    if (bSuccessfulParams)
    {
        return AuraHUD->GetAttributeMenuWidgetController(WCParams);
    }

    return nullptr;
}

USpellMenuWidgetController* UAuraAbilitySystemLibrary::GetSpellMenuWidgetController(const UObject* WorldContextObject)
{
    FWidgetControllerParams WCParams;
    AAuraHUD* AuraHUD = nullptr;
    const bool bSuccessfulParams = MakeWidgetControllerParams(WorldContextObject, WCParams, AuraHUD);
    if (bSuccessfulParams)
    {
        return AuraHUD->GetSpellMenuWidgetController(WCParams);
    }
    return nullptr;
}

void UAuraAbilitySystemLibrary::InitizeDefaultAttributes(const UObject* WorldContextObject, ECharacterClass CharacterClass, float Level, UAbilitySystemComponent* ASC)
{
    // 获取数据资产
    UCharacterClassInfo* ClassInfo = GetCharacterClassInfo(WorldContextObject);
    if (ClassInfo == nullptr) return;
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

void UAuraAbilitySystemLibrary::GiveStartupAbilities(const UObject* WorldContextObject, UAbilitySystemComponent* ASC, ECharacterClass CharacterClass)
{
    // 角色初始化默认属性GE+公共GA
    UCharacterClassInfo* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);
    if (CharacterClassInfo == nullptr) return;
    
    for (TSubclassOf<UGameplayAbility> AbilityClass : CharacterClassInfo->CommonAbilities)
    {
        // GA Spec 实例
        FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);

        // 给予能力GA
        ASC->GiveAbility(AbilitySpec); 
    }

    // ****  获取结构体 用引用，别拷贝
    const FCharacterClassDefaultInfo& DefaultInfo = CharacterClassInfo->GetClassDefaultInfo(CharacterClass);
    for (TSubclassOf<UGameplayAbility> AbilityClass : DefaultInfo.StartupAbilities)
    {
        // 获取等级
        if (ASC->GetAvatarActor()->Implements<UCombatInterface>())
        {
            // GA Spec 实例
            FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, ICombatInterface::Execute_GetPlayerLevel(ASC->GetAvatarActor()));

            // 给予能力GA
            ASC->GiveAbility(AbilitySpec); 
        }
    } 
    
}


int32 UAuraAbilitySystemLibrary::GetXPRewardForClassAndLevel(const UObject* WorldContextObject,
    ECharacterClass CharacterClass, int32 CharacterLevel)
{
    // 角色初始化默认属性GE+公共GA
    UCharacterClassInfo* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);
    if (CharacterClassInfo == nullptr) return 0;

    // 获取曲线
    const FCharacterClassDefaultInfo Info = CharacterClassInfo->GetClassDefaultInfo(CharacterClass);
    const float XPReward = Info.XPReward.GetValueAtLevel(CharacterLevel);
    
    return static_cast<int32>(XPReward);
}


UCharacterClassInfo* UAuraAbilitySystemLibrary::GetCharacterClassInfo(const UObject* WorldContextObject)
{
    // 获取数据资产
    AAuraGameModeBase* AuraGameMode = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));
    if (AuraGameMode == nullptr) return nullptr;
    UCharacterClassInfo* CharacterClassInfo = AuraGameMode->CharacterClassInfo;
    return CharacterClassInfo;
}


bool UAuraAbilitySystemLibrary::IsBlockedHit(const FGameplayEffectContextHandle& EffectContextHandle)
{
    // 转成自定义的 Context
    // 在UE4（虚幻引擎4）中，Cast主要是用于对象和类之间的强制转换。Cast一般不能直接用于FStruct，因为FStruct并不是UObject的子类，而Cast只能对UObject的实例进行操作。
    // 有问题的--> FAuraGameplayEffectContext* AuraContext = Cast<FAuraGameplayEffectContext>(Context); 
    if (const FAuraGameplayEffectContext* AuraEffectContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
    {
        return AuraEffectContext->IsBlockedHit();
    }
    return false;
}

bool UAuraAbilitySystemLibrary::IsCriticalHit(const FGameplayEffectContextHandle& EffectContextHandle)
{
    // *** 注意 (const FGameplayEffectContextHandle& EffectContextHandle) 带有Const 所以 static_cast<const FAuraGameplayEffectContext*> 也必须带有Const
    if (const FAuraGameplayEffectContext* AuraEffectContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
    {
        return AuraEffectContext->IsCriticalHit();
    }
    return false;
}

void UAuraAbilitySystemLibrary::SetIsBlockedHit(FGameplayEffectContextHandle& EffectContextHandle, bool bInIsBlockedHit)
{
    // *** 注意 (FGameplayEffectContextHandle& EffectContextHandle) 没有Const 所以 static_cast<FAuraGameplayEffectContext*> 可以不带Const
    if (FAuraGameplayEffectContext* AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
    {
        AuraEffectContext->SetIsBlockedHit(bInIsBlockedHit);
    }
}

void UAuraAbilitySystemLibrary::SetIsCriticalHit(FGameplayEffectContextHandle& EffectContextHandle,
    bool bInIsCriticalHit)
{
    // *** 注意 (FGameplayEffectContextHandle& EffectContextHandle) 没有Const 所以 static_cast<FAuraGameplayEffectContext*> 可以不带Const
    if (FAuraGameplayEffectContext* AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
    {
        AuraEffectContext->SetIsCriticalHit(bInIsCriticalHit);
    }
}

void UAuraAbilitySystemLibrary::GetLivePlayersWithInRadius(const UObject* WorldContextObject,
    TArray<AActor*>& OutOverlappingActors, const TArray<AActor*>& ActorsToIgnore, float Radius,
    const FVector& SphereOrigin)
{
    FCollisionQueryParams SphereParams;
    SphereParams.AddIgnoredActors(ActorsToIgnore);

    if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
    {
        TArray<FOverlapResult> Overlaps;
        World->OverlapMultiByObjectType(Overlaps, SphereOrigin, FQuat::Identity, FCollisionObjectQueryParams(FCollisionObjectQueryParams::InitType::AllDynamicObjects), FCollisionShape::MakeSphere(Radius), SphereParams);
        for (FOverlapResult& Overlap : Overlaps)
        {
            // 是否实现接口类
            // 获取接口
            if (Overlap.GetActor()->Implements<UCombatInterface>() && !ICombatInterface::Execute_IsDead(Overlap.GetActor()))
            {
                OutOverlappingActors.AddUnique(ICombatInterface::Execute_GetAvator(Overlap.GetActor()));
            }
        }
    }
    
}

bool UAuraAbilitySystemLibrary::IsNotFriend(AActor* FirstActor, AActor* SecondActor)
{
    const bool bBothArePlayers = FirstActor->ActorHasTag(FName("Player")) && SecondActor->ActorHasTag(FName("Player"));
    const bool bBothAreEnemies = FirstActor->ActorHasTag(FName("Enemy")) && SecondActor->ActorHasTag(FName("Enemy"));
    const bool bFriends = bBothArePlayers || bBothAreEnemies;
    return !bFriends;
}
