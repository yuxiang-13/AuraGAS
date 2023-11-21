// yyyyyxxxxxx


#include "AbilitySystem/Abilities/AuraSummonAbility.h"


TArray<FVector> UAuraSummonAbility::GetSpawnLocations()
{
	const FVector Forawrd = GetAvatarActorFromActorInfo()->GetActorForwardVector();
	const FVector Location = GetAvatarActorFromActorInfo()->GetActorLocation();
	// 起始 方向向量
	//const FVector RightOfSpread = Forawrd.RotateAngleAxis(SpawnSpread / 2.f, FVector::UpVector);
	// Z轴 顺时针[正值]是 右，  逆时针[负值]是 左
	const FVector LeftOfSpread = Forawrd.RotateAngleAxis(-SpawnSpread / 2.f, FVector::UpVector);
	// 角度分段
	const float DeltaSpread = SpawnSpread / NumMinions;

	TArray<FVector> SpawnLocations;
	for (int32 i = 0; i < NumMinions; i++)
	{
		// ***** 增加旋转值。是顺时针增加，所以要从左手边累加到右手边
		// 起始向量一步步增加旋转量
		const FVector Direction = LeftOfSpread.RotateAngleAxis(DeltaSpread * i, FVector::UpVector);
		// 生成位置
		FVector ChosenSpawnLocation = Location + Direction * FMath::FRandRange(MinSpawnDistance, MaxSpawnDistance);

		// 射线检测高度上碰撞 （从上到下）
		FHitResult Hit;
		GetWorld()->LineTraceSingleByChannel(Hit, ChosenSpawnLocation + FVector(0.f, 0.f, 400.f), ChosenSpawnLocation - FVector(0.f, 0.f, 400.f), ECC_Vehicle);
		if (Hit.bBlockingHit)
		{
			ChosenSpawnLocation = Hit.ImpactPoint;
		}
		
		
		SpawnLocations.Add(ChosenSpawnLocation);
	}

	
	
	return SpawnLocations;
}

TSubclassOf<APawn> UAuraSummonAbility::GetRandomMinionClass()
{
	const int32 Slection = FMath::RandRange(0, MinionClasses.Num() - 1);
	return MinionClasses[Slection];
}
