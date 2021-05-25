// Fill out your copyright notice in the Description page of Project Settings.


#include "ShortCable.h"


// Sets default values
AShortCable::AShortCable()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AShortCable::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AShortCable::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

