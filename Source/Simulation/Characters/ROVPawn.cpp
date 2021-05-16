// Fill out your copyright notice in the Description page of Project Settings.

#include "ROVPawn.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/CollisionProfile.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/SpringArmComponent.h"


AROVPawn::AROVPawn()
{
    // Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    // Don't rotate when the controller rotates. Let that just affect the camera.
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // CapsuleComponent
    CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>("CapsuleComponent");
    CapsuleComponent->InitCapsuleSize(34.0f, 88.0f);
    CapsuleComponent->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);

    CapsuleComponent->CanCharacterStepUpOn = ECB_No;
    CapsuleComponent->SetShouldUpdatePhysicsVolume(true);
    CapsuleComponent->SetCanEverAffectNavigation(false);
    CapsuleComponent->bDynamicObstacle = true;
    RootComponent = CapsuleComponent;

    // FloatingPawnMovement
    FloatingPawnMovement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("FloatingPawnMovement"));

    // CameraBoom
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character
    CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

    // FollowCamera
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
}

/*void AROVPawn::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    TArray<UStaticMeshComponent*> StaticMeshComps;
    GetComponents<UStaticMeshComponent>(StaticMeshComps);
    for (auto* StaticMeshComponent : StaticMeshComps)
    {
        StaticMeshComponent->AddWorldRotation(FQuat{Rotation * DeltaTime});
        StaticMeshComponent->AddWorldOffset(StaticMeshComponent->GetForwardVector() * MarchSpeed);
        AddActorWorldOffset(StaticMeshComponent->GetForwardVector() * MarchSpeed);
    }
    GEngine->AddOnScreenDebugMessage(INDEX_NONE, 0.0f, FColor::Emerald, FString::Printf(TEXT("%f"), MarchSpeed));
    MarchSpeed -= DeltaTime * MarchResistanceScale * MarchSpeed;
    if (std::abs(MarchSpeed) <= std::numeric_limits<float>::epsilon())
    {
        MarchSpeed = 0;
    }
}*/

// Called to bind functionality to input
void AROVPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    check(PlayerInputComponent);

    PlayerInputComponent->BindAxis("MoveForward", this, &AROVPawn::MoveForward);
    PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
    PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
    PlayerInputComponent->BindAxis("RollRotation", this, &AROVPawn::RollRotation);
    PlayerInputComponent->BindAxis("YawRotation", this, &AROVPawn::YawRotation);
    PlayerInputComponent->BindAxis("PitchRotation", this, &AROVPawn::PitchRotation);
}

void AROVPawn::MoveForward(float Amount)
{
    if (Amount == 0.0f) { return; }

    // const FVector Direction = FRotationMatrix{GetActorRotation()}.GetUnitAxis(EAxis::X);
    AddMovementInput(GetActorForwardVector(), Amount);
}

void AROVPawn::RollRotation(float Amount)
{
    if (Amount == 0.0f) { return; }

    AddActorLocalRotation(FQuat{FRotator{0.0f, 0.0f, Amount}});
}

void AROVPawn::YawRotation(float Amount)
{
    if (Amount == 0.0f) { return; }

    AddActorLocalRotation(FQuat{FRotator{0.0f, Amount, 0.0f}});
}

void AROVPawn::PitchRotation(float Amount)
{
    if (Amount == 0.0f) { return; }

    AddActorLocalRotation(FQuat{FRotator{Amount, 0.0f, 0.0f}});
}
