#include "HuGameLogic.h"
#include "HuPacket.h"

AHuGameLogic::AHuGameLogic()
{
    PrimaryActorTick.bCanEverTick = true;

}

bool AHuGameLogic::LoginCharacter( const FString& Name )
{
    if ( TcpClient->IsConnected() == false )
    {
        if ( HU_CHECK_TRUE( TcpClient->ConnectToServer( ServerIP, Port ) == false ) )
        {
            return false;
        }
    }

    hu::LoginReq Req;
    {
        Req.user_id = TCHAR_TO_UTF8( *Name );
    }
    if ( HU_CHECK_TRUE( TcpClient->SendObj( Req ) == false ) )
    {
        return false;
    }

    HU_LOG_SUCCESS();
    return true;
}

void AHuGameLogic::Tick( float DeltaTime )
{
    Super::Tick( DeltaTime );
}

void AHuGameLogic::BeginPlay()
{
    Super::BeginPlay();

    TcpClient = GetWorld()->SpawnActor<AHuTcpClient>();
    if ( TcpClient )
    {
        TcpClient->OnConnectionStatusChanged.BindDynamic( this, &AHuGameLogic::OnConnectionStatusChanged );
        TcpClient->OnMessageReceived.BindDynamic( this, &AHuGameLogic::OnMessageReceived );
    }
}

void AHuGameLogic::OnConnectionStatusChanged( const EHuConnectionState ConnectionState )
{
    UEnum* EnumPtr = StaticEnum<EHuConnectionState>();
    FString ConnectionStateString = EnumPtr->GetNameStringByValue( (int64)ConnectionState );
    UE_LOG( HuLogCommon, Log, TEXT( "Connection state changed : %s" ), *ConnectionStateString );
}

bool AHuGameLogic::OnMessageReceived( const FHuMessage& Message )
{
    if ( Message.Header.MessageId == hu::LoginRes::kTypeId )
    {
        hu::LoginRes Res;
        if ( HU_CHECK_TRUE( Message.Read( Res ) == false ) )
        {
            return false;
        }

        FHuLoginCharacterResult Result;

        if ( Res.code == hu::kResSuccess )
        {
            Result.Name = UTF8_TO_TCHAR( Res.user.id.c_str() );
        }
        else
        {
            Result.Error = FString::Printf( TEXT( "ErrorCode = %u" ), Res.code );
        }

        OnLoginCharacter.ExecuteIfBound( Result );
        return true;
    }

    return false;
}
