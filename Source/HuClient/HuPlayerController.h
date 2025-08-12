#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "HuPlayerController.generated.h"

UCLASS()
class HUCLIENT_API AHuPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    AHuPlayerController();
};
