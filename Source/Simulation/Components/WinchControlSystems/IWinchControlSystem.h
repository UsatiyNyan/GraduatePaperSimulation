#pragma once

class IWinchControlSystem
{
public:
	IWinchControlSystem(float InitialLength, float MinVelocity, float MaxVelocity, float MinLooseness, float MaxLooseness)
		: CurrentLength{InitialLength},
		  MinVelocity{MinVelocity},
		  MaxVelocity{MaxVelocity},
		  MinLooseness{MinLooseness},
		  MaxLooseness{MaxLooseness}
	{
	}

	virtual ~IWinchControlSystem() = default;

	void Tick(const float DeltaTime, const FVector& ROVPosition, const FVector& ROVVelocity)
	{
		CurrentLength += DeltaTime * CalculateWinchVelocity(ROVPosition, ROVVelocity);
	}

	float GetCurrentLength() const
	{
		return CurrentLength;
	}

protected:
	virtual float CalculateWinchVelocity(const FVector& ROVPosition, const FVector& ROVVelocity) = 0;

protected:
	float CurrentLength;
	float MinVelocity;
	float MaxVelocity;
	float MinLooseness;
	float MaxLooseness;
};
