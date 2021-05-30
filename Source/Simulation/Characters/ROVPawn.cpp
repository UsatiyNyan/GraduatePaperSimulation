// Illarionov Kirill's Graduation Project for BMSTU

#include "ROVPawn.h"

#include "DrawDebugHelpers.h"
#include "Editor.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/CollisionProfile.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/SpringArmComponent.h"
#include "Math/UnrealMathUtility.h"
#include "Simulation/Components/FinalCablePiece.h"
#include "Simulation/Components/WinchControlSystems/SimpleWinchControlSystem.h"

constexpr auto ROVMeshCableSocketName = "CableSocket";

AROVPawn::AROVPawn() : Super{}
{
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	AutoPossessPlayer = EAutoReceiveInput::Player0;

	// MeshComponent
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("MeshComponent");
	MeshComponent->SetStaticMesh(
		ConstructorHelpers::FObjectFinder<UStaticMesh>(TEXT("StaticMesh'/Game/Simulation/ROV.ROV'")).Object);
	MeshComponent->SetCollisionProfileName(UCollisionProfile::Vehicle_ProfileName);
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetSimulatePhysics(true);
	MeshComponent->SetMassOverrideInKg(NAME_None, 200);
	MeshComponent->SetEnableGravity(false);
	MeshComponent->SetLinearDamping(DragCoefficient);
	MeshComponent->SetAngularDamping(DragCoefficient);
	MeshComponent->SetShouldUpdatePhysicsVolume(true);
	MeshComponent->SetCanEverAffectNavigation(false);
	MeshComponent->SetMobility(EComponentMobility::Movable);
	ensureMsgf(SetRootComponent(MeshComponent), TEXT("no meshes("));

	// FloatingPawnMovement
	MovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>("FloatingPawnMovement");
	MovementComponent->SetUpdatedComponent(MeshComponent);

	// CameraBoomComponent
	CameraBoomComponent = CreateDefaultSubobject<USpringArmComponent>("CameraBoomComponent");
	CameraBoomComponent->SetupAttachment(RootComponent);
	CameraBoomComponent->TargetArmLength = 300.f; // The camera follows at this distance behind the character
	CameraBoomComponent->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// FollowCameraComponent
	FollowCameraComponent = CreateDefaultSubobject<UCameraComponent>("FollowCameraComponent");
	// Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCameraComponent->SetupAttachment(CameraBoomComponent, USpringArmComponent::SocketName);
	FollowCameraComponent->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// CableMeshMeasurements
	const auto* CableMesh = ConstructorHelpers::FObjectFinder<UStaticMesh>(
		TEXT("StaticMesh'/Game/Simulation/Cable.Cable'")).Object;
	const FVector CableMeshMeasures = CableMesh->GetBoundingBox().GetSize();
	CableOneLength = CableMeshMeasures.GetAbsMax();
	CableDiameter = CableMeshMeasures.GetAbsMin();
}

void AROVPawn::BeginPlay()
{
	Super::BeginPlay();

	// precalculated values
	OneCableTangentResistancePrecalculated = CableTangentCoefficient * PI * CableDiameter * CableOneLength *
		WaterDensity / 2;

	OneCableNormalResistancePrecalculated = CableNormalCoefficient * CableDiameter * CableOneLength *
		WaterDensity / 2;

	// initial cable length
	const FVector Delta = GetEndPosition() - MeshComponent->GetSocketLocation(ROVMeshCableSocketName);
	const FRotator DeltaRotation = Delta.Rotation();
	const uint32_t InitialCableAmount = Delta.Size() / CableOneLength;

	for (uint32_t i = 0; i < InitialCableAmount; ++i)
	{
		CreateCablePiece(DeltaRotation);
	}

	// FinalPhysicsConstraintComponent
	AddFinalPieceAndConstraint(Delta.Size() - InitialCableAmount * CableOneLength, DeltaRotation);

	// WinchControlSystem
	WinchControlSystem = std::make_unique<FSimpleWinchControlSystem>(Delta.Size(), MinWinchVelocity, MaxWinchVelocity,
	                                                                 MinLooseness, MaxLooseness);
}

void AROVPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TickCable(DeltaTime);
	ApplyForcesToCables();

	DrawDebugSphere(GetWorld(), GetEndComponent()->GetComponentLocation(), CableOneLength, 12,
	                FColor::Red, false, -1, 0, 1);
	GEngine->AddOnScreenDebugMessage(INDEX_NONE, 0.f, FColor::Emerald,
	                                 "Velocity: " + MovementComponent->Velocity.ToString());
}

