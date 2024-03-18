// yyyyyxxxxxx


#include "AbilitySystem/AuraAbilitySystemLibrary.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AuraAbilityTypes.h"
#include "AuraGameplayTags.h"
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

UAbilityInfo* UAuraAbilitySystemLibrary::GetAbilityInfo(const UObject* WorldContextObject)
{
    AAuraGameModeBase* AuraGameMode = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));
    if (AuraGameMode == nullptr) return nullptr;
    UAbilityInfo* AbilityInfo = AuraGameMode->AbilityInfo;
    return AbilityInfo;
}

bool UAuraAbilitySystemLibrary::IsSuccessfulDebuff(const FGameplayEffectContextHandle& EffectContextHandle)
{
    // 转成自定义的 Context
    // 在UE4（虚幻引擎4）中，Cast主要是用于对象和类之间的强制转换。Cast一般不能直接用于FStruct，因为FStruct并不是UObject的子类，而Cast只能对UObject的实例进行操作。
    // 有问题的--> FAuraGameplayEffectContext* AuraContext = Cast<FAuraGameplayEffectContext>(Context); 
    if (const FAuraGameplayEffectContext* AuraEffectContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
    {
        return AuraEffectContext->IsSuccessfulDebuff();
    }
    return false;
}

float UAuraAbilitySystemLibrary::GetDebuffDamage(const FGameplayEffectContextHandle& EffectContextHandle)
{
    if (const FAuraGameplayEffectContext* AuraEffectContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
    {
        return AuraEffectContext->GetDebuffDamage();
    }
    return 0.f;
}

float UAuraAbilitySystemLibrary::GetDebuffFrequency(const FGameplayEffectContextHandle& EffectContextHandle)
{
    if (const FAuraGameplayEffectContext* AuraEffectContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
    {
        return AuraEffectContext->GetDebuffFrequency();
    }
    return 0.f;
}

float UAuraAbilitySystemLibrary::GetDebuffDuration(const FGameplayEffectContextHandle& EffectContextHandle)
{
    if (const FAuraGameplayEffectContext* AuraEffectContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
    {
        return AuraEffectContext->GetDebuffDuration();
    }
    return 0.f;
}

FGameplayTag UAuraAbilitySystemLibrary::GetDamageType(const FGameplayEffectContextHandle& EffectContextHandle)
{
    if (const FAuraGameplayEffectContext* AuraEffectContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
    {
        if (AuraEffectContext->GetDamageType().IsValid())
        {
            return *AuraEffectContext->GetDamageType().Get();
        }
    }
    return FGameplayTag();
}

FVector UAuraAbilitySystemLibrary::GetDeathImpulse(const FGameplayEffectContextHandle& EffectContextHandle)
{
    if (const FAuraGameplayEffectContext* AuraEffectContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
    {
        return AuraEffectContext->GetDeathImpulse();
    }
    return FVector::ZeroVector;
}

FVector UAuraAbilitySystemLibrary::GetKnockbackForce(const FGameplayEffectContextHandle& EffectContextHandle)
{
    if (const FAuraGameplayEffectContext* AuraEffectContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
    {
        return AuraEffectContext->GetKnockbackForce();
    }
    return FVector::ZeroVector;
}

bool UAuraAbilitySystemLibrary::IsRadialDamage(const FGameplayEffectContextHandle& EffectContextHandle)
{
    if (const FAuraGameplayEffectContext* AuraEffectContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
    {
        return AuraEffectContext->IsRadialDamage();
    }
    return false;
}

float UAuraAbilitySystemLibrary::GetRadialDamageInnerRadius(const FGameplayEffectContextHandle& EffectContextHandle)
{
    if (const FAuraGameplayEffectContext* AuraEffectContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
    {
        return AuraEffectContext->GetRadialDamageInnerRadius();
    }
    return 0.f;
}

float UAuraAbilitySystemLibrary::GetRadialDamageOuterRadius(const FGameplayEffectContextHandle& EffectContextHandle)
{
    if (const FAuraGameplayEffectContext* AuraEffectContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
    {
        return AuraEffectContext->GetRadialDamageOuterRadius();
    }
    return 0.f;
}

FVector UAuraAbilitySystemLibrary::GetRadialDamageOrigin(const FGameplayEffectContextHandle& EffectContextHandle)
{
    if (const FAuraGameplayEffectContext* AuraEffectContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
    {
        return AuraEffectContext->GetRadialDamageOrigin();
    }
    return FVector::ZeroVector;
}


void UAuraAbilitySystemLibrary::SetIsSuccessfulDebuff(FGameplayEffectContextHandle& EffectContextHandle, bool bInSuccessfulDebuff)
{
    // *** 注意 (FGameplayEffectContextHandle& EffectContextHandle) 没有Const 所以 static_cast<FAuraGameplayEffectContext*> 可以不带Const
    if (FAuraGameplayEffectContext* AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
    {
        AuraEffectContext->SetIsSuccessfulDebuff(bInSuccessfulDebuff);
    }
}

void UAuraAbilitySystemLibrary::SetDebuffDamage(FGameplayEffectContextHandle& EffectContextHandle, float bInDebuffDamage)
{
    if (FAuraGameplayEffectContext* AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
    {
        AuraEffectContext->SetDebuffDamage(bInDebuffDamage);
    }
}

void UAuraAbilitySystemLibrary::SetDebuffFrequency(FGameplayEffectContextHandle& EffectContextHandle,
    float bInDebuffFrequency)
{
    if (FAuraGameplayEffectContext* AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
    {
        AuraEffectContext->SetDebuffFrequency(bInDebuffFrequency);
    }
}

void UAuraAbilitySystemLibrary::SetDebuffDuration(FGameplayEffectContextHandle& EffectContextHandle,
    float bInDebuffDuration)
{
    if (FAuraGameplayEffectContext* AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
    {
        AuraEffectContext->SetDebuffDuration(bInDebuffDuration);
    }
}

void UAuraAbilitySystemLibrary::SetDamageType(FGameplayEffectContextHandle& EffectContextHandle,
    const FGameplayTag& InDamageType)
{
    if (FAuraGameplayEffectContext* AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
    {
        TSharedPtr<FGameplayTag> DamageType = MakeShared<FGameplayTag>(InDamageType);
        AuraEffectContext->SetDamageType(DamageType);
    } 
}

void UAuraAbilitySystemLibrary::SetDeathImpulse(FGameplayEffectContextHandle& EffectContextHandle,
    const FVector& InImpulse)
{
    if (FAuraGameplayEffectContext* AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
    {
        AuraEffectContext->SetDeathImpulse(InImpulse);
    } 
}

void UAuraAbilitySystemLibrary::SetKnockbackForce(FGameplayEffectContextHandle& EffectContextHandle,
    const FVector& InKnockbackForce)
{
    if (FAuraGameplayEffectContext* AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
    {
        AuraEffectContext->SetKnockbackForce(InKnockbackForce);
    } 
}

void UAuraAbilitySystemLibrary::SetIsRadialDamage(FGameplayEffectContextHandle& EffectContextHandle,
    bool bInIsRadialDamage)
{
    if (FAuraGameplayEffectContext* AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
    {
        AuraEffectContext->SetIsRadialDamage(bInIsRadialDamage);
    } 
}

void UAuraAbilitySystemLibrary::SetRadialDamageInnerRadius(FGameplayEffectContextHandle& EffectContextHandle,
    float InInnerRadius)
{
    if (FAuraGameplayEffectContext* AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
    {
        AuraEffectContext->SetRadialDamageInnerRadius(InInnerRadius);
    } 
}

void UAuraAbilitySystemLibrary::SetRadialDamageOuterRadius(FGameplayEffectContextHandle& EffectContextHandle,
    float InOuterRadius)
{
    if (FAuraGameplayEffectContext* AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
    {
        AuraEffectContext->SetRadialDamageOuterRadius(InOuterRadius);
    } 
}

void UAuraAbilitySystemLibrary::SetRadialDamageOrigin(FGameplayEffectContextHandle& EffectContextHandle,
    FVector InOrigin)
{
    if (FAuraGameplayEffectContext* AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
    {
        AuraEffectContext->SetRadialDamageOrigin(InOrigin);
    } 
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

FGameplayEffectContextHandle UAuraAbilitySystemLibrary::ApplyDamageEffect(const FDamageEffectParams& DamageEffectParams)
{
    const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();
    const AActor* SourceAvatarActor = DamageEffectParams.SourceAbilitySystemComponent->GetAvatarActor();

    // ContextHandle
    FGameplayEffectContextHandle EffectContextHandle = DamageEffectParams.SourceAbilitySystemComponent->MakeEffectContext();
    EffectContextHandle.AddSourceObject(SourceAvatarActor);
    // 设置Context
    SetDeathImpulse(EffectContextHandle, DamageEffectParams.DeathImpulse);
    SetKnockbackForce(EffectContextHandle, DamageEffectParams.KnockbackForce);

    // 径向伤害
    SetIsRadialDamage(EffectContextHandle, DamageEffectParams.bIsRadialDamage);
    SetRadialDamageInnerRadius(EffectContextHandle, DamageEffectParams.RadialDamageInnerRadius);
    SetRadialDamageOuterRadius(EffectContextHandle, DamageEffectParams.RadialDamageOuterRadius);
    SetRadialDamageOrigin(EffectContextHandle, DamageEffectParams.RadialDamageOrigin);
	
    // SpecHande
    const FGameplayEffectSpecHandle SpecHandle = DamageEffectParams.SourceAbilitySystemComponent->MakeOutgoingSpec(
        DamageEffectParams.DamageGameplayEffectClass,
        DamageEffectParams.AbilityLevel,
        EffectContextHandle
    );
    
    // 1 分配 Tag SetByCaller     Magnitude[震级]
    //   您可以实现具有动态标签逻辑的游戏行为，而无需硬编码固定的  标签分配条件
    // 2 Get Damage Set by Caller Magnitude 获取  c++ Set by Caller Magnitude 传递的参数
    //   float Damage = Spec.GetSetByCallerMagnitude(FAuraGameplayTags::Get().Damage);
    // 3 参数1===GEhandle  参数2===Tag  参数3===具体值
    UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, DamageEffectParams.DamageType, DamageEffectParams.BaseDamage);
    UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Debuff_Chance, DamageEffectParams.DebuffChance);
    UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Debuff_Damage, DamageEffectParams.DebuffDamage);
    UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Debuff_Duration, DamageEffectParams.DebuffDuration);
    UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Debuff_Frequency, DamageEffectParams.DebuffFrequency);

    // 应用GE
    DamageEffectParams.TargetAbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);

    return EffectContextHandle;
}

TArray<FRotator> UAuraAbilitySystemLibrary::EvenlySpecedRotators(const FVector& Forward, const FVector& Axis, float Spread, int32 NumRotators)
{
    TArray<FRotator> Rotators;
	
    const FVector LeftOfSpread = Forward.RotateAngleAxis(-Spread / 2.f, Axis);
    if (NumRotators > 1)
    {
        const float DeltaSpread = Spread / (NumRotators - 1);
        for (int32 i = 0; i < NumRotators; i++)
        {
            const FVector Direction = LeftOfSpread.RotateAngleAxis(DeltaSpread * i, FVector::UpVector);
            Rotators.Add(Direction.Rotation());
        }
    }
    else
    {
        Rotators.Add(Forward.Rotation());
    }
    return Rotators;
}

TArray<FVector> UAuraAbilitySystemLibrary::EvenlyRotatedVectors(const FVector& Forward, const FVector& Axis, float Spread, int32 NumVectors)
{
    TArray<FVector> Vectors;
	
    const FVector LeftOfSpread = Forward.RotateAngleAxis(-Spread / 2.f, Axis);
    if (NumVectors > 1)
    {
        const float DeltaSpread = Spread / (NumVectors - 1);
        for (int32 i = 0; i < NumVectors; i++)
        {
            const FVector Direction = LeftOfSpread.RotateAngleAxis(DeltaSpread * i, FVector::UpVector);
            Vectors.Add(Direction);
        }
    }
    else
    {
        Vectors.Add(Forward);
    }
    return Vectors;
}

void UAuraAbilitySystemLibrary::GetClosestTargets(int32 MaxTargets, const TArray<AActor*>& Actors, TArray<AActor*>& OutClosestTargets, const FVector& Origin)
{
    if (Actors.Num() <= MaxTargets)
    {
        OutClosestTargets = Actors;
        return;
    }

    TArray<AActor*> ActorsToCheck = Actors;
    int32 NumTargetsFound = 0;

    while (NumTargetsFound < MaxTargets)
    {
        if (ActorsToCheck.Num() == 0) break;
        double ClosestDistance = TNumericLimits<double>::Max();
        AActor* ClosestActor;
        for (AActor* PotentialTarget : ActorsToCheck)
        {
            const double Distance = (PotentialTarget->GetActorLocation() - Origin).Length();
            if (Distance < ClosestDistance)
            {
                ClosestDistance = Distance;
                ClosestActor = PotentialTarget;
            }
        }
        ActorsToCheck.Remove(ClosestActor);
        OutClosestTargets.AddUnique(ClosestActor);
        ++NumTargetsFound;
    }
}