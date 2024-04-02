// Copyright Epic Games, Inc. All Rights Reserved.


#include "AuraGameModeBase.h"

#include "AuraGameplayTags.h"
#include "EngineUtils.h"
#include "Game/AuraGameInstance.h"
#include "Game/LoadScreenSaveGame.h"
#include "GameFramework/PlayerStart.h"
#include "Interacton/SaveInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "UI/ViewModel/MVVM_LoadSlot.h"

void AAuraGameModeBase::BeginPlay()
{
	Super::BeginPlay();
	
	Maps.Add(DefaultMapName, DefaultMap);
}

void AAuraGameModeBase::SavaSlotData(UMVVM_LoadSlot* LoadSlot, int32 SlotIndex)
{
	// 删除数据
	if (UGameplayStatics::DoesSaveGameExist(LoadSlot->GetLoadSlotName(), SlotIndex))
	{
		UGameplayStatics::DeleteGameInSlot(LoadSlot->GetLoadSlotName(), SlotIndex);
	}

	// 创建 and 修改数据
	USaveGame* SaveGameObject = UGameplayStatics::CreateSaveGameObject(LoadScreenSaveGameClass);
	ULoadScreenSaveGame* LoadScreenSaveGame = Cast<ULoadScreenSaveGame>(SaveGameObject);
	LoadScreenSaveGame->PlayerName = LoadSlot->GetLoadSlotName();
	// 设置已经存在状态
	LoadScreenSaveGame->SaveSlotStatus = ESaveSlotStates::Taken;
	LoadScreenSaveGame->MapName = LoadSlot->GetMapName();
	LoadScreenSaveGame->PlayerStartTag = LoadSlot->PlayerStartTag;
	
	
	// 保存游戏
	UGameplayStatics::SaveGameToSlot(LoadScreenSaveGame, LoadSlot->GetLoadSlotName(), SlotIndex);
}

ULoadScreenSaveGame* AAuraGameModeBase::GetSaveSlotData(const FString& SlotName, int32 SlotIndex) const
{
	USaveGame* SaveGameObject = nullptr;

	if (UGameplayStatics::DoesSaveGameExist(SlotName, SlotIndex))
	{
		SaveGameObject = UGameplayStatics::LoadGameFromSlot(SlotName, SlotIndex);
	} else
	{
		SaveGameObject = UGameplayStatics::CreateSaveGameObject(LoadScreenSaveGameClass);
	}

	return Cast<ULoadScreenSaveGame>(SaveGameObject);
}

void AAuraGameModeBase::DeleteSlot(const FString& SlotName, int32 SlotIndex)
{
	// 删除数据
	if (UGameplayStatics::DoesSaveGameExist(SlotName, SlotIndex))
	{
		UGameplayStatics::DeleteGameInSlot(SlotName, SlotIndex);
	}

}

ULoadScreenSaveGame* AAuraGameModeBase::RetrieveInGameSaveData()
{
	UAuraGameInstance* AuraGameInstance = Cast<UAuraGameInstance>(GetGameInstance());

	const FString InGameLoadSlotName = AuraGameInstance->LoadSlotName;
	const int32 InGameLoadSlotIndex = AuraGameInstance->LoadSlotIndex;

	return GetSaveSlotData(InGameLoadSlotName, InGameLoadSlotIndex);
}

void AAuraGameModeBase::SaveInGameProgressData(ULoadScreenSaveGame* SaveObject)
{
	UAuraGameInstance* AuraGameInstance = Cast<UAuraGameInstance>(GetGameInstance());
	const FString InGameLoadSlotName = AuraGameInstance->LoadSlotName;
	const int32 InGameLoadSlotIndex = AuraGameInstance->LoadSlotIndex;

	AuraGameInstance->PlayerStartTag = SaveObject->PlayerStartTag;

	UGameplayStatics::SaveGameToSlot(SaveObject, InGameLoadSlotName, InGameLoadSlotIndex);
}

