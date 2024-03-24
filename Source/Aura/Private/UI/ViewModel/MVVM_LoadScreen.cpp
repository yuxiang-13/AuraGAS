// yyyyyxxxxxx


#include "UI/ViewModel/MVVM_LoadScreen.h"

#include "Aura/AuraGameModeBase.h"
#include "Game/LoadScreenSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "UI/ViewModel/MVVM_LoadSlot.h"

void UMVVM_LoadScreen::InitializeLoadSlot()
{
	LoadSlot_0 = NewObject<UMVVM_LoadSlot>(this, LoadSlotViewModelClass);
	LoadSlot_0->SetLoadSlotName(FString("LoadSlot_0"));
	LoadSlot_0->SlotIndex = 0;
	LoadSlots.Add(0, LoadSlot_0);
	
	LoadSlot_1 = NewObject<UMVVM_LoadSlot>(this, LoadSlotViewModelClass);
	LoadSlot_1->SetLoadSlotName(FString("LoadSlot_1"));
	LoadSlot_1->SlotIndex = 1;
	LoadSlots.Add(1, LoadSlot_1);
	
	LoadSlot_2 = NewObject<UMVVM_LoadSlot>(this, LoadSlotViewModelClass);
	LoadSlot_2->SetLoadSlotName(FString("LoadSlot_2"));
	LoadSlot_2->SlotIndex = 2;
	LoadSlots.Add(2, LoadSlot_2);

}

UMVVM_LoadSlot* UMVVM_LoadScreen::GetLoadSlotViewModelByIndex(int32 Index) const
{
	return LoadSlots.FindChecked(Index);
}

void UMVVM_LoadScreen::NewSlotButtonPressed(int32 Slot, const FString& EnteredName)
{
	AAuraGameModeBase* AuraGameMode = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(this));

	// 这种无法触发，必须是触发Set函数
	// LoadSlots[Slot]->PlayerName = EnteredName;
	LoadSlots[Slot]->SetPlayerName(EnteredName);

	LoadSlots[Slot]->SlotStatus = ESaveSlotStates::Taken;

	// 这种无法触发，必须是触发Set函数
	LoadSlots[Slot]->SetMapName(AuraGameMode->DefaultMapName);

	AuraGameMode->SavaSlotData(LoadSlots[Slot], Slot);
	LoadSlots[Slot]->InitializeSlot();
}

void UMVVM_LoadScreen::NewGameButtonPressed(int32 Slot)
{
	LoadSlots[Slot]->SetWidgetSwitcherIndex.Broadcast(1);
}

void UMVVM_LoadScreen::SelectSlotButtonPressed(int32 Slot)
{
	SlotSelected.Broadcast();
	for (TTuple<int, UMVVM_LoadSlot*> LoadSlot: LoadSlots)
	{
		if (LoadSlot.Key == Slot)
		{
			LoadSlot.Value->EnableSelectSlotButton.Broadcast(false);
		} else
		{
			LoadSlot.Value->EnableSelectSlotButton.Broadcast(true);
		}
	}

	SelectedSlot = LoadSlots[Slot];
}

void UMVVM_LoadScreen::DeleteButtonOnPressed()
{
	// 删除数据
	if (IsValid(SelectedSlot))
	{
		AAuraGameModeBase::DeleteSlot(SelectedSlot->GetLoadSlotName(), SelectedSlot->SlotIndex);
		SelectedSlot->SlotStatus = ESaveSlotStates::Vacant;
		SelectedSlot->InitializeSlot();

		SelectedSlot->EnableSelectSlotButton.Broadcast(true);
	}
}

void UMVVM_LoadScreen::PlayButtonPressed()
{
	AAuraGameModeBase* AuraGameMode = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(this));
	if (SelectedSlot)
	{
		AuraGameMode->TravelToMap(SelectedSlot);
	}
}

void UMVVM_LoadScreen::LoadData()
{
	AAuraGameModeBase* AuraGameMode = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(this));
	for (TTuple<int, UMVVM_LoadSlot*> LoadSlot: LoadSlots)
	{
		ULoadScreenSaveGame* SaveObject = AuraGameMode->GetSaveSlotData(LoadSlot.Value->GetLoadSlotName(), LoadSlot.Key);
		const FString PlayerName = SaveObject->PlayerName;
		// 触发 属性通知
		LoadSlot.Value->SetPlayerName(PlayerName);
		// 获取 状态存储 状态
		TEnumAsByte<ESaveSlotStates> SaveSlotStatus = SaveObject->SaveSlotStatus;
		LoadSlot.Value->SlotStatus = SaveSlotStatus;
		// 触发 属性通知
		const FString MapName = SaveObject->MapName;
		LoadSlot.Value->SetMapName(MapName);
		
		
		// 触发自己写的DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSetWidgetSwitcherIndex, int32, WidgetSwitcherIndex);
		LoadSlot.Value->InitializeSlot();
	}
}
