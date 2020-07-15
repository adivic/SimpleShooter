// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SWeapon.generated.h"

UENUM() enum class EFireType : uint8 {Semi, Bolt, Auto, Burst};

USTRUCT(BlueprintType)
struct FWeaponInfo {
	GENERATED_BODY()

	//Max Ammo reference
	UPROPERTY(EditDefaultsOnly)
	int32 MaxAmmoRef;
	//Maxammo holding
	UPROPERTY(EditDefaultsOnly)
	int32 MaxAmmo;
	//Bullets in the clip
	UPROPERTY(EditDefaultsOnly)
	int32 FullClip;
	//Ammo in the magazine
	UPROPERTY(EditDefaultsOnly)
	int32 CurrentAmmo;
	//Bullets fired in minute
	UPROPERTY(EditDefaultsOnly)
	float FireRate;
	//Damage caused by weapon
	UPROPERTY(EditDefaultsOnly)
	float Damage;
	//Weapon Firing type
	UPROPERTY(EditDefaultsOnly)
	EFireType FireType;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponFire, float, Damage, bool, bIsFiring);

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

	UPROPERTY(EditDefaultsOnly, Category = Weapon)
	class USoundCue* FiringSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	FWeaponInfo WeaponInfo;

	UPROPERTY(EditDefaultsOnly, Category = Weapon)
	TSubclassOf<class UDamageType> DamageType;

	UPROPERTY(EditDefaultsOnly, Category = Weapon)
	TMap<FString, class UAnimMontage*> GunMontages;

	//Derived from WeaponInfo.FireRate;
	float TimeBetweenShots;

	float LastFiredTime;

	FTimerHandle TimerHandle_FireHandle;

	void FindAndPlayMontage(FString MontageKey);

public:	

	// Line trace logic, and animations, same for every weapon type
	void Fire();

	UPROPERTY(BlueprintReadOnly, Category = Weapon)
	bool bIsFiring = false;

	virtual void Reload();

	// Override in child class to implement different firing logic eg. burst
	virtual void StartFire();

	virtual void StopFire();

	FORCEINLINE bool GetIsFiring() const { return bIsFiring; }

	FORCEINLINE const FWeaponInfo& GetWeaponInfo() const { return WeaponInfo; }
};
