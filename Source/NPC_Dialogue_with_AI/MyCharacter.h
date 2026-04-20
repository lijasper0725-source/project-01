#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "HttpFwd.h"
#include "MyCharacter.generated.h"

class UUserWidget;
class UInputAction;
class UAIChatComponent;
class AActor;

UCLASS()
class NPC_DIALOGUE_WITH_AI_API AMyCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AMyCharacter();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SetTalkTarget(AActor* InNPC);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void ClearTalkTarget(AActor* InNPC);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SendPlayerMessage(const FString& Message);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void CloseDialogue();

    UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue")
    void BP_OnDialogueOpened(UUserWidget* Widget);

    UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue")
    void BP_OnDialogueClosed();

    UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue")
    void BP_OnNpcReply(const FText& Reply);

protected:
    virtual void BeginPlay() override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    TSubclassOf<UUserWidget> DialogueWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> InteractAction;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    TObjectPtr<UUserWidget> DialogueWidget = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    TObjectPtr<AActor> CurrentNPC = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    bool bCanTalk = false;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    bool bIsTalking = false;

private:
    void OnInteract(const FInputActionValue& Value);
    void OpenDialogue();
    void SendRequestToPython(const FString& PlayerMessage);
    void OnLLMResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

    UFUNCTION()
    void HandleNpcReply(const FText& Reply);

    UPROPERTY()
    TObjectPtr<UAIChatComponent> CurrentAIChat = nullptr;
};