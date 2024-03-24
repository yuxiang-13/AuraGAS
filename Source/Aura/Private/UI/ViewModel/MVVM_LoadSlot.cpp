// yyyyyxxxxxx


#include "UI/ViewModel/MVVM_LoadSlot.h"

void UMVVM_LoadSlot::InitializeSlot()
{
	// 根据 数据 更改Slot的Index
	// 实际，每当加载数据时，都会进行广播
	SetWidgetSwitcherIndex.Broadcast(1);
}

void UMVVM_LoadSlot::SetLoadSlotName(FString InLoadSlotName)
{
	// 使用MVVM宏 设置属性
	UE_MVVM_SET_PROPERTY_VALUE(LoadSlotName, InLoadSlotName);
}
