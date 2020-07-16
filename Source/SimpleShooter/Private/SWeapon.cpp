// Fill out your copyright notice in the Description page of Project Settings.


#include "SWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Animation/AnimMontage.h"
#include "SCharacter.h"

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
	WeaponInfo.FireType = WeaponInfo.AvailableFireTypes[0];
	WeaponInfo.FireRate = 650;
}

// Called when the game starts or when spawned
void ASWeapon::BeginPlay()
{
	Super::BeginPlay();
	
	TimeBetweenShots = 60 / WeaponInfo.FireRate; 
}

void ASWeapon::FindAndPlayMontage(FString MontageKey) {
	auto MontageToPlay = GunMontages.Find(MontageKey);
	if (MontageToPlay) {
		MeshComp->GetAnimInstance()->Montage_Play(*MontageToPlay);
	}
}

void ASWeapon::BurstFire(short Bursts) {
	if (Bursts != 0) {
		Fire();
		FTimerDelegate RespawnDelegate = FTimerDelegate::CreateUObject(this, &ASWeapon::BurstFire, --Bursts);
		GetWorldTimerManager().SetTimer(TimerHandle_FireHandle, RespawnDelegate, .1f, true);
	}
}

void ASWeapon::FullAutoFire() {
	float FirstDelay = FMath::Max(LastFiredTime - TimeBetweenShots - GetWorld()->TimeSeconds, 0.0f);
	bIsFiring = true;
	GetWorldTimerManager().SetTimer(TimerHandle_FireHandle, this, &ASWeapon::Fire, TimeBetweenShots, true, FirstDelay);
}

void ASWeapon::Fire() {
	AActor* MyOwner = GetOwner();
	const UWorld* World = GetWorld();
	if (MyOwner && WeaponInfo.CurrentAmmo > 0) {
		WeaponInfo.CurrentAmmo--;

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

		//Sound
		FName SocketName = FName("MuzzleSocket");
		FVector SocketLocation = MeshComp->GetSocketLocation(SocketName);

		UGameplayStatics::SpawnSoundAtLocation(World, FiringSound, SocketLocation);

		//Set LastFireTime
		LastFiredTime = World->TimeSeconds;

		//Play Fire montage
		FindAndPlayMontage("Fire");
		
	}
}

void ASWeapon::Reload() {
	if (WeaponInfo.MaxAmmo > 0) {
		FindAndPlayMontage("Reload");
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
	switch (WeaponInfo.FireType) {
		case EFireType::Auto:
			FullAutoFire();
			break;
		case EFireType::Burst:
			BurstFire(3);
			break;
		case EFireType::Semi:
		case EFireType::Bolt:
			ASWeapon::Fire();
			break;
	}
}

void ASWeapon::StopFire() {
	bIsFiring = false;
	GetWorldTimerManager().ClearTimer(TimerHandle_FireHandle);
}

void ASWeapon::ChangeFireMode() {
	if (WeaponInfo.AvailableFireTypes.Num() <= 1) return;
	short index = WeaponInfo.AvailableFireTypes.Find(WeaponInfo.FireType);
	if (index+1 < WeaponInfo.AvailableFireTypes.Num()) {
		WeaponInfo.FireType = WeaponInfo.AvailableFireTypes[++index];
	} else {
		WeaponInfo.FireType = WeaponInfo.AvailableFireTypes[0];
	}
}