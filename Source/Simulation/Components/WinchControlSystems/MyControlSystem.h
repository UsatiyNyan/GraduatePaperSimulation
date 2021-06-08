#pragma once

#include "IWinchControlSystem.h"

class FMyControlSystem : public IWinchControlSystem
{
public:
	using IWinchControlSystem::IWinchControlSystem;

	virtual void Tick(const float DeltaTime, const float DesiredLength, const float CounteractingForce) override;

private:
	float AccumulatedError = 0;
	float PreviousError = 0;
};
