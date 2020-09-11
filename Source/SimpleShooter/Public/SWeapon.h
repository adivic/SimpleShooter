// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SWeapon.generated.h"

UENUM(BlueprintType) 
enum class EFireType : uint8 {
	Semi UMETA(DisplayName = "Semi"),
	Bolt UMETA(DisplayName = "Bolt"),
	Auto UMETA(DisplayName = "Auto"),
	Burst UMETA(DisplayName = "Burst")
};

USTRUCT()
struct FHitTrace {
	GENERATED_BODY()

	UPROPERTY()
	TEnumAsByte<EPhysicalSurface> SurfaceType;

	UPROPERTY()
	FVector_NetQuantize TraceTo;
};

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
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TEnumAsByte<EFireType> FireType;

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

	UPROPERTY(EditDefaultsOnly, Category = Weapon)
	class USoundCue* FiringSound;

	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	FWeaponInfo WeaponInfo;

	UPROPERTY(EditDefaultsOnly, Category = Weapon)
	TSubclassOf<class UDamageType> DamageType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	class UParticleSystem* PathEffect;

	UPROPERTY(BlueprintReadOnly, Category = Weapon)
	bool bIsFiring = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	float HipFireSpread;

	UPROPERTY(ReplicatedUsing = OnRep_HitTrace)
	FHitTrace HitTrace;

	short Burst = 3;

	//Derived from WeaponInfo.FireRate;
	float TimeBetweenShots;

	float LastFiredTime;

	FTimerHandle TimerHandle_FireHandle;

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFire();

	UFUNCTION(NetMulticast, Reliable)
	void MultiFindAndPlayMontage(const FString& MontageKey);

	UFUNCTION()
	void OnRep_HitTrace();

	void PlayFireEffects(FVector TraceEnd);

	void BurstFire();
	
	void FullAutoFire();

	void PlayImpactEffect(EPhysicalSurface SurfaceType, FVector ImpactPoint);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	UParticleSystem* DefaultImpactEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	UParticleSystem* BodyImpactEffect;

public:	

	// Line trace logic, and animations, same for every weapon type
	void Fire();

	void ServerPlayMontage(const FString& Key);

	virtual void Reload();

	virtual void StartFire();

	virtual void StopFire();

	void ChangeFireMode();

	UPROPERTY(BlueprintReadOnly, Category = Weapon)
	class USWeaponDataAsset* WeaponData;

	FORCEINLINE bool GetIsFiring() const { return bIsFiring; }

	FORCEINLINE const FWeaponInfo& GetWeaponInfo() const { return WeaponInfo; }

	FORCEINLINE bool IsFullClip() const { return WeaponInfo.CurrentAmmo == WeaponInfo.FullClip; }

	FORCEINLINE bool CanReload() const { return WeaponInfo.MaxAmmo > 0; }
};
