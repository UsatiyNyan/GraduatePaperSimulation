#include "SimpleWinchControlSystem.h"

void FSimpleWinchControlSystem::Tick(const float DeltaTime, const float DesiredLength, const float CounteractingForce)
{
	const float Error = DesiredLength - CurrentLength;

	Winch.Tick(DeltaTime, Error, CounteractingForce * DrumRadius);
	CurrentLength += Winch.GetAngularSpeed() * DrumRadius;
}
