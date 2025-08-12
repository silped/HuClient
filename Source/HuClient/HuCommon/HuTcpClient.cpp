#include "HuTcpClient.h"

#include "SocketSubsystem.h"
#include "Common/TcpSocketBuilder.h"

AHuTcpClient::AHuTcpClient()
{
    PrimaryActorTick.bCanEverTick = true;
}

bool AHuTcpClient::IsConnected() const
{
    return ( Socket != nullptr ) && ( Socket->GetConnectionState() == SCS_Connected );
}

bool AHuTcpClient::ConnectToServer( const FString& ServerIP, const int32 Port )
{
    if ( HU_CHECK_TRUE( IsConnected() ) )
    {
        return false;
    }

    ISocketSubsystem* const SocketSubsystem = ISocketSubsystem::Get( PLATFORM_SOCKETSUBSYSTEM );
    if ( HU_CHECK_TRUE( SocketSubsystem == nullptr ) )
    {
        return false;
    }

    const FString SocketDesc = FString::Printf( TEXT( "TcpSocket %s:%d" ), *ServerIP, Port );
    Socket = FTcpSocketBuilder( SocketDesc ).WithSendBufferSize( SendBufferSize ).WithReceiveBufferSize( ReceiveBufferSize );
    if ( HU_CHECK_TRUE( Socket == nullptr ) )
    {
        return false;
    }

    Socket->SetNoDelay( true );

    TSharedRef<FInternetAddr> Addr = SocketSubsystem->CreateInternetAddr();
    bool bIsValid = false;
    Addr->SetIp( *ServerIP, bIsValid );
    Addr->SetPort( Port );
    if ( HU_CHECK_TRUE( bIsValid == false ) )
    {
        DeleteSocket();
        return false;
    }

    if ( HU_CHECK_TRUE( Socket->Connect( *Addr ) == false ) )
    {
        DeleteSocket();
        return false;
    }

    if ( Socket->GetConnectionState() != SCS_Connected )
    {
        DeleteSocket();
        return false;
    }

    LastServerIP = ServerIP;
    LastServerPort = Port;

    HU_LOG_SUCCESS();

    OnConnectionStatusChanged.ExecuteIfBound( EHuConnectionState::Connected );
    return true;
}

void AHuTcpClient::DisconnectFromServer()
{
    if ( HU_CHECK_TRUE( IsConnected() == false ) )
    {
        return;
    }

    DeleteSocket();

    RecvMsg.Clear();

    HU_LOG_SUCCESS();

    OnConnectionStatusChanged.ExecuteIfBound( EHuConnectionState::Disconnected );
}

bool AHuTcpClient::SendMessage( const FHuMessage& Message )
{
    if ( HU_CHECK_TRUE( Message.Header.IsValid( MaxMessageSize ) == false ) )
    {
        return false;
    }

    if ( HU_CHECK_TRUE( SendData( Message.Header ) == false ) )
    {
        return false;
    }

    if ( HU_CHECK_TRUE( Send( Message.GetData(), Message.Header.MessageSize ) == false ) )
    {
        return false;
    }

    return true;
}

void AHuTcpClient::Tick( float DeltaTime )
{
    Super::Tick( DeltaTime );

    if ( bAutoReconnect && ( IsConnected() == false ) )
    {
        if ( ( LastServerIP.IsEmpty() == false ) && ( LastServerPort > 0 ) )
        {
            const float CurrentTime = GetWorld()->GetTimeSeconds();
            if ( ( CurrentTime - LastReconnectTime ) > ReconnectInterval )
            {
                ReconnectToServer();
                LastReconnectTime = CurrentTime;
            }
        }
    }
}

void AHuTcpClient::BeginPlay()
{
    Super::BeginPlay();

    Init();
}

void AHuTcpClient::EndPlay( const EEndPlayReason::Type EndPlayReason )
{
    Clear();

    Super::EndPlay( EndPlayReason );
}

bool AHuTcpClient::Init()
{
    if ( HU_CHECK_TRUE( SendBufferSize <= 1024 ) )
    {
        return false;
    }

    if ( HU_CHECK_TRUE( ReceiveBufferSize <= 1024 ) )
    {
        return false;
    }

    if ( HU_CHECK_TRUE( MaxMessageSize <= 1024 ) )
    {
        return false;
    }

    if ( HU_CHECK_TRUE( ReceiveInterval <= 0.0f ) )
    {
        return false;
    }

    if ( bAutoReconnect )
    {
        if ( HU_CHECK_TRUE( ReconnectInterval <= 0.0f ) )
        {
            return false;
        }
    }

    RecvMsg.Buffer.reserve( MaxMessageSize );

    GetWorld()->GetTimerManager().SetTimer( RecvTimerHandle, this, &AHuTcpClient::RecvTick, ReceiveInterval, true );

    return true;
}

void AHuTcpClient::Clear()
{
    DisconnectFromServer();

    GetWorld()->GetTimerManager().ClearTimer( RecvTimerHandle );
}

