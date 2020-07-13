// Fill out your copyright notice in the Description page of Project Settings.


#include "SWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystem.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

// Sets default values
ASWeapon::ASWeapon()
{
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	RootComponent = MeshComp;

	//Initializing default values
	WeaponInfo.CurrentAmmo = 30;
	WeaponInfo.FullClip = 30;
	WeaponInfo.MaxAmmo = 100;
	WeaponInfo.MaxAmmoRef = 100;
	WeaponInfo.Damage = 20;
	WeaponInfo.FireType = EFireType::Auto;
	WeaponInfo.FireRate = 650;
}

// Called when the game starts or when spawned
void ASWeapon::BeginPlay()
{
	Super::BeginPlay();
	
	TimeBetweenShots = 60 / WeaponInfo.FireRate;
}

void ASWeapon::Fire() {
	AActor* MyOwner = GetOwner();
	const UWorld* World = GetWorld();
	if (MyOwner && WeaponInfo.CurrentAmmo > 0) {
		WeaponInfo.CurrentAmmo--;

		//Bullet Spread?
		FVector EyeLocation;
		FRotator EyeRotator;
		MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotator);

		FCollisionQueryParams Params;
		Params.AddIgnoredActor(MyOwner);
		Params.AddIgnoredActor(this);
		Params.bTraceComplex = true;
		
		//Cast to ASCharacter?
		FVector EndTrace = EyeLocation + (EyeRotator.Vector() * 10000.f);
		FHitResult Hit;
		World->LineTraceSingleByChannel(Hit, EyeLocation, EndTrace, ECC_Visibility, Params);
		if (Hit.bBlockingHit) {
			AActor* HitActor = Hit.GetActor();
			UGameplayStatics::ApplyPointDamage(HitActor, WeaponInfo.Damage, EyeRotator.Vector(), Hit, MyOwner->GetInstigatorController(), MyOwner, DamageType);
		}

		//Debug Line of fire
		DrawDebugLine(World, EyeLocation, EndTrace, FColor::Red, false, 5.f);

		//Visual Effects
		FName SocketName = FName("MuzzleSocket");
		FVector SocketLocation = MeshComp->GetSocketLocation(SocketName);
		FRotator SocketRotator = MeshComp->GetSocketRotation(SocketName);

		UGameplayStatics::SpawnEmitterAtLocation(World, MuzzleEffect, SocketLocation, SocketRotator);
		UGameplayStatics::SpawnSoundAtLocation(World, FiringSound, SocketLocation);

		//Set LastFireTime
		LastFiredTime = World->TimeSeconds;
	}
}

void ASWeapon::Reload() {
	if (WeaponInfo.MaxAmmo > 0) {
		if (WeaponInfo.CurrentAmmo > 0) {
			int32 AmmoDifference = WeaponInfo.FullClip - WeaponInfo.CurrentAmmo;
			if (WeaponInfo.MaxAmmo - AmmoDifference >= 0) {
				WeaponInfo.CurrentAmmo = WeaponInfo.FullClip;
				WeaponInfo.MaxAmmo -= AmmoDifference;
			} else {
				WeaponInfo.CurrentAmmo += WeaponInfo.MaxAmmo;
				WeaponInfo.MaxAmmo = 0;
			}
		} else {
			if (WeaponInfo.MaxAmmo - WeaponInfo.FullClip > 0) {
				WeaponInfo.CurrentAmmo = WeaponInfo.FullClip;
				WeaponInfo.MaxAmmo -= WeaponInfo.FullClip;
			} else {
				WeaponInfo.CurrentAmmo = WeaponInfo.MaxAmmo;
				WeaponInfo.MaxAmmo = 0;
			}
		}
	}
}

void ASWeapon::StartFire() {
	float FirstDelay = FMath::Max(LastFiredTime - TimeBetweenShots - GetWorld()->TimeSeconds, 0.0f);
	GetWorldTimerManager().SetTimer(TimerHandle_FireHandle, this, &ASWeapon::Fire, TimeBetweenShots, true, FirstDelay);
}

void ASWeapon::StopFire() {
	GetWorldTimerManager().ClearTimer(TimerHandle_FireHandle);
}

