// yyyyyxxxxxx


#include "Actor/PointCollection.h"

#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"

APointCollection::APointCollection()
{
	PrimaryActorTick.bCanEverTick = false;

	Pt_0 = CreateDefaultSubobject<USceneComponent>("Pt_0");
	ImmutablePts.Add(Pt_0);
	SetRootComponent(Pt_0);

	Pt_1 = CreateDefaultSubobject<USceneComponent>("Pt_1");
	ImmutablePts.Add(Pt_1);
	Pt_1->SetupAttachment(GetRootComponent());

	Pt_2 = CreateDefaultSubobject<USceneComponent>("Pt_2");
	ImmutablePts.Add(Pt_2);
	Pt_2->SetupAttachment(GetRootComponent());

	Pt_3 = CreateDefaultSubobject<USceneComponent>("Pt_3");
	ImmutablePts.Add(Pt_3);
	Pt_3->SetupAttachment(GetRootComponent());
	
	Pt_4 = CreateDefaultSubobject<USceneComponent>("Pt_4");
	ImmutablePts.Add(Pt_4);
	Pt_4->SetupAttachment(GetRootComponent());

	Pt_5 = CreateDefaultSubobject<USceneComponent>("Pt_5");
	ImmutablePts.Add(Pt_5);
	Pt_5->SetupAttachment(GetRootComponent());
	
	Pt_6 = CreateDefaultSubobject<USceneComponent>("Pt_6");
	ImmutablePts.Add(Pt_6);
	Pt_6->SetupAttachment(GetRootComponent());

	Pt_7 = CreateDefaultSubobject<USceneComponent>("Pt_7");
	ImmutablePts.Add(Pt_7);
	Pt_7->SetupAttachment(GetRootComponent());

	Pt_8 = CreateDefaultSubobject<USceneComponent>("Pt_8");
	ImmutablePts.Add(Pt_8);
	Pt_8->SetupAttachment(GetRootComponent());

	Pt_9 = CreateDefaultSubobject<USceneComponent>("Pt_9");
	ImmutablePts.Add(Pt_9);
	Pt_9->SetupAttachment(GetRootComponent());

	Pt_10 = CreateDefaultSubobject<USceneComponent>("Pt_10");
	ImmutablePts.Add(Pt_10);
	Pt_10->SetupAttachment(GetRootComponent());

}

// 获取区域生成点，粒子生成的位置（位置+角度）
// GroundLocation 地面位置
TArray<USceneComponent*> APointCollection::GetGroundPoints(const FVector& GroundLocation, int32 NumPoints,
	float YawOverride)
{
	// 访问越界
	checkf(ImmutablePts.Num() >= NumPoints, TEXT("Attempted to access ImmutablePts out of bounds"));

	TArray<USceneComponent*> ArrayCopy;

	for (USceneComponent* Pt : ImmutablePts)
	{
		// 生成所需点
		if (ArrayCopy.Num() >= NumPoints) return ArrayCopy;

        // Pt_0 是actor 根,不参与位置和旋转，其余都是这个中心点向四周 做成点
		if (Pt != Pt_0)
		{
			// 当前到中心点位置
			FVector ToPoint = Pt->GetComponentLocation() - Pt_0->GetComponentLocation();
			// 旋转位置
			ToPoint = ToPoint.RotateAngleAxis(YawOverride, FVector::UpVector);
			// 设置新位置
			Pt->SetWorldLocation(Pt_0->GetComponentLocation() + ToPoint);
		}

		// 最高点位置
		const FVector RaisedLocation = FVector(Pt->GetComponentLocation().X, Pt->GetComponentLocation().Y, Pt->GetComponentLocation().Z + 500.f);
		// 最低点位置
		const FVector LoweredLocation = FVector(Pt->GetComponentLocation().X, Pt->GetComponentLocation().Y, Pt->GetComponentLocation().Z - 500.f);

		FHitResult HitResult;
		// IgnoreActors 这个实际就是 GetActorLocation() 半径内的敌人。这个名字是下面我们射线要忽略的 敌人，所以名字叫 IgnoreActors
		TArray<AActor*> IgnoreActors;
		UAuraAbilitySystemLibrary::GetLivePlayersWithInRadius(this, IgnoreActors, TArray<AActor*>(), 1500.f, GetActorLocation());

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActors(IgnoreActors);
		GetWorld()->LineTraceSingleByProfile(HitResult, RaisedLocation, LoweredLocation, FName("BlockAll"), QueryParams);

		// 设置碰撞位置为 高度Z
		const FVector AdjustedLocation = FVector(Pt->GetComponentLocation().X, Pt->GetComponentLocation().Y, HitResult.ImpactPoint.Z);
		Pt->SetWorldLocation(AdjustedLocation);
		// HitResult.ImpactNormal = 垂直地面的法线，MakeRotFromZ 为旋转，也就保证了 Z=Yaw=
		/*
			1  UKismetMathLibrary::MakeRotFromZ() 是另一个函数，它根据输入的单一向量创建一个旋转量，
					使得该旋转的Z轴与输入的向量一致。其它轴（X和Y）被以不可预测的方式填充，但总会形成一个完全的坐标系。
			2  HitResult.ImpactNormal 是一个向量，描述了一个碰撞事件中的面的法线，即该面垂直于其平面的向量。
			3  因此，整条语句 SetWorldRotation(UKismetMathLibrary::MakeRotFromZ(HitResult.ImpactNormal)) 的作用是，
			根据碰撞面的法线向量设置一个对象的世界旋转。也就是将对象的Z轴对齐至碰撞面的法线，使得对象的前进方向与碰撞面保持垂直。
			如此可以使对象沿着聊天面进行移动，例如使人物沿着墙壁走或者使汽车沿着坡道上升等等。
			注意，此处的Z轴对于不同的坐标系可能意味着不同的方向，可能是向上，也可能是向前，具体需要考虑具体的游戏或者应用相关设置。

			总结：例如Character,X轴是向前，Z轴是向上，用这种手段的话，也就保证Z与HitResult.ImpactNormal同向，也就会人物垂直于地面站直
		 */
		Pt->SetWorldRotation(UKismetMathLibrary::MakeRotFromZ(HitResult.ImpactNormal));

		ArrayCopy.Add(Pt);
	}
	return ArrayCopy;
	
}

void APointCollection::BeginPlay()
{
	Super::BeginPlay();
	
}
