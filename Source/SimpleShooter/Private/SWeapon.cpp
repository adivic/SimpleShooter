// Fill out your copyright notice in the Description page of Project Settings.


#include "SWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Animation/AnimMontage.h"
#include "SCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Net/UnrealNetwork.h"

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
	WeaponInfo.AvailableFireTypes.Add(WeaponInfo.FireType);
	WeaponInfo.FireRate = 650;
	WeaponInfo.Recoil = .4f;

	HipFireSpread = 4.f;
	TimeBetweenShots = 60 / WeaponInfo.FireRate;

	SetReplicates(true);
}

// Called when the game starts or when spawned
void ASWeapon::BeginPlay()
{
	Super::BeginPlay();
}

void ASWeapon::ServerFire_Implementation() {
	Fire();
}

void ASWeapon::OnRep_HitTrace() {
	PlayFireEffects(HitTrace.TraceTo);
	//SpawnImpactEffects(HitTrace.SurfaceType, HitTrace.TraceTo);
}

void ASWeapon::PlayFireEffects(FVector TraceEnd) {
	ServerPlayMontage("Fire");
	FVector MuzzleLocation = MeshComp->GetSocketLocation("MuzzleSocket");

	if (PathEffect) {
		UParticleSystemComponent* TracerComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), PathEffect, MuzzleLocation);
		if (TracerComp) {
			TracerComp->SetVectorParameter("Target", TraceEnd);
		}
	}
	if (FiringSound)
		UGameplayStatics::PlaySoundAtLocation(this, FiringSound, MuzzleLocation);
}

void ASWeapon::PlayImpactEffects(EPhysicalSurface SurfaceType, FVector ImpactPoint) {
	//TODO Add later
}

void ASWeapon::MultiFindAndPlayMontage_Implementation(const FString& MontageKey) {
	auto MontageToPlay = GunMontages.Find(MontageKey);
	if (MontageToPlay) {
		MeshComp->GetAnimInstance()->Montage_Play(*MontageToPlay);
	}
}

void ASWeapon::ServerPlayMontage_Implementation(const FString& Key) {
	MultiFindAndPlayMontage(Key);
}

void ASWeapon::BurstFire() {
	if (Burst != 0) {
		Fire();
		FTimerDelegate RespawnDelegate = FTimerDelegate::CreateUObject(this, &ASWeapon::BurstFire);
		GetWorldTimerManager().SetTimer(TimerHandle_FireHandle, RespawnDelegate, .1f, true);
		--Burst;

		return;
	}
	bIsFiring = false;
	GetWorldTimerManager().ClearTimer(TimerHandle_FireHandle);
	Burst = 3;
}

void ASWeapon::FullAutoFire() {
	float FirstDelay = FMath::Max(LastFiredTime - TimeBetweenShots - GetWorld()->TimeSeconds, 0.0f);
	GetWorldTimerManager().SetTimer(TimerHandle_FireHandle, this, &ASWeapon::Fire, TimeBetweenShots, true, FirstDelay);
}

