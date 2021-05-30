// Illarionov Kirill's Graduation Project for BMSTU

#pragma once

#include "CoreMinimal.h"
#include "CablePiece.h"
#include "FinalCablePiece.generated.h"


UCLASS(Config=Game)
class SIMULATION_API UFinalCablePiece : public UCablePiece 
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UFinalCablePiece();

	void SetLength(float Length);

private:
	FVector InitialBoxSize;
};
