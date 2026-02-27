#pragma once

#include "HttpFwd.h"
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AIChatComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChatReply, const FText&, Reply);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class NPC_DIALOGUE_WITH_AI_API UAIChatComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    FString ServerUrl = TEXT("http://127.0.0.1:8000/chat");

    UPROPERTY(BlueprintAssignable, Category = "AI")
    FOnChatReply OnReply;

    UFUNCTION(BlueprintCallable, Category = "AI")
    void SendChat(const FText& Message);
};