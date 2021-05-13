// Copyright Epic Games, Inc. All Rights Reserved.

#include "SimulationGameMode.h"
#include "SimulationHUD.h"
#include "SimulationCharacter.h"
#include "UObject/ConstructorHelpers.h"

ASimulationGameMode::ASimulationGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = ASimulationHUD::StaticClass();
}
