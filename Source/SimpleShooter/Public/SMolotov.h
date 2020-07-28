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

	bool bIsOverlapp;
	
	bool bActive;

	UFUNCTION()
	void OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
	void TakeFireDamage(AActor* DamagedActor, float BaseDamage);

	TArray<AActor*> OverlappingActors;

	TArray<FTimerHandle> ActorHandles;

public:
	virtual void Explode() override; 

	//virtual void Tick
};
