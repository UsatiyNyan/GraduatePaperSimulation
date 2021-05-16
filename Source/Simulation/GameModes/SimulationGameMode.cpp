// Copyright Epic Games, Inc. All Rights Reserved.

#include "SimulationGameMode.h"
#include "Simulation/Characters/ROVPawn.h"
#include "UObject/ConstructorHelpers.h"

ASimulationGameMode::ASimulationGameMode()
    : Super()
{
    // set default pawn class to our Blueprinted character
    // DefaultPawnClass = AROVPawn::StaticClass();
}
