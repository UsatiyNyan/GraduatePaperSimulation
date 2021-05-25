// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "ROVPawn.generated.h"

UCLASS(config=Game)
class SIMULATION_API AROVPawn : public APawn
{
    GENERATED_BODY()

public:
    AROVPawn();

protected:
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
    virtual void Tick(float DeltaTime) override;

protected:
    void MoveForward(float Amount);

    void RollRotation(float Amount);
    void PitchRotation(float Amount);
    void YawRotation(float Amount);

    void Scroll(float Amount);

    void Stop();

private:
    FVector Velocity;
    
public:
    /** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
    float YawRate{1.f};

    /** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
    float PitchRate{1.f};

public:
    virtual UPawnMovementComponent* GetMovementComponent() const override;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Pawn, meta=(AllowPrivateAccess="true"))
    class UCapsuleComponent* CapsuleComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Pawn, meta=(AllowPrivateAccess="true"))
    class UPawnMovementComponent* MovementComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera, meta=(AllowPrivateAccess="true"))
    class USpringArmComponent* CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera, meta=(AllowPrivateAccess="true"))
    class UCameraComponent* FollowCamera;

    // UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Pawn, meta=(AllowPrivateAccess="true"))
    // UMeshComponent* Mesh;
};
