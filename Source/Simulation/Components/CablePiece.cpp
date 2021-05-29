// Illarionov Kirill's Graduation Project for BMSTU

#include "CablePiece.h"

#include "Engine/CollisionProfile.h"


UCablePiece::UCablePiece() : Super{}
{
	PrimaryComponentTick.bCanEverTick = true;
	bTickInEditor = true;
	bAutoActivate = true;
	bIgnoreRadialForce = false;

	SetStaticMesh(
		ConstructorHelpers::FObjectFinder<UStaticMesh>(TEXT("StaticMesh'/Game/Simulation/Cable.Cable'")).Object);
	SetEnableGravity(false);
	SetSimulatePhysics(true);
	SetMobility(EComponentMobility::Movable);
	SetCollisionProfileName(UCollisionProfile::PhysicsActor_ProfileName);
}

void UCablePiece::SetupThis(uint32_t CableId, float Density,
                            UPhysicsConstraintComponent* CreatedPhysicsConstraintComponent,
                            UPrimitiveComponent* AttachableComponent,
                            const FName AttachableComponentSocketName)
{
	Id = CableId;

	SetupAttachment(AttachableComponent, AttachableComponentSocketName);
	SetMassOverrideInKg(NAME_None, GetStaticMesh()->GetBoundingBox().GetVolume() * PI * Density / 1e6 / 4);

	// PhysicsConstraintComponent
	WeakPhysicsConstraintComponent = CreatedPhysicsConstraintComponent;
	WeakPhysicsConstraintComponent->SetRelativeRotation(FRotator{90.f, 0.f, 0.f});
	WeakPhysicsConstraintComponent->SetAngularTwistLimit(ACM_Locked, 0);
	WeakPhysicsConstraintComponent->SetAngularSwing1Limit(ACM_Limited, 90);
	WeakPhysicsConstraintComponent->SetAngularSwing2Limit(ACM_Limited, 90);
	WeakPhysicsConstraintComponent->SetupAttachment(AttachableComponent, AttachableComponentSocketName);
	WeakAttachableComponent = AttachableComponent;
}

UCablePiece::~UCablePiece()
{
	GEngine->AddOnScreenDebugMessage(INDEX_NONE, 1.f, FColor::Red, "deleted: " + FString::FromInt(Id));
}

void UCablePiece::BeginPlay()
{
	Super::BeginPlay();
	WeakPhysicsConstraintComponent->SetConstrainedComponents(this, NAME_None, WeakAttachableComponent, NAME_None);
	GEngine->AddOnScreenDebugMessage(INDEX_NONE, 10.f, FColor::Blue,
	                                 FString::Printf(TEXT("Created CableId: %d | Mass: %f"), Id, GetMass()));
}

void UCablePiece::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}
