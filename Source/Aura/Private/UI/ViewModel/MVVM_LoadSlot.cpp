// yyyyyxxxxxx


#include "UI/ViewModel/MVVM_LoadSlot.h"

void UMVVM_LoadSlot::InitializeSlot()
{
	// 根据 数据 更改Slot的Index
	// 实际，每当加载数据时，都会进行广播
	SetWidgetSwitcherIndex.Broadcast(1);
}
