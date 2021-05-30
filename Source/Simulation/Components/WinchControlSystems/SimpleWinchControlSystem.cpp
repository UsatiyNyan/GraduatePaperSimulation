#include "SimpleWinchControlSystem.h"

#include <limits>

float FSimpleWinchControlSystem::CalculateWinchVelocity(const FVector& ROVPosition, const FVector& ROVVelocity)
{
	const float Distance = ROVPosition.Size();
	const float CurrentLooseness = CurrentLength / Distance;
	const float DesiredLength = FMath::Clamp(CurrentLooseness, MinLooseness, MaxLooseness) * Distance;
	const float DeltaLength = DesiredLength - CurrentLength;

	if (FMath::Abs(DeltaLength) < std::numeric_limits<float>::epsilon())
	{
		return 0;
	}

	const float ROVFloatingAwayVelocity = FVector::DotProduct(ROVVelocity, ROVPosition) / Distance;
	
	if (FMath::Sign(DeltaLength) == FMath::Sign(ROVFloatingAwayVelocity))
	{
		return FMath::Sign(DeltaLength) * MaxVelocity;
	}

	return FMath::Clamp(FMath::Abs(ROVFloatingAwayVelocity), MinVelocity, MaxVelocity) * FMath::Sign(DeltaLength);
}