void AROVPawn::TickCable(const float DeltaTime)
{
	const FVector EndPosition = GetEndPosition();
	const FVector ROVPosition = MeshComponent->GetSocketLocation(ROVMeshCableSocketName) - EndPosition;
	GEngine->AddOnScreenDebugMessage(INDEX_NONE, 0.f, FColor::Emerald,
	                                 "ROVPosition: " + ROVPosition.ToString());

	WinchControlSystem->Tick(DeltaTime, ROVPosition, MovementComponent->Velocity);

	const float WholeCableDiscreteLength = CableOneLength * CablePiecesAndConstraints.Num();
	const float WholeCableLength = WinchControlSystem->GetCurrentLength();
	const float OneCableDeltaLength = WholeCableLength - WholeCableDiscreteLength;
	float OneCableDeltaLengthAbs = FMath::Abs(OneCableDeltaLength);

	if (OneCableDeltaLengthAbs < std::numeric_limits<float>::epsilon()) // OneCableDeltaLength == 0
	{
		return;
	}

	if (OneCableDeltaLengthAbs >= CableOneLength)
	{
		if (OneCableDeltaLength > 0)
		{
			// add one cable piece
			const auto LastAttachableComponentAndSocket = GetLastAttachableComponentAndSocket();
			FVector Delta = EndPosition -
				LastAttachableComponentAndSocket.Key->GetSocketLocation(LastAttachableComponentAndSocket.Value);
			CreateCablePiece(Delta.Rotation());
		}
		else // OneCableDeltaLength < 0
		{
			// remove one cable piece
			auto LastCablePieceAndConstraint = CablePiecesAndConstraints.Pop();
			LastCablePieceAndConstraint.Key->UnregisterComponent();
			LastCablePieceAndConstraint.Value->UnregisterComponent();
		}
	}

	const auto LastAttachableComponentAndSocket = GetLastAttachableComponentAndSocket();
	FVector Delta = EndPosition -
		LastAttachableComponentAndSocket.Key->GetSocketLocation(LastAttachableComponentAndSocket.Value);
	AddFinalPieceAndConstraint(Delta.Size(), Delta.Rotation());
}

void AROVPawn::ApplyForcesToCables()
{
	for (auto& CablePieceAndPhysicsConstraint : CablePiecesAndConstraints)
	{
		auto* CablePiece = CablePieceAndPhysicsConstraint.Key;
		FVector Rotation = CablePiece->GetSocketLocation(UCablePiece::EndSocketName) -
			CablePiece->GetComponentLocation();

		FVector TotalVelocity = (FlowVelocity - MovementComponent->Velocity) / 100;
		FVector TangentVelocity = TotalVelocity.ProjectOnTo(Rotation);
		FVector NormalVelocity = TotalVelocity - TangentVelocity;

		DrawDebugDirectionalArrow(GetWorld(), CablePiece->GetComponentLocation(),
		                          CablePiece->GetComponentLocation() + TotalVelocity.GetSafeNormal() * CableOneLength,
		                          15, FColor::Blue, false, -1, 0, 6);

		DrawDebugDirectionalArrow(GetWorld(), CablePiece->GetComponentLocation(),
		                          CablePiece->GetComponentLocation() + TangentVelocity.GetSafeNormal() * CableOneLength,
		                          15, FColor::Cyan, false, -1, 0, 6);

		DrawDebugDirectionalArrow(GetWorld(), CablePiece->GetComponentLocation(),
		                          CablePiece->GetComponentLocation() + NormalVelocity.GetSafeNormal() * CableOneLength,
		                          15, FColor::Magenta, false, -1, 0, 6);

		FVector TangentForce = TangentVelocity.SizeSquared() * TangentVelocity.GetSafeNormal();
		FVector NormalForce = NormalVelocity.SizeSquared() * NormalVelocity.GetSafeNormal();
		CablePiece->AddForce(CablePiece->GetWaterDisplacementForce() + TangentForce + NormalForce);
	}
}

void AROVPawn::CreateCablePiece(FRotator Rotation)
{
	uint32_t Id = CablePiecesAndConstraints.Num();
	const auto LastAttachableComponentAndSocket = GetLastAttachableComponentAndSocket();

	auto NewCablePiece = NewObject<UCablePiece>(this, ToCStr("CablePiece" + FString::FromInt(Id)));
	auto NewPhysicsConstraintComponent = NewObject<UPhysicsConstraintComponent>(this,
		ToCStr("PhysicsConstraint" + FString::FromInt(Id)));
	NewCablePiece->SetupThis(Id, CableDensity, WaterDensity, NewPhysicsConstraintComponent,
	                         LastAttachableComponentAndSocket.Key, LastAttachableComponentAndSocket.Value);
	NewCablePiece->SetWorldRotation(Rotation);
	NewCablePiece->RegisterComponent();
	NewPhysicsConstraintComponent->RegisterComponent();

	CablePiecesAndConstraints.Emplace(NewCablePiece, NewPhysicsConstraintComponent);
}