void ASWeapon::Fire() {
	if (GetOwner()->GetLocalRole() < ROLE_Authority) {
		ServerFire();
	}
	ASCharacter* MyChar = Cast<ASCharacter>(GetOwner());
	const UWorld* World = GetWorld();
	if (MyChar && WeaponInfo.CurrentAmmo > 0) {
		WeaponInfo.CurrentAmmo--;
		
		FName SocketName = FName("MuzzleSocket");
		auto Camera = MyChar->GetPlayerCamera();
		FVector EyeLocation = MyChar->bAiming ? MeshComp->GetSocketLocation(SocketName) : Camera->GetComponentLocation();
		FRotator EyeRotator = MyChar->bAiming ? MeshComp->GetSocketRotation(SocketName) : Camera->GetComponentRotation();

		FVector Direction = EyeRotator.Vector();
		
		//HipFire Spread
		if(!MyChar->bAiming) {
			float HalfRadius = FMath::DegreesToRadians(HipFireSpread); 
			Direction = FMath::VRandCone(Direction, HalfRadius);
		}

		FCollisionQueryParams Params;
		Params.AddIgnoredActor(MyChar);
		Params.AddIgnoredActor(this);
		Params.bTraceComplex = true;
		Params.bReturnPhysicalMaterial = true;

		//Recoil 
		//TODO Smoothness 
		float RecoilPitch = WeaponInfo.Recoil * FMath::FRandRange(-.1f, -.5f);
		MyChar->AddControllerPitchInput(FMath::FInterpTo(0, RecoilPitch, World->DeltaTimeSeconds, 10));
		float RecoilYaw = WeaponInfo.Recoil * FMath::FRandRange(-.7f, .7f);
		MyChar->AddControllerYawInput(FMath::FInterpTo(0, RecoilYaw, World->DeltaTimeSeconds, 10));
		
		//Cast to ASCharacter?
		FVector EndTrace = EyeLocation + (Direction * 10000.f);
		FVector TraceEndPoint = EndTrace;
		EPhysicalSurface SurfaceType = SurfaceType_Default;
		FHitResult Hit;
		World->LineTraceSingleByChannel(Hit, EyeLocation, EndTrace, ECC_Visibility, Params);
		if (Hit.bBlockingHit) {
			TraceEndPoint = Hit.ImpactPoint;
			AActor* HitActor = Hit.GetActor();
			SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());
			UGameplayStatics::ApplyPointDamage(HitActor, WeaponInfo.Damage, Direction, Hit, MyChar->GetInstigatorController(), MyChar, DamageType);

			//PLayImpactEffect(SurfaceType, Hit.ImpactPoint);
		}

		//Debug Line of fire
		DrawDebugLine(World, EyeLocation, Hit.Location, FColor::Red, false, 5.f);
		DrawDebugPoint(World, Hit.Location, 10, FColor::Red, false, 5.f);
		
		//Trail Effect, Sound & Fire Animation
		PlayFireEffects(TraceEndPoint);

		//Set LastFireTime
		LastFiredTime = World->TimeSeconds;

		if (GetLocalRole() == ROLE_Authority) {
			HitTrace.TraceTo = TraceEndPoint;
			HitTrace.SurfaceType = SurfaceType;
		}
	}
}

void ASWeapon::Reload_Implementation() {
	if (WeaponInfo.MaxAmmo > 0) {
		ServerPlayMontage("Reload");
		if (WeaponInfo.CurrentAmmo > 0) {
			short AmmoDifference = WeaponInfo.FullClip - WeaponInfo.CurrentAmmo;
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
	if (WeaponInfo.CurrentAmmo <= 0) {
		StopFire();
		return;
	}
	bIsFiring = true;

	switch (WeaponInfo.FireType) {
		case EFireType::Auto:
			FullAutoFire();
			break;
		case EFireType::Burst:
			if (Burst == 3)
				BurstFire();
			break;
		case EFireType::Semi:
		case EFireType::Bolt:
			bIsFiring = true;
			Fire();
			bIsFiring = false;
			break;
	}
}

void ASWeapon::StopFire() {
	if (WeaponInfo.FireType != EFireType::Burst) {
		bIsFiring = false;
		GetWorldTimerManager().ClearTimer(TimerHandle_FireHandle);
	}
}

void ASWeapon::ChangeFireMode() {
	if (WeaponInfo.AvailableFireTypes.Num() <= 1 || bIsFiring) return;
	short index = WeaponInfo.AvailableFireTypes.Find(WeaponInfo.FireType);
	if (index+1 < WeaponInfo.AvailableFireTypes.Num()) {
		WeaponInfo.FireType = WeaponInfo.AvailableFireTypes[++index];
	} else {
		WeaponInfo.FireType = WeaponInfo.AvailableFireTypes[0];
	}
}

void ASWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ASWeapon, HitTrace, COND_SkipOwner);
	DOREPLIFETIME(ASWeapon, WeaponInfo);
}