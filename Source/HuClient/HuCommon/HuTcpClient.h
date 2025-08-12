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

// �޽��� ��� ����
#pragma pack( push, 1 )
USTRUCT()
struct HUCLIENT_API FHuHeader
{
    GENERATED_BODY()

    // �޽��� ũ��
    HuMessageSize MessageSize { 0 };

    // �޽��� �ĺ���
    HuMessageId MessageId { 0 };

    // ��� ������ �ʱ�ȭ �Ѵ�.
    void Clear()
    {
        MessageSize = 0;
        MessageId = 0;
    }

    // ����� �ϼ��Ǿ����� �˻��Ѵ�.
    bool IsComplete() const
    {
        return ( MessageSize > 0 ) && ( MessageId != 0 );
    }

    // ����� ��ȿ���� �˻��Ѵ�.
    bool IsValid( const HuMessageSize MaxMessageSize ) const
    {
        return IsComplete() && ( MessageSize <= MaxMessageSize );
    }
};
#pragma pack( pop )

HU_STATIC_ASSERT( sizeof( FHuHeader ) == 8 );

// �޽��� ����
USTRUCT()
struct HUCLIENT_API FHuMessage
{
    GENERATED_BODY()

    using IBuffer     = hu::Buffer;
    using ISerializer = hu::BinSerializer;

    // ��� ����
    FHuHeader Header;

    // ����� ���� ũ��
    HuMessageSize PendingSize { 0 };

    // �޽��� ����
    IBuffer Buffer;

    // �޽��� ������ �ʱ�ȭ �Ѵ�.
    void Clear()
    {
        Header.Clear();

        PendingSize = 0;
        
        Buffer.clear();
    }

    // �޽����� �ʱ�ȭ �Ѵ�.
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

    // �޽����� �ϼ��Ǿ����� �˻��Ѵ�.
    bool IsComplete() const
    {
        return Header.IsComplete() && ( PendingSize >= Header.MessageSize );
    }

    // ���� �޽��� ũ�⸦ ��´�.
    HuMessageSize GetRemainSize() const
    {
        return ( Header.MessageSize - PendingSize );
    }

    // �޽��� ���� �����͸� ��´�.
    HuByte* GetData()
    {
        return ( Buffer.data() + PendingSize );
    }

    // �޽��� ���� �����͸� ��´�.
    const HuByte* GetData() const
    {
        return ( Buffer.data() + PendingSize );
    }

    // ��ü�� ����.
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

    // ��ü�� �д´�.
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
    // ������ ���� �Ǿ����� �˻��Ѵ�.
    bool IsConnected() const;

    // ������ �����Ѵ�.
    bool ConnectToServer( const FString& ServerIP, const int32 Port );

    // ���� ������ ���´�.
    void DisconnectFromServer();

    // �޽����� ������.
    bool SendMessage( const FHuMessage& Message );

    // ��ü�� ������.
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
    // Ŭ���̾�Ʈ�� �ʱ�ȭ �Ѵ�.
    bool Init();

    // Ŭ���̾�Ʈ�� �����Ѵ�.
    void Clear();

    // ������ �����Ѵ�.
    void DeleteSocket();

    // ������ �翬���� �õ��Ѵ�.
    void ReconnectToServer();

    // ������ ũ�⸸ŭ �����͸� �����Ѵ�.
    bool Send( const HuByte* const Data, const int32 Size );

    // �����͸� �����Ѵ�.
    bool SendData( const auto& Data )
    {
        return Send( reinterpret_cast<const HuByte*>( &Data ), sizeof( Data ) );
    }

    // ������ ũ�⸸ŭ �����͸� �޴´�.
    EHuReceiveResult Recv( HuByte* const Data, const int32 Size );

    // �����͸� �޴´�.
    EHuReceiveResult RecvData( auto& Data )
    {
        return Recv( reinterpret_cast<HuByte*>( &Data ), sizeof( Data ) );
    }

    // �޽��� ����� �޴´�.
    EHuReceiveResult RecvHeader();

    // �޽��� �ϳ��� �ޱ� �õ��Ѵ�.
    EHuReceiveResult RecvOnce();

    // �޽��� �ޱ� ƽ�� ó���Ѵ�.
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
