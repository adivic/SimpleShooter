// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SGrenade.h"
#include "SMolotov.generated.h"

/**
 * 
 */
UCLASS()
class SIMPLESHOOTER_API ASMolotov : public ASGrenade
{
	GENERATED_BODY()

public:
	ASMolotov();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);

public:
	virtual void Explode() override; 
};