void AHuTcpClient::DeleteSocket()
{
    if ( HU_CHECK_TRUE( Socket == nullptr ) )
    {
        return;
    }

    Socket->Close();
    ISocketSubsystem::Get( PLATFORM_SOCKETSUBSYSTEM )->DestroySocket( Socket );
    Socket = nullptr;
}

void AHuTcpClient::ReconnectToServer()
{
    if ( HU_CHECK_TRUE( IsConnected() ) )
    {
        return;
    }

    if ( Socket == nullptr )
    {
        const FString SocketDesc = FString::Printf( TEXT( "TcpSocket %s:%d" ), *LastServerIP, LastServerPort );
        Socket = FTcpSocketBuilder( SocketDesc ).WithSendBufferSize( SendBufferSize ).WithReceiveBufferSize( ReceiveBufferSize );
        if ( HU_CHECK_TRUE( Socket == nullptr ) )
        {
            return;
        }

        Socket->SetNoDelay( true );
    }

    ISocketSubsystem* const SocketSubsystem = ISocketSubsystem::Get( PLATFORM_SOCKETSUBSYSTEM );
    if ( HU_CHECK_TRUE( SocketSubsystem == nullptr ) )
    {
        return;
    }

    TSharedRef<FInternetAddr> Addr = SocketSubsystem->CreateInternetAddr();
    bool bIsValid = false;
    Addr->SetIp( *LastServerIP, bIsValid );
    Addr->SetPort( LastServerPort );
    if ( HU_CHECK_TRUE( bIsValid == false ) )
    {
        return;
    }

    if ( HU_CHECK_TRUE( Socket->Connect( *Addr ) == false ) )
    {
        return;
    }

    if ( Socket->GetConnectionState() != SCS_Connected )
    {
        return;
    }

    HU_LOG_SUCCESS();

    OnConnectionStatusChanged.ExecuteIfBound( EHuConnectionState::Reconnected );
    return;
}

bool AHuTcpClient::Send( const HuByte* const Data, const int32 Size )
{
    if ( HU_CHECK_TRUE( IsConnected() == false ) )
    {
        return false;
    }

    int32 TotalSendSize = 0;

    while ( TotalSendSize < Size )
    {
        const int32 RemainSize = Size - TotalSendSize;
        int32 SendSize = 0;

        if ( HU_CHECK_TRUE( Socket->Send( Data + TotalSendSize, RemainSize, SendSize ) == false ) )
        {
            return false;
        }

        if ( HU_CHECK_TRUE( SendSize == 0 ) )
        {
            return false;
        }

        TotalSendSize += SendSize;
    }

    return true;
}

EHuReceiveResult AHuTcpClient::Recv( HuByte* const Data, const int32 Size )
{
    int32 TotalSize = 0;

    while ( TotalSize < Size )
    {
        const int32 RemainSize = Size - TotalSize;
        int32 RecvSize = 0;

        if ( HU_CHECK_TRUE( Socket->Recv( Data + TotalSize, RemainSize, RecvSize ) == false ) )
        {
            return EHuReceiveResult::Failed;
        }

        if ( RecvSize == 0 )
        {
            return EHuReceiveResult::Pending;
        }

        TotalSize += RecvSize;
    }

    return EHuReceiveResult::Succeed;
}

EHuReceiveResult AHuTcpClient::RecvHeader()
{
    const EHuReceiveResult Result = RecvData( RecvMsg.Header );
    if ( Result != EHuReceiveResult::Succeed )
    {
        return Result;
    }

    if ( HU_CHECK_TRUE( RecvMsg.Init( MaxMessageSize ) == false ) )
    {
        return EHuReceiveResult::Failed;
    }

    return EHuReceiveResult::Succeed;
}

EHuReceiveResult AHuTcpClient::RecvOnce()
{
    if ( RecvMsg.Header.IsComplete() == false )
    {
        const EHuReceiveResult Result = RecvHeader();
        if ( Result != EHuReceiveResult::Succeed )
        {
            return Result;
        }
    }

    const int32 RemainSize = RecvMsg.GetRemainSize();
    if ( RemainSize > 0 )
    {
        int32 RecvSize = 0;

        if ( HU_CHECK_TRUE( Socket->Recv( RecvMsg.GetData(), RemainSize, RecvSize ) == false ) )
        {
            return EHuReceiveResult::Failed;
        }

        if ( HU_CHECK_TRUE( RecvSize == 0 ) )
        {
            return EHuReceiveResult::Failed;
        }

        RecvMsg.PendingSize += RecvSize;

        if ( RecvMsg.IsComplete() == false )
        {
            return EHuReceiveResult::Pending;
        }
    }

    return EHuReceiveResult::Succeed;
}

void AHuTcpClient::RecvTick()
{
    while ( IsConnected() )
    {
        const EHuReceiveResult Result = RecvOnce();
        if ( Result == EHuReceiveResult::Succeed )
        {
            if ( OnMessageReceived.Execute( RecvMsg ) == false )
            {
                DisconnectFromServer();
                return;
            }

            RecvMsg.Clear();
            continue;
        }
        else if ( Result == EHuReceiveResult::Failed )
        {
            DisconnectFromServer();
            return;
        }
        else if ( Result == EHuReceiveResult::Pending )
        {
            return;
        }
    }
}
