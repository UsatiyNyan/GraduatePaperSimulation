// Illarionov Kirill's Graduation Project for BMSTU

#pragma once

#include "CoreMinimal.h"

#include <vector>

#include "GameFramework/Pawn.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "Simulation/Components/CablePiece.h"
#include "ROVPawn.generated.h"

UCLASS(config=Game)
class SIMULATION_API AROVPawn : public APawn
{
	GENERATED_BODY()

public:
	AROVPawn();

protected:
	virtual void BeginPlay() override;
	
public:	
	virtual void Tick(float DeltaTime) override;

private:
	void ApplyForcesToCables();

protected:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void MoveForward(float Amount);

	void RollRotation(float Amount);
	void PitchRotation(float Amount);
	void YawRotation(float Amount);

	void Scroll(float Amount);

	void Stop();

public:
	UPROPERTY(Category=ControlSystem, EditAnywhere, BlueprintReadWrite)
	float MinLooseness{1.3f};

	UPROPERTY(Category=ControlSystem, EditAnywhere, BlueprintReadWrite)
	float MaxLooseness{1.6f};

	UPROPERTY(Category=ROV, EditAnywhere, BlueprintReadWrite)
	float DragCoefficient{0.22f};
	
	UPROPERTY(Category=Cable, EditAnywhere, BlueprintReadWrite)
	FComponentReference AttachEndTo{};
	
	UPROPERTY(Category=Cable, EditAnywhere, BlueprintReadWrite)
	FName AttachEndToSocketName{};

	UPROPERTY(Category=Cable, EditAnywhere, BlueprintReadWrite, meta=(ToolTip="kg/m^3"))
	float CableDensity{1025.f};
	
	UPROPERTY(Category=Cable, EditAnywhere, BlueprintReadWrite)
	float CableNormalCoefficient{1.2f};

	UPROPERTY(Category=Cable, EditAnywhere, BlueprintReadWrite)
	float CableTangentCoefficient{0.2f};

	UPROPERTY(Category=WaterEnvironment, EditAnywhere, BlueprintReadWrite)
	FVector FlowVelocity{GetActorForwardVector() * -1};

	UPROPERTY(Category=WaterEnvironment, EditAnywhere, BlueprintReadWrite, meta=(ToolTip="kg/m^3"))
	float WaterDensity{1025.f};
	
private:
	FVector CableWeightAndWaterDisplacementForce; // precalculated in BeginPlay
	float OneCableTangentResistancePrecalculated; // precalculated in BeginPlay
	float OneCableNormalResistancePrecalculated; // precalculated in BeginPlay
	float CableOneLengthSquared; // precalculated in BeginPlay
	float CableDiameter;
	float CableOneLength;
	
	std::vector<std::pair<UCablePiece*, UPhysicsConstraintComponent*>> CablePiecesAndConstraints;
	UPhysicsConstraintComponent* FinalPhysicsConstraintComponent;

public:
	virtual UPawnMovementComponent* GetMovementComponent() const override;

protected:
	UPROPERTY(Category=Pawn, VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	class UFloatingPawnMovement* MovementComponent;

	UPROPERTY(Category=Camera, VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	class USpringArmComponent* CameraBoomComponent;

	UPROPERTY(Category=Camera, VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	class UCameraComponent* FollowCameraComponent;

	UPROPERTY(Category=Mesh, VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	UStaticMeshComponent* MeshComponent;
	
private:
	UPrimitiveComponent* GetEndComponent();
	FVector GetEndPosition();
};
