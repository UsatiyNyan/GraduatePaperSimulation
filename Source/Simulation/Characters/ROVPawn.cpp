// Fill out your copyright notice in the Description page of Project Settings.

#include "ROVPawn.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/CollisionProfile.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/MovementComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"


AROVPawn::AROVPawn()
{
    // Don't rotate when the controller rotates. Let that just affect the camera.
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // CapsuleComponent
    CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>("CapsuleComponent");
    CapsuleComponent->InitCapsuleSize(60.f, 40.f);
    CapsuleComponent->SetCollisionProfileName(UCollisionProfile::Vehicle_ProfileName);

    CapsuleComponent->CanCharacterStepUpOn = ECB_No;
    CapsuleComponent->SetShouldUpdatePhysicsVolume(true);
    CapsuleComponent->SetCanEverAffectNavigation(false);
    CapsuleComponent->bDynamicObstacle = true;
    SetRootComponent(CapsuleComponent);

    // FloatingPawnMovement
    MovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("FloatingPawnMovement"));
    MovementComponent->SetUpdatedComponent(CapsuleComponent);

    // CameraBoom
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 300.f; // The camera follows at this distance behind the character
    CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

    // FollowCamera
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
}

void AROVPawn::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    AddMovementInput(GetActorForwardVector() * Velocity.X, DeltaTime);
    GEngine->AddOnScreenDebugMessage(INDEX_NONE, 0.f, FColor::Emerald,
                                     FString::Printf(TEXT("%f %f %f"),
                                                     Velocity.X,
                                                     Velocity.Y,
                                                     Velocity.Z));
}

// Called to bind functionality to input
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
    
    PlayerInputComponent->BindAction("Stop", EInputEvent::IE_Pressed, this, &AROVPawn::Stop);
}

void AROVPawn::MoveForward(float Amount)
{
    if (Amount == 0.f) { return; }

    Velocity.X += Amount;
}

void AROVPawn::RollRotation(float Amount)
{
    if (Amount == 0.f) { return; }

    AddActorLocalRotation(FQuat{FRotator{0.f, 0.f, Amount}});
}

void AROVPawn::PitchRotation(float Amount)
{
    if (Amount == 0.f) { return; }

    AddActorLocalRotation(FQuat{FRotator{Amount, 0.f, 0.f}});
}

void AROVPawn::YawRotation(float Amount)
{
    if (Amount == 0.f) { return; }

    AddActorLocalRotation(FQuat{FRotator{0.f, Amount, 0.f}});
}

void AROVPawn::Scroll(float Amount)
{
    if (Amount == 0.f) { return; }

    CameraBoom->TargetArmLength += 10 * Amount;
}

void AROVPawn::Stop()
{
    Velocity.Set(0.f, 0.f, 0.f);
}

UPawnMovementComponent* AROVPawn::GetMovementComponent() const
{
    return MovementComponent;
}
