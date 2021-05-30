#pragma once

#include "IWinchControlSystem.h"

class FSimpleWinchControlSystem : public IWinchControlSystem 
{
public:
	using IWinchControlSystem::IWinchControlSystem;

protected:
	virtual float CalculateWinchVelocity(const FVector& ROVPosition, const FVector& ROVVelocity) override;
};