void AAuraGameModeBase::SaveWorldState(UWorld* World)
{
	FString WorldName = World->GetMapName();
	// `WorldName.RemoveFromStart(World->StreamingLevelsPrefix);`：从WorldName的开始部分中移除World对象的StreamingLevelsPrefix。
	// StreamingLevelsPrefix通常是一个特定的前缀字符串，标识了Streaming Level的名称。
	WorldName.RemoveFromStart(World->StreamingLevelsPrefix);

	UAuraGameInstance* AuraGI = Cast<UAuraGameInstance>(GetGameInstance());
	if (ULoadScreenSaveGame* SaveGame = GetSaveSlotData(AuraGI->LoadSlotName, AuraGI->LoadSlotIndex))
	{
		if (!SaveGame->HasMap(WorldName))
		{
			FSavedMap NewSavedMap;
			NewSavedMap.MapAssetName = WorldName;
			SaveGame->SavedMaps.Add(NewSavedMap);
		}
		FSavedMap SavedMap =  SaveGame->GetSavedMapWithMapName(WorldName);
		
		SavedMap.SavedActors.Empty();
		for (FActorIterator It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (!IsValid(Actor) || !Actor->Implements<USaveInterface>())
			{
				continue;
			}

			FSavedActor SavedActor;
			SavedActor.ActorName = Actor->GetFName();
			SavedActor.Transform = Actor->GetTransform();

			//在UE4中，TMemoryWriter是一种序列化工具，用于向内存中写入数据。它经常用于把一个对象或一组数据写入到一个内存缓冲区中，
			//并且可以按照预先定义的格式进行编码。TMemoryWriter实际上是FArchive类的一个子类，其作用是实现"save"的操作。
			FMemoryWriter MemoryWriter(SavedActor.Bytes);
			// 一： 这个类主要用于跨版本或者跨平台的情况，其中字符串形式的序列化可以提供更好的兼容性。因此，你可以理解FObjectAndNameAsStringProxyArchive为：提供更多序列化控制，
			// 并且可以选择将对象名和名称引用存储为字符串的工具。
			// 二： 如果构造FObjectAndNameAsStringProxyArchive时传入的bool值为true，那么当序列化UObject引用或FName实例时，它们的字符串形式将被写入到内存中。
			// 这样做的好处是便于调试和可读性，因为可以直接查看和理解序列化后的数据。但是，这种方式占据的空间也会相对较大。
			FObjectAndNameAsStringProxyArchive Archive(MemoryWriter, true);
			// `Archive.ArIsSaveGame = true;`这一语句是设置Archive对象的ArIsSaveGame属性。
			// 如果将它设为true，那么系统将明白这个归档操作是为了保存游戏的状态。这对于在游戏中保存和加载对象的状态非常有用。
			Archive.ArIsSaveGame = true;
			// `Actor->Serialize(Archive);`这一语句则是开始序列化Actor对象。这会调用Actor对象的Serialize函数，
			// 将其状态写入到之前创建的Archive中。这个过程也被应用于游戏中的保存功能。
			Actor->Serialize(Archive);
			SavedMap.SavedActors.AddUnique(SavedActor);
		}

		for (FSavedMap& MapToReplace : SaveGame->SavedMaps)
		{
			if (MapToReplace.MapAssetName == WorldName)
			{
				MapToReplace = SavedMap;
			}
		}
		UGameplayStatics::SaveGameToSlot(SaveGame, AuraGI->LoadSlotName, AuraGI->LoadSlotIndex);
	}
}

void AAuraGameModeBase::LoadWorldState(UWorld* World)
{
	FString WorldName = World->GetMapName();
	// `WorldName.RemoveFromStart(World->StreamingLevelsPrefix);`：从WorldName的开始部分中移除World对象的StreamingLevelsPrefix。
	// StreamingLevelsPrefix通常是一个特定的前缀字符串，标识了Streaming Level的名称。
	WorldName.RemoveFromStart(World->StreamingLevelsPrefix);
	UAuraGameInstance* AuraGI = Cast<UAuraGameInstance>(GetGameInstance());

	// 缓存中存在
	if (UGameplayStatics::DoesSaveGameExist(AuraGI->LoadSlotName, AuraGI->LoadSlotIndex))
	{
		// 获取缓存
		ULoadScreenSaveGame* SaveGame = Cast<ULoadScreenSaveGame>(UGameplayStatics::LoadGameFromSlot(AuraGI->LoadSlotName, AuraGI->LoadSlotIndex));
		
		if (SaveGame == nullptr)
		{
			return;
		}
		
		for (FActorIterator It(World); It; ++It)
		{
			AActor* Actor = *It;

			if (!Actor->Implements<USaveInterface>())
			{
				continue;
			}

			for (FSavedActor SavedActor: SaveGame->GetSavedMapWithMapName(WorldName).SavedActors)
			{
				if (SavedActor.ActorName == Actor->GetFName())
				{
					if (ISaveInterface::Execute_ShouldLoadTransform(Actor))
					{
						Actor->SetActorTransform(SavedActor.Transform);
						continue;
					}

					// 读取  这段代码是Unreal Engine 4中的一段序列化相关代码。它主要用于从存储对象状态的字节数据中重新构造Actor。
					// MemoryReader，它负责从SavedActor的字节数据中读取信息。
					FMemoryReader MemoryReader(SavedActor.Bytes);
					// `FObjectAndNameAsStringProxyArchive Archive(MemoryReader, true);`创建了FObjectAndNameAsStringProxyArchive的一个实例，
					// 该实例能够将其他对象序列化为能表示为字符串的形式，并且这个字符串可以是该对象的类名或变量名。
					FObjectAndNameAsStringProxyArchive Archive(MemoryReader, true);
					// `Archive.ArIsSaveGame = true;`设置了用于序列化的归档对象的ArIsSaveGame属性，意味着归档/解归档操作用于保存游戏状态。
					Archive.ArIsSaveGame = true;
					// `Actor->Serialize(Archive);`用给定的归档数据反序列化Actor对象。这个步骤执行后，Actor对象的状态就会变为保存时的状态。
					Actor->Serialize(Archive);

					ISaveInterface::Execute_LoadActor(Actor);
				}
			}
		}
	}
}

void AAuraGameModeBase::TravelToMap(UMVVM_LoadSlot* Slot)
{
	const FString SlotName = Slot->GetLoadSlotName();
	const int32 SlotIndex = Slot->SlotIndex;
	const FString MapName = Slot->GetMapName();

	UGameplayStatics::OpenLevelBySoftObjectPtr(Slot, Maps.FindChecked(MapName));
}

AActor* AAuraGameModeBase::ChoosePlayerStart_Implementation(AController* Player)
{
	UAuraGameInstance* AuraGameInstance = Cast<UAuraGameInstance>(GetGameInstance());
	
	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), Actors);

	if (Actors.Num() > 0)
	{
		AActor* SelectedActor = Actors[0];
		for (AActor* Actor : Actors)
		{
			if (APlayerStart* PlayerStart = Cast<APlayerStart>(Actor))
			{
				if (PlayerStart->PlayerStartTag == AuraGameInstance->PlayerStartTag)
				{
					SelectedActor = PlayerStart;
					break;
				}
			}
		}
		return SelectedActor;
	}

	return nullptr;
}
