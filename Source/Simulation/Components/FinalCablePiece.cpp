// Illarionov Kirill's Graduation Project for BMSTU


#include "FinalCablePiece.h"


UFinalCablePiece::UFinalCablePiece() : Super{}
{
	auto CableStaticMesh = ConstructorHelpers::FObjectFinder<UStaticMesh>(
		TEXT("StaticMesh'/Game/Simulation/Cable1.Cable1'")).Object;
	UStaticMeshComponent::SetStaticMesh(CableStaticMesh);
	InitialBoxSize = CableStaticMesh->GetBoundingBox().GetSize();
}

void UFinalCablePiece::SetLength(float Length)
{
	FVector CurrentSize = InitialBoxSize;
	CurrentSize.X = Length;
	SetWorldScale3D(CurrentSize / InitialBoxSize);
}