void AROVPawn::AddFinalPieceAndConstraint(const float Radius, const FRotator& Rotation)
{
	const auto LastAttachableComponentAndSocket = GetLastAttachableComponentAndSocket();

	if (!FinalCablePiece)
	{
		FinalCablePiece = NewObject<UFinalCablePiece>(this, "FinalCablePiece");
	}

	if (!FinalConstraint)
	{
		FinalConstraint = NewObject<UPhysicsConstraintComponent>(this, "FinalConstraint");
	}

	if (!CableToEndpointConstraint)
	{
		CableToEndpointConstraint = NewObject<UPhysicsConstraintComponent>(this, "CableToEndpointConstraint");
	}

	FinalCablePiece->SetLength(Radius);
	FinalCablePiece->SetupThis(-1, CableDensity, WaterDensity, FinalConstraint,
	                           LastAttachableComponentAndSocket.Key, LastAttachableComponentAndSocket.Value);
	FinalCablePiece->SetWorldRotation(Rotation);

	if (!FinalCablePiece->IsRegistered())
	{
		FinalCablePiece->RegisterComponent();
	}

	if (!FinalConstraint->IsRegistered())
	{
		FinalConstraint->RegisterComponent();
	}
	
	if (!CableToEndpointConstraint->IsRegistered())
	{
		
		CableToEndpointConstraint->SetAngularTwistLimit(ACM_Locked, 0);
		CableToEndpointConstraint->SetAngularSwing1Limit(ACM_Limited, 90);
		CableToEndpointConstraint->SetAngularSwing2Limit(ACM_Limited, 90);
		CableToEndpointConstraint->SetLinearXLimit(LCM_Limited, 50.f);
		CableToEndpointConstraint->SetLinearYLimit(LCM_Limited, 50.f);
		CableToEndpointConstraint->SetLinearZLimit(LCM_Limited, 50.f);
		CableToEndpointConstraint->SetAngularDriveParams(1000, 1000, 10);
		CableToEndpointConstraint->SetLinearDriveParams(1000, 1000, 10);
		CableToEndpointConstraint->SetupAttachment(GetEndComponent(), AttachEndToSocketName);
		CableToEndpointConstraint->SetConstrainedComponents(GetEndComponent(), NAME_None, FinalCablePiece, NAME_None);
		CableToEndpointConstraint->RegisterComponent();
	}
}


void AROVPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AROVPawn::MoveForward);

	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

	PlayerInputComponent->BindAxis("RollRotation", this, &AROVPawn::RollRotation);
	PlayerInputComponent->BindAxis("PitchRotation", this, &AROVPawn::PitchRotation);
	PlayerInputComponent->BindAxis("YawRotation", this, &AROVPawn::YawRotation);

	PlayerInputComponent->BindAxis("Scroll", this, &AROVPawn::Scroll);

	PlayerInputComponent->BindAction("Stop", IE_Pressed, this, &AROVPawn::Stop);
}

void AROVPawn::MoveForward(float Amount)
{
	if (Amount == 0.f) { return; }

	MovementComponent->AddInputVector(GetActorForwardVector() * Amount, true);
}

void AROVPawn::RollRotation(float Amount)
{
	if (Amount == 0.f) { return; }

	AddActorLocalRotation(FRotator{0.f, 0.f, Amount});
}

void AROVPawn::PitchRotation(float Amount)
{
	if (Amount == 0.f) { return; }

	AddActorLocalRotation(FRotator{Amount, 0.f, 0.f});
}

void AROVPawn::YawRotation(float Amount)
{
	if (Amount == 0.f) { return; }

	AddActorLocalRotation(FRotator{0.f, Amount, 0.f});
}

void AROVPawn::Scroll(float Amount)
{
	if (Amount == 0.f) { return; }

	CameraBoomComponent->TargetArmLength += 10 * Amount;
}

void AROVPawn::Stop()
{
	MovementComponent->StopActiveMovement();
	MovementComponent->StopMovementImmediately();
}

UPawnMovementComponent* AROVPawn::GetMovementComponent() const
{
	return Cast<UPawnMovementComponent>(MovementComponent);
}

UPrimitiveComponent* AROVPawn::GetEndComponent()
{
	return Cast<UPrimitiveComponent>(AttachEndTo.GetComponent(GetOwner()));
}

FVector AROVPawn::GetEndPosition()
{
	auto EndComponent = GetEndComponent();
	return AttachEndToSocketName != NAME_None
		       ? EndComponent->GetSocketLocation(AttachEndToSocketName)
		       : EndComponent->GetComponentLocation();
}

TPair<UPrimitiveComponent*, FName> AROVPawn::GetLastAttachableComponentAndSocket()
{
	if (CablePiecesAndConstraints.Num() == 0)
	{
		return TPair<UPrimitiveComponent*, FName>{MeshComponent, ROVMeshCableSocketName};
	}

	return TPair<UPrimitiveComponent*, FName>{CablePiecesAndConstraints.Last().Key, UCablePiece::EndSocketName};
}
