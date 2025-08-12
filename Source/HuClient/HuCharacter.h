#pragma once

#include "CoreMinimal.h"
#include "HuBaseCharacter.h"
#include "HuCharacter.generated.h"

UCLASS()
class HUCLIENT_API AHuCharacter : public AHuBaseCharacter
{
    GENERATED_BODY()

public:
    AHuCharacter();

public:
    void SetPlayerController( class AHuPlayerController* InPlayerController );

public:
    virtual void SetupPlayerInputComponent( class UInputComponent* PlayerInputComponent ) override;

private:
    void RightMouseClick();
    bool GetMouseLocation( FVector& Location ) const;

public:
    UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "HuGame" )
    class USpringArmComponent* SpringArm { nullptr };

    UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "HuGame" )
    class UCameraComponent* Camera { nullptr };

private:
    UPROPERTY()
    class AHuPlayerController* PlayerController { nullptr };
};
