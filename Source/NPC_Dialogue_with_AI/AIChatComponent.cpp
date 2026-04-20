#include "AIChatComponent.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "UObject/UnrealType.h"
#include "TimerManager.h"
#include "SaloonGameInstance.h"
#include "Kismet/GameplayStatics.h"

static FString GetOwnerStringProperty(AActor* Owner, const FName PropertyName, const FString& DefaultValue)
{
    if (!Owner) return DefaultValue;

    FProperty* Prop = FindFProperty<FProperty>(Owner->GetClass(), PropertyName);
    if (!Prop) return DefaultValue;

    if (FStrProperty* StrProp = CastField<FStrProperty>(Prop))
    {
        return StrProp->GetPropertyValue_InContainer(Owner);
    }

    return DefaultValue;
}

void UAIChatComponent::SendChat(const FText& Message)
{
    AActor* OwnerActor = GetOwner();

    // 1) 从 BP_NPC 读取 npc_id
    FString NpcId = GetOwnerStringProperty(OwnerActor, TEXT("NpcId"), TEXT("Mara"));

    // 2) 准备 JSON 请求
    FString PlayerId = TEXT("Player0");
    TSharedPtr<FJsonObject> JsonObj = MakeShared<FJsonObject>();
    JsonObj->SetStringField(TEXT("player_id"), PlayerId);
    JsonObj->SetStringField(TEXT("npc_id"), NpcId);
    JsonObj->SetStringField(TEXT("message"), Message.ToString());
    
    if (USaloonGameInstance* GI = Cast<USaloonGameInstance>(UGameplayStatics::GetGameInstance(GetWorld())))
    {
        // 塞入当前时间
        JsonObj->SetStringField(TEXT("time"), GI->CurrentTime);

        // 把大事件列表 (ActiveEvents) 转换成 JSON 数组并塞入包裹
        TArray<TSharedPtr<FJsonValue>> EventsArray;
        for (const FString& EventStr : GI->ActiveEvents)
        {
            EventsArray.Add(MakeShareable(new FJsonValueString(EventStr)));
        }
        JsonObj->SetArrayField(TEXT("events"), EventsArray);
    }

    FString Body;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Body);
    FJsonSerializer::Serialize(JsonObj.ToSharedRef(), Writer);

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Req = FHttpModule::Get().CreateRequest();
    Req->SetURL(ServerUrl);
    Req->SetVerb(TEXT("POST"));
    Req->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    Req->SetContentAsString(Body);

    Req->OnProcessRequestComplete().BindLambda(
        [this](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
        {
            if (!bWasSuccessful || !Response.IsValid())
            {
                FString ErrorMsg = TEXT("NPC: (server unreachable)");
                OnReply.Broadcast(FText::FromString(ErrorMsg));
                StartTypewriter(ErrorMsg); // 错误信息也走打字机
                return;
            }

            const FString RespStr = Response->GetContentAsString();
            TSharedPtr<FJsonObject> Obj;
            const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(RespStr);

            if (FJsonSerializer::Deserialize(Reader, Obj) && Obj.IsValid() && Obj->HasField(TEXT("reply")))
            {
                FString FinalReply = Obj->GetStringField(TEXT("reply"));

                // 广播原始完整文本（保留你原有的逻辑）
                OnReply.Broadcast(FText::FromString(FinalReply));

                // --- 核心改动：触发打字机效果 ---
                StartTypewriter(FinalReply, 0.05f);
            }
            else
            {
                FString ErrorMsg = TEXT("NPC: (bad response)");
                OnReply.Broadcast(FText::FromString(ErrorMsg));
                StartTypewriter(ErrorMsg);
            }
        });

    UE_LOG(LogTemp, Warning, TEXT("SendChat => npc_id=%s player_id=%s message=%s"),
        *NpcId, *PlayerId, *Message.ToString());

    Req->ProcessRequest();
}

// --- 打字机功能实现 ---

void UAIChatComponent::StartTypewriter(const FString& FullMessage, float CharacterDelay)
{
    // 如果当前正在打字，先停止旧的计时器
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(TypewriterTimerHandle);
    }

    TargetMessage = FullMessage;
    CurrentCharIndex = 0;

    // 启动计时器，每隔 CharacterDelay 秒执行一次 OnTimerTick
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            TypewriterTimerHandle,
            this,
            &UAIChatComponent::OnTimerTick,
            CharacterDelay,
            true
        );
    }
}

void UAIChatComponent::OnTimerTick()
{
    if (CurrentCharIndex <= TargetMessage.Len())
    {
        // 截取从 0 开始到当前索引的字符串并广播
        FString CurrentDisplay = TargetMessage.Left(CurrentCharIndex);
        OnTypewriterUpdate.Broadcast(CurrentDisplay);

        CurrentCharIndex++;
    }
    else
    {
        // 打字结束，清理计时器
        if (GetWorld())
        {
            GetWorld()->GetTimerManager().ClearTimer(TypewriterTimerHandle);
        }
        OnTypewriterFinished.Broadcast();
    }
}