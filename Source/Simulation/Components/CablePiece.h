// Illarionov Kirill's Graduation Project for BMSTU

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "CablePiece.generated.h"


UCLASS(Config=Game)
class SIMULATION_API UCablePiece : public UStaticMeshComponent
{
	GENERATED_BODY()

public:
	static constexpr auto EndSocketName = "EndSocket";

	UCablePiece();
	void SetupThis(uint32_t CableId, float Density, float WaterDensity, UPhysicsConstraintComponent* CreatedPhysicsConstraintComponent,
	               UPrimitiveComponent* AttachableComponent, const FName AttachableComponentSocketName);
	virtual ~UCablePiece() override;

	FVector GetWaterDisplacementForce() const;

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

private:
	uint32_t Id;
	FVector WaterDisplacementForce;
	UPhysicsConstraintComponent* WeakPhysicsConstraintComponent;
	UPrimitiveComponent* WeakAttachableComponent;
};
