#pragma once

#include "Winch.h"

#include <string>

class IWinchControlSystem
{
public:
	IWinchControlSystem(float InitialLength)
		: CurrentLength{InitialLength}
	{
	}

	virtual ~IWinchControlSystem() = default;

	virtual void Tick(const float DeltaTime, const float DesiredLength, const float CounteractingForce) = 0;

	float GetCurrentLength() const
	{
		return CurrentLength;
	}

	virtual std::string GetLogName() const = 0;

protected:
	float CurrentLength;
	FWinch Winch{};

	static constexpr float DrumRadius = 0.4f;
};
