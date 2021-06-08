#include "Winch.h"

namespace
{
	constexpr float L = 0.0013;
	constexpr float R = 5;
	constexpr float Ke = 0.1 * PI / 30;
	constexpr float Km = 2.4;
	constexpr float Jp = 0.0039;
	constexpr float GearRatio = 30;
	constexpr float GearEfficiency = 1 / (30 * 0.8);

	constexpr float K = Km / R;
	constexpr float T = L / R;
}


void FWinch::Tick(const float DeltaTime, const float ControlVoltage, const float CounteractingTorque)
{
	const float CounterEmf = Ke * AngularSpeed;
	const float EngineVoltage = FMath::Clamp(ControlVoltage, -220.f, 220.f) - CounterEmf;
	const float Fd = 1 / DeltaTime;

	const float Torque = ((EngineVoltage + PrevVoltage) * K - PrevTorque * (1 - 2 * T * Fd)) / (1 + 2 * T * Fd);

	PrevVoltage = EngineVoltage;
	PrevTorque = Torque;

	const float TotalTorque = Torque - CounteractingTorque * GearEfficiency;
	const float AngularAcceleration = TotalTorque / Jp;

	AngularSpeed += AngularAcceleration * DeltaTime;
}

float FWinch::GetAngularSpeed() const
{
	return AngularSpeed / GearRatio;
}
