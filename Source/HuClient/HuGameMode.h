#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "HuCommon/HuGameLogic.h"
#include "HuGameMode.generated.h"

UCLASS()
class HUCLIENT_API AHuGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AHuGameMode();

public:
    UFUNCTION( BlueprintCallable, Category = "HuGame" )
    void LoginCharacter( const FString& Name );

public:
    virtual APawn* SpawnDefaultPawnAtTransform_Implementation( AController* NewPlayer, const FTransform& SpawnTransform ) override;
    virtual APawn* SpawnDefaultPawnFor_Implementation( AController* NewPlayer, AActor* StartSpot ) override;

protected:
    virtual void BeginPlay() override;

private:
    UFUNCTION()
    void OnLoginCharacter( const FHuLoginCharacterResult& Result );

private:
    void ShowLoginUI();
    void HideLoginUI();
    class AHuCharacter* SpawnCharacter( const FString& Name, const FVector& Location, const FRotator& Rotation );

public:
    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "HuGame" )
    TSubclassOf<class UUserWidget> LoginWidgetClass;

    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "HuGame" )
    TSubclassOf<class AHuCharacter> CharacterClass;

private:
    UPROPERTY()
    class UUserWidget* LoginWidget { nullptr };

    UPROPERTY()
    class AHuPlayerController* PlayerController { nullptr };

    UPROPERTY()
    AHuGameLogic* GameLogic { nullptr };
};
