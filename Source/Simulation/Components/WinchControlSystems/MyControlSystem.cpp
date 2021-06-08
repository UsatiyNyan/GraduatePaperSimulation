#include "MyControlSystem.h"

namespace 
{
	constexpr float Kp = 7.8;
	constexpr float Ki = 3.6;
	constexpr float Kd = 3.05;
}

void FMyControlSystem::Tick(const float DeltaTime, const float DesiredLength, const float CounteractingForce)
{
	const float Error = DesiredLength - CurrentLength;
	
	AccumulatedError += (PreviousError + Error) * DeltaTime / 2;
	
	const float ControlVoltage =
		Kp * Error +
		Ki * AccumulatedError +
		Kd * (Error - PreviousError) / DeltaTime;
	
	PreviousError = Error;
	
	Winch.Tick(DeltaTime, ControlVoltage, CounteractingForce * DrumRadius);
	CurrentLength += Winch.GetAngularSpeed() * DrumRadius;
}
