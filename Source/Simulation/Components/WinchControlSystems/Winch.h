#pragma once

class FWinch
{
public:
	FWinch() = default;

	void Tick(const float DeltaTime, const float ControlVoltage, const float CounteractingTorque);
	float GetAngularSpeed() const;

private:
	float AngularSpeed = 0;
	
	float PrevVoltage = 0;
	float PrevTorque = 0;
};
