#include "HuCharacter.h"

#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "HuPlayerController.h"

AHuCharacter::AHuCharacter()
{
    static const FName SpringArmName { TEXT( "SpringArm" ) };
    SpringArm = CreateDefaultSubobject<USpringArmComponent>( SpringArmName );
    SpringArm->SetupAttachment( RootComponent );

    static const FName CameraName { TEXT( "Camera" ) };
    Camera = CreateDefaultSubobject<UCameraComponent>( CameraName );
    Camera->SetupAttachment( SpringArm );
}

void AHuCharacter::SetPlayerController( AHuPlayerController* InPlayerController )
{
    PlayerController = InPlayerController;
}

void AHuCharacter::SetupPlayerInputComponent( UInputComponent* PlayerInputComponent )
{
    Super::SetupPlayerInputComponent( PlayerInputComponent );

    static const FName RightMouseClickName = TEXT( "RightMouseClick" );
    PlayerInputComponent->BindAction( RightMouseClickName, IE_Pressed, this, &AHuCharacter::RightMouseClick );
}

void AHuCharacter::RightMouseClick()
{
    FVector Loc;
    if ( GetMouseLocation( Loc ) )
    {
        MoveToLocation( Loc );
    }
}

bool AHuCharacter::GetMouseLocation( FVector& Location ) const
{
    if ( PlayerController )
    {
        float MouseX = 0;
        float MouseY = 0;
        PlayerController->GetMousePosition( MouseX, MouseY );

        FVector Dir;
        if ( PlayerController->DeprojectScreenPositionToWorld( MouseX, MouseY, Location, Dir ) )
        {
            FHitResult HitResult;
            FCollisionQueryParams CollisionParams;
            CollisionParams.AddIgnoredActor( this );

            const UWorld* const World = GetWorld();
            if ( World )
            {
                const FVector End = Location + ( Dir * 10000.0f );

                if ( World->LineTraceSingleByChannel( HitResult, Location, End, ECC_Visibility, CollisionParams ) )
                {
                    Location = HitResult.Location;
                    return true;
                }
                else
                {
                    if ( Dir.Z != 0 )
                    {
                        const float T = -Location.Z / Dir.Z;
                        Location += ( Dir * T );
                        return true;
                    }
                }
            }
        }
    }

    return false;
}
