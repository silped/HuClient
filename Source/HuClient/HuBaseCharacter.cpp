#include "HuBaseCharacter.h"

#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/TextBlock.h"

AHuBaseCharacter::AHuBaseCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    static const FName CapsuleName { TEXT( "Capsule" ) };
    Capsule = CreateDefaultSubobject<UCapsuleComponent>( CapsuleName );
    RootComponent = Capsule;

    static const FName MeshName { TEXT( "Mesh" ) };
    Mesh = CreateDefaultSubobject<UStaticMeshComponent>( MeshName );
    Mesh->SetupAttachment( RootComponent );

    static const FName WidgetName { TEXT( "Widget" ) };
    Widget = CreateDefaultSubobject<UWidgetComponent>( WidgetName );
    Widget->SetupAttachment( RootComponent );
}

void AHuBaseCharacter::SetName( const FString& InName )
{
    Name = InName;

    if ( NameWidget )
    {
        NameWidget->SetText( FText::FromString( Name ) );
    }
}

void AHuBaseCharacter::MoveToLocation( const FVector& Location )
{
    if ( Location.X != TargetLocation.X || Location.Y != TargetLocation.Y )
    {
        TargetLocation = Location;
        TargetLocation.Z = GetActorLocation().Z;

        bIsMoving = true;

        if ( LocationWidget )
        {
            static const FString MovingStr { TEXT( "Moving.." ) };
            LocationWidget->SetText( FText::FromString( MovingStr ) );
        }
    }
}

void AHuBaseCharacter::Tick( float DeltaTime )
{
    Super::Tick( DeltaTime );

    if ( bIsMoving )
    {
        const FVector CurLoc = GetActorLocation();
        const FVector Dir = ( TargetLocation - CurLoc ).GetSafeNormal();

        const float Dist = FVector::Dist( CurLoc, TargetLocation );
        if ( Dist < 10.0f )
        {
            bIsMoving = false;
            SetActorLocation( TargetLocation );

            if ( LocationWidget )
            {
                LocationWidget->SetText( TargetLocation.ToText() );
            }
        }
        else
        {
            const FVector NewLoc = CurLoc + ( Dir * MoveSpeed * DeltaTime );
            SetActorLocation( NewLoc );
        }
    }
}

void AHuBaseCharacter::BeginPlay()
{
    Super::BeginPlay();

    UUserWidget* const UserWidget = Widget->GetUserWidgetObject();
    if ( UserWidget )
    {
        static const FName NameWidgetName { TEXT( "Name" ) };
        NameWidget = Cast<UTextBlock>( UserWidget->GetWidgetFromName( NameWidgetName ) );
        if ( NameWidget )
        {
            if ( Name.IsEmpty() == false )
            {
                NameWidget->SetText( FText::FromString( Name ) );
            }
        }

        static const FName LocationWidgetName { TEXT( "Location" ) };
        LocationWidget = Cast<UTextBlock>( UserWidget->GetWidgetFromName( LocationWidgetName ) );
        if ( LocationWidget )
        {
            LocationWidget->SetText( GetActorLocation().ToText() );
        }
    }
}
