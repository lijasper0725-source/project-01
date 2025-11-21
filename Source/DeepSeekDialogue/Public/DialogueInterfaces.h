#pragma once
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DialogueInterfaces.generated.h"

UCLASS()
class UDialogueInterfaces : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category="DeepSeek")
    static void RequestDialogueForNPC(const FString& NPCId, const FString& Context, const FDeepSeekResponseDelegate& Callback);
};
