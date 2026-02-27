#include "AIChatComponent.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "UObject/UnrealType.h"

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

    // 1) Read npc id from BP_NPC (string variable "NpcId")
    FString NpcId = GetOwnerStringProperty(OwnerActor, TEXT("NpcId"), TEXT("Mara"));

    // 2) Player id (for now just hardcode)
    FString PlayerId = TEXT("Player0");
    TSharedPtr<FJsonObject> JsonObj = MakeShared<FJsonObject>();
    JsonObj->SetStringField(TEXT("player_id"), PlayerId);
    JsonObj->SetStringField(TEXT("npc_id"), NpcId);
    JsonObj->SetStringField(TEXT("message"), Message.ToString());

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
                OnReply.Broadcast(FText::FromString(TEXT("NPC: (server unreachable)")));
                return;
            }

            const FString RespStr = Response->GetContentAsString();

            TSharedPtr<FJsonObject> Obj;
            const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(RespStr);
            if (FJsonSerializer::Deserialize(Reader, Obj) && Obj.IsValid() && Obj->HasField(TEXT("reply")))
            {
                OnReply.Broadcast(FText::FromString(Obj->GetStringField(TEXT("reply"))));
            }
            else
            {
                OnReply.Broadcast(FText::FromString(TEXT("NPC: (bad response)")));
            }
        });

    UE_LOG(LogTemp, Warning, TEXT("SendChat => npc_id=%s player_id=%s message=%s"),
        *NpcId, *PlayerId, *Message.ToString());

    Req->ProcessRequest();
}
