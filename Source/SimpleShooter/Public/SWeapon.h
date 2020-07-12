// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SWeapon.generated.h"

USTRUCT()
struct FWeaponInfo {
	GENERATED_BODY()

public:
	UPROPERTY()
	int32 MaxAmmoRef;

	UPROPERTY()
	int32 MaxAmmo;

	UPROPERTY()
	int32 FullClip;

	UPROPERTY()
	int32 CurrentAmmo;

	UPROPERTY()
	float FireRate;
};


UCLASS()
class SIMPLESHOOTER_API ASWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASWeapon();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components)
	USkeletalMeshComponent* MeshComp;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
