#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "SaloonGameInstance.generated.h"

UCLASS()
class NPC_DIALOGUE_WITH_AI_API USaloonGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	// 当前时间与天气环境
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World State")
	FString CurrentTime = TEXT("Late at night, a heavy rainstorm is raging outside.");

	// 当前发生的大事件列表
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World State")
	TArray<FString> ActiveEvents;

	// 玩家的近期行为记录
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Context")
	TArray<FString> PlayerRecentActions;

	// 重写 Init 函数，用于在游戏启动时初始化数据
	virtual void Init() override;
};