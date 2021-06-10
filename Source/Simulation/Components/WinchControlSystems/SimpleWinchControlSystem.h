#pragma once

#include "IWinchControlSystem.h"

class FSimpleWinchControlSystem : public IWinchControlSystem 
{
public:
	using IWinchControlSystem::IWinchControlSystem;
	
	virtual void Tick(const float DeltaTime, const float DesiredLength, const float CounteractingForce) override;

	virtual std::string GetLogName() const override;
};
