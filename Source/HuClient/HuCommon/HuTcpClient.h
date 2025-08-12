#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HuCommon.h"
#include "HuSerial.h"
#include "HuTcpClient.generated.h"

UENUM( BlueprintType )
enum class EHuConnectionState : uint8
{
    Connected       UMETA( DisplayName = "Connected" ),
    Disconnected    UMETA( DisplayName = "Disconnected" ),
    Reconnected     UMETA( DisplayName = "Reconnected" )
};

UENUM( BlueprintType )
enum class EHuReceiveResult : uint8
{
    Succeed    UMETA( DisplayName = "Succeed" ),
    Failed     UMETA( DisplayName = "Failed" ),
    Pending    UMETA( DisplayName = "Pending" )
};

// 메시지 헤더 정보
#pragma pack( push, 1 )
USTRUCT()
struct HUCLIENT_API FHuHeader
{
    GENERATED_BODY()

    // 메시지 크기
    HuMessageSize MessageSize { 0 };

    // 메시지 식별자
    HuMessageId MessageId { 0 };

    // 헤더 정보를 초기화 한다.
    void Clear()
    {
        MessageSize = 0;
        MessageId = 0;
    }

    // 헤더가 완성되었는지 검사한다.
    bool IsComplete() const
    {
        return ( MessageSize > 0 ) && ( MessageId != 0 );
    }

    // 헤더가 유효한지 검사한다.
    bool IsValid( const HuMessageSize MaxMessageSize ) const
    {
        return IsComplete() && ( MessageSize <= MaxMessageSize );
    }
};
#pragma pack( pop )

HU_STATIC_ASSERT( sizeof( FHuHeader ) == 8 );

// 메시지 정보
USTRUCT()
struct HUCLIENT_API FHuMessage
{
    GENERATED_BODY()

    using IBuffer     = hu::Buffer;
    using ISerializer = hu::BinSerializer;

    // 헤더 정보
    FHuHeader Header;

    // 입출력 진행 크기
    HuMessageSize PendingSize { 0 };

    // 메시지 버퍼
    IBuffer Buffer;

    // 메시지 정보를 초기화 한다.
    void Clear()
    {
        Header.Clear();

        PendingSize = 0;
        
        Buffer.clear();
    }

    // 메시지를 초기화 한다.
    bool Init( const HuMessageSize MaxMessageSize )
    {
        if ( Header.IsValid( MaxMessageSize ) == false )
        {
            return false;
        }

        PendingSize = 0;

        Buffer.clear();
        Buffer.resize( Header.MessageSize );

        return true;
    }

    // 메시지가 완성되었는지 검사한다.
    bool IsComplete() const
    {
        return Header.IsComplete() && ( PendingSize >= Header.MessageSize );
    }

    // 남은 메시지 크기를 얻는다.
    HuMessageSize GetRemainSize() const
    {
        return ( Header.MessageSize - PendingSize );
    }

    // 메시지 버퍼 데이터를 얻는다.
    HuByte* GetData()
    {
        return ( Buffer.data() + PendingSize );
    }

    // 메시지 버퍼 데이터를 얻는다.
    const HuByte* GetData() const
    {
        return ( Buffer.data() + PendingSize );
    }

    // 객체를 쓴다.
    bool Write( const auto& Obj )
    {
        if ( ISerializer::Write( Obj, Buffer ) == false )
        {
            return false;
        }

        Header.MessageId   = Obj.kTypeId;
        Header.MessageSize = static_cast<HuMessageSize>( Buffer.size() );

        return true;
    }

    // 객체를 읽는다.
    bool Read( auto& Obj ) const
    {
        if ( Header.MessageId != Obj.kTypeId )
        {
            return false;
        }

        return ISerializer::Read( Buffer, Obj );
    }
};

DECLARE_DYNAMIC_DELEGATE_OneParam( FHuOnConnectionStatusChanged, const EHuConnectionState, ConnectionState );
DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam( bool, FHuOnMessageReceived, const FHuMessage&, Message );

UCLASS()
class HUCLIENT_API AHuTcpClient : public AActor
{
    GENERATED_BODY()

public:
    AHuTcpClient();

public:
    // 서버에 연결 되었는지 검사한다.
    bool IsConnected() const;

    // 서버에 연결한다.
    bool ConnectToServer( const FString& ServerIP, const int32 Port );

    // 서버 연결을 끊는다.
    void DisconnectFromServer();

    // 메시지를 보낸다.
    bool SendMessage( const FHuMessage& Message );

    // 객체를 보낸다.
    bool SendObj( const auto& Obj )
    {
        FHuMessage Message;
        if ( Message.Write( Obj ) == false )
        {
            return false;
        }

        return SendMessage( Message );
    }

public:
    virtual void Tick( float DeltaTime ) override;

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay( const EEndPlayReason::Type EndPlayReason ) override;

private:
    // 클라이언트를 초기화 한다.
    bool Init();

    // 클라이언트를 정리한다.
    void Clear();

    // 소켓을 해지한다.
    void DeleteSocket();

    // 서버에 재연결을 시도한다.
    void ReconnectToServer();

    // 지정된 크기만큼 데이터를 전송한다.
    bool Send( const HuByte* const Data, const int32 Size );

    // 데이터를 전송한다.
    bool SendData( const auto& Data )
    {
        return Send( reinterpret_cast<const HuByte*>( &Data ), sizeof( Data ) );
    }

    // 지정된 크기만큼 데이터를 받는다.
    EHuReceiveResult Recv( HuByte* const Data, const int32 Size );

    // 데이터를 받는다.
    EHuReceiveResult RecvData( auto& Data )
    {
        return Recv( reinterpret_cast<HuByte*>( &Data ), sizeof( Data ) );
    }

    // 메시지 헤더를 받는다.
    EHuReceiveResult RecvHeader();

    // 메시지 하나를 받기 시도한다.
    EHuReceiveResult RecvOnce();

    // 메시지 받기 틱을 처리한다.
    void RecvTick();

public:
    UPROPERTY( BlueprintReadOnly, Category = "HuCommon" )
    FHuOnMessageReceived OnMessageReceived;

    UPROPERTY( BlueprintReadOnly, Category = "HuCommon" )
    FHuOnConnectionStatusChanged OnConnectionStatusChanged;

    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "HuCommon" )
    int32 SendBufferSize { 1024 * 1024 };

    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "HuCommon" )
    int32 ReceiveBufferSize { 1024 * 1024 };

    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "HuCommon" )
    int32 MaxMessageSize { 1024 * 1024 };

    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "HuCommon" )
    float ReceiveInterval { 0.1f };

    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "HuCommon" )
    bool bAutoReconnect { true };

    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "HuCommon" )
    float ReconnectInterval { 3.0f };

private:
    FSocket*      Socket { nullptr };
    FString       LastServerIP;
    int32         LastServerPort { 0 };
    float         LastReconnectTime { 0.0f };
    FTimerHandle  RecvTimerHandle;
    FHuMessage    RecvMsg;
};
