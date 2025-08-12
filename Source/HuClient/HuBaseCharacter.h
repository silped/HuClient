#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "HuBaseCharacter.generated.h"

UCLASS()
class HUCLIENT_API AHuBaseCharacter : public APawn
{
    GENERATED_BODY()

public:
    AHuBaseCharacter();

public:
    void SetName( const FString& InName );
    void MoveToLocation( const FVector& Location );

public:
    virtual void Tick( float DeltaTime ) override;

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY( VisibleAnywhere, BlueprintReadWrite, Category = "HuGame" )
    float MoveSpeed { 600.0f };

    UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "HuGame" )
    class UCapsuleComponent* Capsule { nullptr };

    UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "HuGame" )
    UStaticMeshComponent* Mesh { nullptr };

    UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "HuGame" )
    class UWidgetComponent* Widget { nullptr };

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "HuGame" )
    FString Name;

protected:
    UPROPERTY()
    class UTextBlock* NameWidget { nullptr };

    UPROPERTY()
    class UTextBlock* LocationWidget { nullptr };

protected:
    FVector TargetLocation { FVector::ZeroVector };
    bool    bIsMoving      { false };
};
