#include "MyCharacter.h"
#include "AIChatComponent.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "GameFramework/PlayerController.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Serialization/JsonSerializer.h"
#include "JsonObjectConverter.h"
#include "SaloonGameInstance.h"
#include "Kismet/GameplayStatics.h"

AMyCharacter::AMyCharacter()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AMyCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        UWidgetBlueprintLibrary::SetInputMode_GameOnly(PC);
        PC->SetShowMouseCursor(false);
    }

    bCanTalk = false;
    bIsTalking = false;
    CurrentNPC = nullptr;
    DialogueWidget = nullptr;
    CurrentAIChat = nullptr;
}

void AMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        if (InteractAction)
        {
            EnhancedInput->BindAction(InteractAction, ETriggerEvent::Started, this, &AMyCharacter::OnInteract);
        }
    }
}

void AMyCharacter::OnInteract(const FInputActionValue& Value)
{
    if (!bCanTalk || !CurrentNPC)
    {
        return;
    }

    if (bIsTalking)
    {
        CloseDialogue();
    }
    else
    {
        OpenDialogue();
    }
}

void AMyCharacter::OpenDialogue()
{
    if (!bCanTalk || !CurrentNPC || bIsTalking)
    {
        return;
    }

    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC || !DialogueWidgetClass)
    {
        return;
    }

    DialogueWidget = CreateWidget<UUserWidget>(PC, DialogueWidgetClass);
    if (!DialogueWidget)
    {
        return;
    }

    DialogueWidget->AddToViewport();

    UWidgetBlueprintLibrary::SetInputMode_GameAndUIEx(
        PC,
        DialogueWidget,
        EMouseLockMode::DoNotLock,
        false
    );

    PC->SetShowMouseCursor(true);

    CurrentAIChat = CurrentNPC->FindComponentByClass<UAIChatComponent>();
    if (CurrentAIChat)
    {
        CurrentAIChat->OnReply.RemoveDynamic(this, &AMyCharacter::HandleNpcReply);
        CurrentAIChat->OnReply.AddDynamic(this, &AMyCharacter::HandleNpcReply);
    }

    bIsTalking = true;

    BP_OnDialogueOpened(DialogueWidget);
}

void AMyCharacter::CloseDialogue()
{
    if (CurrentAIChat)
    {
        CurrentAIChat->OnReply.RemoveDynamic(this, &AMyCharacter::HandleNpcReply);
        CurrentAIChat = nullptr;
    }

    if (DialogueWidget)
    {
        DialogueWidget->RemoveFromParent();
        DialogueWidget = nullptr;
    }

    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        UWidgetBlueprintLibrary::SetInputMode_GameOnly(PC);
        PC->SetShowMouseCursor(false);
    }

    bIsTalking = false;

    BP_OnDialogueClosed();
}

void AMyCharacter::SetTalkTarget(AActor* InNPC)
{
    CurrentNPC = InNPC;
    bCanTalk = (InNPC != nullptr);

    if (InNPC)
    {
        FString FoundName;
        FProperty* Prop = InNPC->GetClass()->FindPropertyByName(TEXT("NpcId"));
        if (Prop)
        {
            if (FStrProperty* StrProp = CastField<FStrProperty>(Prop))
            {
                FoundName = StrProp->GetPropertyValue_InContainer(InNPC);
            }
        }

        if (!FoundName.IsEmpty() && DialogueWidget)
        {
            
        }
    }
}

void AMyCharacter::ClearTalkTarget(AActor* InNPC)
{
    if (CurrentNPC == InNPC)
    {
        CloseDialogue();
        CurrentNPC = nullptr;
        bCanTalk = false;
    }
}

void AMyCharacter::SendPlayerMessage(const FString& Message)
{
    // 1. 如果玩家发的消息是空的，直接退出
    if (Message.IsEmpty())
    {
        return;
    }

    SendRequestToPython(Message);
}

void AMyCharacter::HandleNpcReply(const FText& Reply)
{
    BP_OnNpcReply(Reply);
}

void AMyCharacter::SendRequestToPython(const FString& PlayerMessage)
{
    USaloonGameInstance* GI = Cast<USaloonGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
    if (!GI)
    {
        // 如果没找到，在屏幕上打出巨大的红字报警！
        GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, TEXT("【拦截警告】：GameInstance配置错误！引擎现在用的不是SaloonGameInstance！"));
        return;
    }

    // 如果成功了，打出绿字提示
    //GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("【通行】：成功拿到大本营数据，正在向 Python 发射！"));
    
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

    FString NpcName = TEXT("Brewer"); // 默认给个名字兜底
    if (CurrentNPC)
    {
        // 从当前正在对话的 NPC 身上，读取它配置好的 NpcId
        FProperty* Prop = CurrentNPC->GetClass()->FindPropertyByName(TEXT("NpcId"));
        if (FStrProperty* StrProp = CastField<FStrProperty>(Prop))
        {
            NpcName = StrProp->GetPropertyValue_InContainer(CurrentNPC);
        }
    }
    // 把正确的身份标识发给 Python！这对应了 Python 里的 npc_id
    JsonObject->SetStringField(TEXT("npc_id"), NpcName);
    // 塞入玩家说的话
    JsonObject->SetStringField(TEXT("message"), PlayerMessage);
    // 塞入当前时间
    JsonObject->SetStringField(TEXT("time"), GI->CurrentTime);

    // 把大事件列表 (ActiveEvents) 转换成 JSON 数组
    TArray<TSharedPtr<FJsonValue>> EventsArray;
    for (const FString& EventStr : GI->ActiveEvents)
    {
        EventsArray.Add(MakeShareable(new FJsonValueString(EventStr)));
    }
    JsonObject->SetArrayField(TEXT("events"), EventsArray);

    // 3. 将 JSON 对象压缩成一串字符串文本
    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

    // 4. 创建 HTTP POST 请求并发射！
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();

    // !!! 这里填你 Python FastAPI 的具体接口地址 !!!
    Request->SetURL("http://127.0.0.1:8000/chat");
    Request->SetVerb("POST");
    Request->SetHeader("Content-Type", "application/json");
    Request->SetContentAsString(OutputString);
    Request->OnProcessRequestComplete().BindUObject(this, &AMyCharacter::OnLLMResponseReceived);

    // 真正的发送动作
    Request->ProcessRequest();
}

void AMyCharacter::OnLLMResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    if (bWasSuccessful && Response.IsValid())
    {
        // 获取 Python 传回来的原始 JSON 文本
        FString ResponseString = Response->GetContentAsString();

        TSharedPtr<FJsonObject> JsonObject;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseString);

        // 如果成功解析了 JSON
        if (FJsonSerializer::Deserialize(Reader, JsonObject))
        {
            // 假设你的 Python 返回的格式里有一个字段叫 "reply"
            // 例如：{"reply": "滚出去，外乡人！"}
            FString NpcReply = JsonObject->GetStringField(TEXT("reply"));

            // 调用你之前写好的处理函数！这会在屏幕上或蓝图里触发！
            HandleNpcReply(FText::FromString(NpcReply));
        }
    }
    else
    {
        // 如果网络不通，打印个错误方便我们排查
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Network request failed! Please check if the Python server is running!"));
    }
}