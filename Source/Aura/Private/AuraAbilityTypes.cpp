
#include "AuraAbilityTypes.h"

bool FAuraGameplayEffectContext::IsSuccessfulDebuff() const
{
	return bIsSuccessfulDebuff ;
}

void FAuraGameplayEffectContext::SetIsSuccessfulDebuff(bool bInIsDebuff)
{
	bIsSuccessfulDebuff = bInIsDebuff ;
}

bool FAuraGameplayEffectContext::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	// uint32 最多32位，下面的这些 RepBits |= 1 << 32
	uint32 RepBits = 0;
	
	if (Ar.IsSaving())
	{
		if (bReplicateInstigator && Instigator.IsValid())
		{
			RepBits |= 1 << 0;
		}
		if (bReplicateEffectCauser && EffectCauser.IsValid() )
		{
			RepBits |= 1 << 1;
		}
		if (AbilityCDO.IsValid())
		{
			RepBits |= 1 << 2;
		}
		if (bReplicateSourceObject && SourceObject.IsValid())
		{
			RepBits |= 1 << 3;
		}
		if (Actors.Num() > 0)
		{
			RepBits |= 1 << 4;
		}
		if (HitResult.IsValid())
		{
			RepBits |= 1 << 5;
		}
		if (bHasWorldOrigin)
		{
			RepBits |= 1 << 6;
		}
		// 1------ 加入自己自定义的
		if (bIsCriticalHit)
		{
			RepBits |= 1 << 7;
		}
		if (bIsBlockedHit)
		{
			RepBits |= 1 << 8;
		}
		if (bIsSuccessfulDebuff)
		{
			RepBits |= 1 << 9;
		}
		if (Debuff_Damage > 0.f)
		{
			RepBits |= 1 << 10;
		}
		if (Debuff_Frequency > 0.f)
		{
			RepBits |= 1 << 11;
		}
		if (Debuff_Duration > 0.f)
		{
			RepBits |= 1 << 12;
		}
		if (DamageType.IsValid())
		{
			RepBits |= 1 << 13;
		}
		if (!DeathImpulse.IsZero())
		{
			RepBits |= 1 << 14;
		}
		if (!KnockbackForce.IsZero())
		{
			RepBits |= 1 << 15;
		}
	}
	
	
	// ***  序列化 前面 15 个
 	Ar.SerializeBits(&RepBits, 15);

	if (RepBits & (1 << 0))
	{
		Ar << Instigator;
	}
	if (RepBits & (1 << 1))
	{
		Ar << EffectCauser;
	}
	if (RepBits & (1 << 2))
	{
		Ar << AbilityCDO;
	}
	if (RepBits & (1 << 3))
	{
		Ar << SourceObject;
	}
	if (RepBits & (1 << 4))
	{
		SafeNetSerializeTArray_Default<31>(Ar, Actors);
	}
	if (RepBits & (1 << 5))
	{
		if (Ar.IsLoading())
		{
			if (!HitResult.IsValid())
			{
				HitResult = TSharedPtr<FHitResult>(new FHitResult());
			}
		}
		HitResult->NetSerialize(Ar, Map, bOutSuccess);
	}
	if (RepBits & (1 << 6))
	{
		Ar << WorldOrigin;
		bHasWorldOrigin = true;
	}
	else
	{
		bHasWorldOrigin = false;
	}

	// 2 ---- 自定义数据
	if (RepBits & (1 << 7))
	{
		Ar << bIsBlockedHit;
	}
	
	if (RepBits & (1 << 8))
	{
		Ar << bIsCriticalHit;
	}

	
	if (RepBits & (1 << 9))
	{
		Ar << bIsSuccessfulDebuff;
	}
	
	if (RepBits & (1 << 10))
	{
		Ar << Debuff_Damage;
	}
	
	if (RepBits & (1 << 11))
	{
		Ar << Debuff_Frequency;
	}
	
	if (RepBits & (1 << 12))
	{
		Ar << Debuff_Duration;
	}
	if (RepBits & (1 << 13))
	{
		// 1 正在序列化
		if (Ar.IsLoading())
		{
			// 2 如果不存在 GameplayTag
			if (!DamageType.IsValid())
			{
				// 3 创建新的
				DamageType = TSharedPtr<FGameplayTag>(new FGameplayTag());
			}
		}
		// 4 进行序列化
		DamageType->NetSerialize(Ar, Map, bOutSuccess);
	}
	if (RepBits & (1 << 14))
	{
		// 进行序列化  FVector 自身具备网络序列化能力
		DeathImpulse.NetSerialize(Ar, Map, bOutSuccess);
	}
	
	if (RepBits & (1 << 15))
	{
		// 进行序列化  FVector 自身具备网络序列化能力
		KnockbackForce.NetSerialize(Ar, Map, bOutSuccess);
	}
	

	if (Ar.IsLoading())
	{
		AddInstigator(Instigator.Get(), EffectCauser.Get()); // Just to initialize InstigatorAbilitySystemComponent
	}
	bOutSuccess = true;

	
	return true;
}
