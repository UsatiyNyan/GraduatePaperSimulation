// Illarionov Kirill's Graduation Project for BMSTU

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "CablePiece.generated.h"


UCLASS(config=Game)
class SIMULATION_API UCablePiece : public UStaticMeshComponent
{
	GENERATED_BODY()

public:
	static constexpr auto EndSocketName = "EndSocket";

	UCablePiece();
	void SetupThis(uint32_t CableId, float Density, UPhysicsConstraintComponent* CreatedPhysicsConstraintComponent,
	               UPrimitiveComponent* AttachableComponent, const FName AttachableComponentSocketName);
	virtual ~UCablePiece() override;

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

private:
	uint32_t Id;
	UPhysicsConstraintComponent* WeakPhysicsConstraintComponent;
	UPrimitiveComponent* WeakAttachableComponent;
};
