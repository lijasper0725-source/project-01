#include "SaloonGameInstance.h"

void USaloonGameInstance::Init()
{
	// 必须调用父类的 Init，保证引擎底层逻辑正常运行
	Super::Init();

	// 游戏一启动，默认塞入这几个世界事件
	ActiveEvents.Add(TEXT("The old town sheriff was found dead in the desert yesterday; the town is currently in a state of anarchy."));
	ActiveEvents.Add(TEXT("An unmarked, black-armored train just pulled into the station, and no one knows what is inside."));
	ActiveEvents.Add(TEXT("Strange, muffled knocking sounds have recently been echoing from the saloon's basement."));

	// 玩家初始行为
	PlayerRecentActions.Add(TEXT("Just pushed open the saloon's swinging doors."));
	PlayerRecentActions.Add(TEXT("Carrying a strong scent of gunpowder and dried blood."));
}