

#include "MPShooterPlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"

AMPShooterPlayerController::AMPShooterPlayerController()
{
    static ConstructorHelpers::FObjectFinder<UInputMappingContext> MappingContextAsset(TEXT("/Script/EnhancedInput.InputMappingContext'/Game/Input/DefaultControls.DefaultControls'"));
    InputMapping=MappingContextAsset.Object;

}

void AMPShooterPlayerController::BeginPlay()
{
    ULocalPlayer* LocalPlayer = GetLocalPlayer();

    // El siguiente condicional es importante. Si se quita crashea para configuraciones con servidor dedicado.
    // Cada jugador en un cliente o listen server tiene un LocalPlayer, de donde
    // tomar cosas como el subsistema de entrada. En el servidor dedicado sin embargo 
    // no hay un LocalPlayer, y este c'odigo se ejecuta tanto en los clientes como
    // en el servidor
    if(LocalPlayer)
    {
        InputSystem=LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
        if(InputSystem)
        {
            if(InputMapping)
            {
                InputSystem->AddMappingContext(InputMapping,0);
            }
        }
    }

}