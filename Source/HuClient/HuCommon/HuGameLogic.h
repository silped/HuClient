#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HuTcpClient.h"
#include "HuGameLogic.generated.h"

USTRUCT()
struct HUCLIENT_API FHuLoginCharacterResult
{
    GENERATED_BODY()

    FString Error;
    FString Name;
};

DECLARE_DYNAMIC_DELEGATE_OneParam( FHuOnLoginCharacter, const FHuLoginCharacterResult&, Result );

UCLASS()
class HUCLIENT_API AHuGameLogic : public AActor
{
    GENERATED_BODY()

public:
    AHuGameLogic();

public:
    bool LoginCharacter( const FString& Name );

public:
    virtual void Tick( float DeltaTime ) override;

protected:
    virtual void BeginPlay() override;

private:
    UFUNCTION()
    void OnConnectionStatusChanged( const EHuConnectionState ConnectionState );

    UFUNCTION()
    bool OnMessageReceived( const FHuMessage& Message );

public:
    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "HuCommon" )
    FString ServerIP { TEXT( "127.0.0.1" ) };

    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "HuCommon" )
    int32 Port { 10001 };

    UPROPERTY( BlueprintReadOnly, Category = "HuCommon" )
    FHuOnLoginCharacter OnLoginCharacter;

private:
    UPROPERTY()
    AHuTcpClient* TcpClient { nullptr };
};
