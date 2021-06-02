// Illarionov Kirill's Graduation Project for BMSTU

#include "CablePiece.h"

#include "Engine/CollisionProfile.h"


UCablePiece::UCablePiece() : Super{}
{
	PrimaryComponentTick.bCanEverTick = true;
	bTickInEditor = true;
	bAutoActivate = true;
	bIgnoreRadialForce = true;

	SetStaticMesh(ConstructorHelpers::FObjectFinder<UStaticMesh>(
		TEXT("StaticMesh'/Game/Simulation/Cable.Cable'")).Object);
	SetEnableGravity(true);
	SetSimulatePhysics(true);
	SetMobility(EComponentMobility::Movable);
	SetCollisionProfileName(UCollisionProfile::PhysicsActor_ProfileName);
	SetLinearDamping(0.5);
	SetAngularDamping(0.5);
}

void UCablePiece::SetupThis(uint32_t CableId, float Density, float WaterDensity,
                            UPhysicsConstraintComponent* CreatedPhysicsConstraintComponent,
                            UPrimitiveComponent* AttachableComponent,
                            const FName AttachableComponentSocketName)
{
	Id = CableId;

	SetupAttachment(AttachableComponent, AttachableComponentSocketName);
	SetMassOverrideInKg(NAME_None, GetStaticMesh()->GetBoundingBox().GetVolume() * PI * Density / 1e6 / 4);
	WaterDisplacementForce = FVector{
		0.f,
		0.f,
		GetStaticMesh()->GetBoundingBox().GetVolume() * PI * WaterDensity / 1e6f / 4.f
		* -GetWorld()->GetWorldSettings()->GetGravityZ()
	};

	// PhysicsConstraintComponent
	WeakPhysicsConstraintComponent = CreatedPhysicsConstraintComponent;
	WeakPhysicsConstraintComponent->SetRelativeRotation(FRotator{90.f, 0.f, 0.f});
	WeakPhysicsConstraintComponent->SetAngularTwistLimit(ACM_Locked, 0);
	WeakPhysicsConstraintComponent->SetAngularSwing1Limit(ACM_Limited, 90);
	WeakPhysicsConstraintComponent->SetAngularSwing2Limit(ACM_Limited, 90);
	WeakPhysicsConstraintComponent->SetLinearXLimit(LCM_Locked, 0.f);
	WeakPhysicsConstraintComponent->SetLinearYLimit(LCM_Locked, 0.f);
	WeakPhysicsConstraintComponent->SetLinearZLimit(LCM_Locked, 0.f);
	WeakPhysicsConstraintComponent->SetAngularBreakable(false, 1000000);
	WeakPhysicsConstraintComponent->SetLinearBreakable(false, 1000000);
	WeakPhysicsConstraintComponent->SetupAttachment(AttachableComponent, AttachableComponentSocketName);
	WeakAttachableComponent = AttachableComponent;
}

UCablePiece::~UCablePiece()
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(INDEX_NONE, 1.f, FColor::Red, "Deleted CableId: " + FString::FromInt(Id));
	}
}

FVector UCablePiece::GetWaterDisplacementForce() const
{
	return WaterDisplacementForce;
}

void UCablePiece::BeginPlay()
{
	Super::BeginPlay();
	WeakPhysicsConstraintComponent->SetConstrainedComponents(this, NAME_None, WeakAttachableComponent, NAME_None);
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(INDEX_NONE, 10.f, FColor::Blue,
										FString::Printf(TEXT("Created CableId: %d | Mass: %f"), Id, GetMass()));	
	}
}

void UCablePiece::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}
