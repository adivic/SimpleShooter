// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SGrenade.generated.h"

UENUM()
enum class EGrenadeType : uint8 {Lethal, Tactical};

UCLASS()
class SIMPLESHOOTER_API ASGrenade : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASGrenade();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = Components)
	UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleAnywhere, Category = Components)
	class USphereComponent* SphereCol;

	UPROPERTY(EditDefaultsOnly, Category = Grenade)
	class UParticleSystem* ExplosionEffect;

	UPROPERTY(EditDefaultsOnly, Category = Weapon)
	class USoundCue* ExplosionSound;

	UPROPERTY(EditDefaultsOnly, Category = Grenade)
	EGrenadeType GrenadeType;

	UPROPERTY(EditDefaultsOnly, Category = Grenade)
	float DamageRadius = 300.f;

public:	
	virtual void Explode();
};
