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
	CableOneLengthSquared = FMath::Pow(CableOneLength, 2);

	OneCableTangentResistancePrecalculated = CableTangentCoefficient * PI * CableDiameter * CableOneLength *
		WaterDensity / 2;

	OneCableNormalResistancePrecalculated = CableNormalCoefficient * CableDiameter * CableOneLength *
		WaterDensity / 2;

	float OneCableVolume = CableOneLength * FMath::Pow(CableDiameter / 2, 2);
	constexpr float GravityConstant = 9.8f;
	CableWeightAndWaterDisplacementForce =
		FVector{0.f, 0.f, GravityConstant} *
		OneCableVolume *
		(WaterDensity - CableDensity);

	// initial cable length
	FVector Delta = GetEndPosition() - MeshComponent->GetSocketLocation(ROVMeshCableSocketName);
	uint32_t InitialCableAmount = Delta.Size() / CableOneLength;

	if (InitialCableAmount == 0) { return; }

	auto FirstCablePiece = NewObject<UCablePiece>(this, ToCStr("CablePiece" + FString::FromInt(0)));
	auto FirstPhysicsConstraintComponent = NewObject<UPhysicsConstraintComponent>(this,
		ToCStr("PhysicsConstraint" + FString::FromInt(0)));
	FirstCablePiece->SetupThis(0, CableDensity, FirstPhysicsConstraintComponent,
	                           MeshComponent, ROVMeshCableSocketName);
	FirstCablePiece->SetWorldRotation(Delta.Rotation());
	FirstCablePiece->RegisterComponent();
	FirstPhysicsConstraintComponent->RegisterComponent();

	CablePiecesAndConstraints.emplace_back(FirstCablePiece, FirstPhysicsConstraintComponent);

	for (uint32_t i = 1; i < InitialCableAmount; ++i)
	{
		auto NewCablePiece = NewObject<UCablePiece>(this, ToCStr("CablePiece" + FString::FromInt(i)));
		auto NewPhysicsConstraintComponent = NewObject<UPhysicsConstraintComponent>(this,
			ToCStr("PhysicsConstraint" + FString::FromInt(i)));
		NewCablePiece->SetupThis(i, CableDensity, NewPhysicsConstraintComponent,
		                         CablePiecesAndConstraints.back().first, UCablePiece::EndSocketName);
		NewCablePiece->SetWorldRotation(Delta.Rotation());
		NewCablePiece->RegisterComponent();
		NewPhysicsConstraintComponent->RegisterComponent();

		CablePiecesAndConstraints.emplace_back(NewCablePiece, NewPhysicsConstraintComponent);
	}

	// FinalPhysicsConstraintComponent
	FinalPhysicsConstraintComponent = NewObject<UPhysicsConstraintComponent>(
		this, "FinalPhysicsConstraintComponent");
	FinalPhysicsConstraintComponent->SetRelativeRotation(FRotator{90.f, 0.f, 0.f});
	FinalPhysicsConstraintComponent->SetAngularTwistLimit(ACM_Free, 0.f);
	FinalPhysicsConstraintComponent->SetAngularSwing1Limit(ACM_Free, 0.f);
	FinalPhysicsConstraintComponent->SetAngularSwing2Limit(ACM_Free, 0.f);
	FinalPhysicsConstraintComponent->SetLinearXLimit(LCM_Locked, CableOneLength);
	FinalPhysicsConstraintComponent->SetLinearYLimit(LCM_Locked, CableOneLength);
	FinalPhysicsConstraintComponent->SetLinearZLimit(LCM_Locked, CableOneLength);
	FinalPhysicsConstraintComponent->
		SetupAttachment(CablePiecesAndConstraints.back().first, UCablePiece::EndSocketName);
	FinalPhysicsConstraintComponent->SetConstrainedComponents(
		CablePiecesAndConstraints.back().first, NAME_None, GetEndComponent(), NAME_None);
	FinalPhysicsConstraintComponent->RegisterComponent();
}

void AROVPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	const auto EndComponent = GetEndComponent();
	const auto EndPosition = GetEndPosition();

	const FVector AbsoluteDelta = (RootComponent->GetComponentLocation() - EndPosition).GetAbs();
	const float WholeCableLength = CableOneLength * CablePiecesAndConstraints.size();
	const float CurrentLooseness = WholeCableLength / AbsoluteDelta.Size();
	if (CurrentLooseness < MinLooseness)
	{
		// 	// TODO: ask system to extend with speed
		// extend cable

		const float DesiredLengthOfCable = MinLooseness * AbsoluteDelta.Size();
		const uint32_t DesiredCablePiecesAmount = DesiredLengthOfCable / CableOneLength;
		const uint32_t AlreadyPresentCablePiecesAmount = CablePiecesAndConstraints.size();

		for (uint32_t i = AlreadyPresentCablePiecesAmount; i < DesiredCablePiecesAmount; ++i)
		{
			auto NewCablePiece = NewObject<UCablePiece>(this, ToCStr("CablePiece" + FString::FromInt(i)));
			auto NewPhysicsConstraintComponent = NewObject<UPhysicsConstraintComponent>(this,
				ToCStr("PhysicsConstraint" + FString::FromInt(i)));
			if (CablePiecesAndConstraints.size() == 0)
			{
				NewCablePiece->SetupThis(0, CableDensity, NewPhysicsConstraintComponent,
				                         MeshComponent, ROVMeshCableSocketName);
			}
			else
			{
				NewCablePiece->SetupThis(i, CableDensity, NewPhysicsConstraintComponent,
				                         CablePiecesAndConstraints.back().first, UCablePiece::EndSocketName);
			}

			NewCablePiece->RegisterComponent();
			NewPhysicsConstraintComponent->RegisterComponent();

			if (i == DesiredCablePiecesAmount - 1)
			{
				if (FinalPhysicsConstraintComponent)
				{
					FinalPhysicsConstraintComponent->UnregisterComponent();
				}
				// FinalPhysicsConstraintComponent
				FinalPhysicsConstraintComponent = NewObject<UPhysicsConstraintComponent>(
					this, "FinalPhysicsConstraintComponent");
				FinalPhysicsConstraintComponent->SetRelativeRotation(FRotator{90.f, 0.f, 0.f});
				FinalPhysicsConstraintComponent->SetAngularTwistLimit(ACM_Locked, 0.f);
				FinalPhysicsConstraintComponent->SetAngularSwing1Limit(ACM_Free, 0.f);
				FinalPhysicsConstraintComponent->SetAngularSwing2Limit(ACM_Free, 0.f);
				FinalPhysicsConstraintComponent->SetLinearXLimit(LCM_Locked, 0.f);
				FinalPhysicsConstraintComponent->SetLinearYLimit(LCM_Locked, 0.f);
				FinalPhysicsConstraintComponent->SetLinearZLimit(LCM_Locked, 0.f);
				FinalPhysicsConstraintComponent->SetupAttachment(NewCablePiece, UCablePiece::EndSocketName);
				FinalPhysicsConstraintComponent->SetConstrainedComponents(
					NewCablePiece, NAME_None, EndComponent, NAME_None);
				FinalPhysicsConstraintComponent->RegisterComponent();
			}

			CablePiecesAndConstraints.emplace_back(NewCablePiece, NewPhysicsConstraintComponent);
		}
	}
	else if (CurrentLooseness > MaxLooseness)
	{
		// retract cable
	}

	ApplyForcesToCables();

	DrawDebugSphere(GetWorld(), FinalPhysicsConstraintComponent->GetComponentLocation(), CableOneLength, 8,
	                FColor::Red, false, -1, 0, 2);
	GEngine->AddOnScreenDebugMessage(INDEX_NONE, 0.f, FColor::Emerald,
	                                 "Velocity: " + MovementComponent->Velocity.ToString());
	GEngine->AddOnScreenDebugMessage(INDEX_NONE, 0.f, FColor::Emerald,
	                                 "AbsoluteDelta: " + AbsoluteDelta.ToString());
}

void AROVPawn::ApplyForcesToCables()
{
	for (auto& CablePieceAndPhysicsConstraint : CablePiecesAndConstraints)
	{
		auto* CablePiece = CablePieceAndPhysicsConstraint.first;
		FVector Rotation = CablePiece->GetSocketLocation(UCablePiece::EndSocketName) -
			CablePiece->GetComponentLocation();

		FVector TangentVelocity = FlowVelocity.ProjectOnTo(Rotation);
		FVector NormalVelocity = FlowVelocity - TangentVelocity;

		DrawDebugDirectionalArrow(GetWorld(), CablePiece->GetComponentLocation(),
		                          CablePiece->GetComponentLocation() + FlowVelocity.GetSafeNormal() * CableOneLength,
		                          15, FColor::Blue, false, -1, 0, 6);

		DrawDebugDirectionalArrow(GetWorld(), CablePiece->GetComponentLocation(),
		                          CablePiece->GetComponentLocation() + TangentVelocity.GetSafeNormal() * CableOneLength,
		                          15, FColor::Cyan, false, -1, 0, 6);

		DrawDebugDirectionalArrow(GetWorld(), CablePiece->GetComponentLocation(),
		                          CablePiece->GetComponentLocation() + NormalVelocity.GetSafeNormal() * CableOneLength,
		                          15, FColor::Magenta, false, -1, 0, 6);

		FVector TangentForce = TangentVelocity.SizeSquared() * TangentVelocity.GetSafeNormal();
		FVector NormalForce = NormalVelocity.SizeSquared() * NormalVelocity.GetSafeNormal();
		CablePiece->AddForce(CableWeightAndWaterDisplacementForce + TangentForce + NormalForce);
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
