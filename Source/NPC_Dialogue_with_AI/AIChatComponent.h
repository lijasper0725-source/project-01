#pragma once

#include "HttpFwd.h"
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AIChatComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChatReply, const FText&, Reply);

// 2. 新增：打字机逐字更新委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTypewriterUpdate, const FString&, CurrentlyDisplayedText);

// 3. 新增：打字完成后的委托（可选，用于触发后续动画）
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTypewriterFinished);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class NPC_DIALOGUE_WITH_AI_API UAIChatComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    FString ServerUrl = TEXT("http://127.0.0.1:8000/chat");

    UPROPERTY(BlueprintAssignable, Category = "AI")
    FOnChatReply OnReply;

    // --- 新增打字机属性 ---
    UPROPERTY(BlueprintAssignable, Category = "AI|Typewriter")
    FOnTypewriterUpdate OnTypewriterUpdate;

    UPROPERTY(BlueprintAssignable, Category = "AI|Typewriter")
    FOnTypewriterFinished OnTypewriterFinished;

    UFUNCTION(BlueprintCallable, Category = "AI")
    void SendChat(const FText& Message);

    // --- 新增打字机控制函数 ---
    UFUNCTION(BlueprintCallable, Category = "AI|Typewriter")
    void StartTypewriter(const FString& FullMessage, float CharacterDelay = 0.05f);

private:
    // 处理计时器每秒跳动的逻辑
    void OnTimerTick();

    FTimerHandle TypewriterTimerHandle;
    FString TargetMessage;
    int32 CurrentCharIndex;
};