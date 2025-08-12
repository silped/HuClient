#include "HuGameMode.h"

#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"

#include "HuCommon/HuGameLogic.h"
#include "HuCharacter.h"
#include "HuPlayerController.h"

AHuGameMode::AHuGameMode()
{
    DefaultPawnClass = AHuCharacter::StaticClass();
    PlayerControllerClass = AHuPlayerController::StaticClass();
}

APawn* AHuGameMode::SpawnDefaultPawnAtTransform_Implementation( AController* NewPlayer, const FTransform& SpawnTransform )
{
    return nullptr;
}

APawn* AHuGameMode::SpawnDefaultPawnFor_Implementation( AController* NewPlayer, AActor* StartSpot )
{
    return nullptr;
}

void AHuGameMode::LoginCharacter( const FString& Name )
{
    if ( HU_CHECK_TRUE( GameLogic->LoginCharacter( Name ) == false ) )
    {
        return;
    }
}

void AHuGameMode::BeginPlay()
{
    Super::BeginPlay();

    PlayerController = Cast<AHuPlayerController>( UGameplayStatics::GetPlayerController( this, 0 ) );

    GameLogic = GetWorld()->SpawnActor<AHuGameLogic>();
    if ( GameLogic )
    {
        GameLogic->OnLoginCharacter.BindDynamic( this, &AHuGameMode::OnLoginCharacter );
    }

    ShowLoginUI();
}

void AHuGameMode::OnLoginCharacter( const FHuLoginCharacterResult& Result )
{
    if ( HU_CHECK_TRUE( Result.Error.IsEmpty() == false ) )
    {
        return;
    }

    const AActor* const PlayerStart = UGameplayStatics::GetActorOfClass( GetWorld(), APlayerStart::StaticClass() );
    if ( HU_CHECK_TRUE( PlayerStart == nullptr ) )
    {
        return;
    }

    AHuCharacter* const Character = SpawnCharacter( Result.Name, PlayerStart->GetActorLocation(), PlayerStart->GetActorRotation() );
    if ( HU_CHECK_TRUE( Character == nullptr ) )
    {
        return;
    }

    Character->SetPlayerController( PlayerController );

    PlayerController->Possess( Character );
    PlayerController->DefaultMouseCursor = EMouseCursor::Crosshairs;
    PlayerController->CurrentMouseCursor = EMouseCursor::Crosshairs;

    HideLoginUI();
}

void AHuGameMode::ShowLoginUI()
{
    if ( ( LoginWidget == nullptr ) && LoginWidgetClass )
    {
        LoginWidget = CreateWidget<UUserWidget>( PlayerController, LoginWidgetClass );
        if ( LoginWidget )
        {
            LoginWidget->AddToViewport();
        }
    }
}

void AHuGameMode::HideLoginUI()
{
    if ( LoginWidget )
    {
        LoginWidget->RemoveFromParent();
        LoginWidget = nullptr;
    }
}

AHuCharacter* AHuGameMode::SpawnCharacter( const FString& Name, const FVector& Location, const FRotator& Rotation )
{
    if ( CharacterClass && ( Name.IsEmpty() == false ) )
    {
        UWorld* const World = GetWorld();
        if ( World )
        {
            FActorSpawnParameters SpawnParams;
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

            AHuCharacter* const Character = World->SpawnActor<AHuCharacter>( CharacterClass, Location, Rotation, SpawnParams );
            if ( Character )
            {
                Character->SetName( Name );
                return Character;
            }
        }
    }

    return nullptr;
}
