// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SWeapon.generated.h"

UENUM() 
enum class EFireType : uint8 {Semi, Bolt, Auto, Burst};

USTRUCT(BlueprintType)
struct FWeaponInfo {
	GENERATED_BODY()

	//Max Ammo reference
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 MaxAmmoRef;
	//Maxammo holding
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 MaxAmmo;
	//Bullets in the clip
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 FullClip;
	//Ammo in the magazine
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 CurrentAmmo;
	//Bullets fired in minute
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float FireRate;
	//Damage caused by weapon
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Damage;
	//Weapon recoil multiplier
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Recoil;
	//Weapon Firing type
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EFireType FireType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<EFireType> AvailableFireTypes;
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

	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components)
	//class USphereComponent* MeleeCheck;

	UPROPERTY(EditDefaultsOnly, Category = Weapon)
	class USoundCue* FiringSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	FWeaponInfo WeaponInfo;

	UPROPERTY(EditDefaultsOnly, Category = Weapon)
	TSubclassOf<class UDamageType> DamageType;

	UPROPERTY(EditDefaultsOnly, Category = Weapon)
	TMap<FString, class UAnimMontage*> GunMontages;

	UPROPERTY(BlueprintReadOnly, Category = Weapon)
	bool bIsFiring = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	float HipFireSpread;

	short Burst = 3;

	UFUNCTION(Server, Reliable)
	void ServerFire();

	//Derived from WeaponInfo.FireRate;
	float TimeBetweenShots;

	float LastFiredTime;

	FTimerHandle TimerHandle_FireHandle;

	float FindAndPlayMontage(FString MontageKey);

	void BurstFire();
	
	void FullAutoFire();

public:	

	// Line trace logic, and animations, same for every weapon type
	void Fire();

	virtual void Reload();

	virtual void StartFire();

	virtual void StopFire();

	void ChangeFireMode();

	FORCEINLINE bool GetIsFiring() const { return bIsFiring; }

	FORCEINLINE const FWeaponInfo& GetWeaponInfo() const { return WeaponInfo; }

	FORCEINLINE bool IsFullClip() const { return WeaponInfo.CurrentAmmo == WeaponInfo.FullClip; }

	FORCEINLINE bool CanReload() const { return WeaponInfo.MaxAmmo > 0; }
};
