// Fill out your copyright notice in the Description page of Project Settings.


#include "SWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Animation/AnimMontage.h"
#include "SCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"


// Sets default values
ASWeapon::ASWeapon()
{
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	RootComponent = MeshComp;

	/*MeleeCheck = CreateDefaultSubobject<USphereComponent>(TEXT("MeleeCheck"));
	MeleeCheck->SetupAttachment(MeshComp);
	MeleeCheck->SetSphereRadius(15);
	MeleeCheck->SetGenerateOverlapEvents(true);
	*/
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

float ASWeapon::FindAndPlayMontage(FString MontageKey) {
	auto MontageToPlay = GunMontages.Find(MontageKey);
	if (MontageToPlay) {
		return MeshComp->GetAnimInstance()->Montage_Play(*MontageToPlay, 1.f, EMontagePlayReturnType::MontageLength, 0, true);
	}
	return 0.f;
}

void ASWeapon::BurstFire() {
	//Fix-at
	if (Burst != 0) {
		Fire();
		FTimerDelegate RespawnDelegate = FTimerDelegate::CreateUObject(this, &ASWeapon::BurstFire, --Burst);
		GetWorldTimerManager().SetTimer(TimerHandle_FireHandle, RespawnDelegate, .1f, true);

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
	ASCharacter* MyChar = Cast<ASCharacter>(GetOwner());
	const UWorld* World = GetWorld();
	if (MyChar && WeaponInfo.CurrentAmmo > 0) {
		WeaponInfo.CurrentAmmo--;

		auto Camera = MyChar->GetPlayerCamera();
		FVector EyeLocation = Camera->GetComponentLocation();
		FRotator EyeRotator = Camera->GetComponentRotation();

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

		//Recoil 
		//TODO Smoothness 
		float RecoilPitch = WeaponInfo.Recoil * FMath::FRandRange(-.1f, -.5f);
		MyChar->AddControllerPitchInput(FMath::FInterpTo(0, RecoilPitch, World->DeltaTimeSeconds, 10));
		float RecoilYaw = WeaponInfo.Recoil * FMath::FRandRange(-.7f, .7f);
		MyChar->AddControllerYawInput(FMath::FInterpTo(0, RecoilYaw, World->DeltaTimeSeconds, 10));
		
		//Cast to ASCharacter?
		FVector EndTrace = EyeLocation + (Direction * 10000.f);
		FHitResult Hit;
		World->LineTraceSingleByChannel(Hit, EyeLocation, EndTrace, ECC_Visibility, Params);
		if (Hit.bBlockingHit) {
			AActor* HitActor = Hit.GetActor();
			UGameplayStatics::ApplyPointDamage(HitActor, WeaponInfo.Damage, Direction, Hit, MyChar->GetInstigatorController(), MyChar, DamageType);
		}

		//Debug Line of fire
		DrawDebugLine(World, EyeLocation, Hit.Location, FColor::Red, false, 5.f);
		DrawDebugPoint(World, Hit.Location, 15, FColor::Red, false, 5.f);

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
				BurstFire(3);
			break;
		case EFireType::Semi:
		case EFireType::Bolt:
			ASWeapon::Fire();
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
	if (WeaponInfo.AvailableFireTypes.Num() <= 1) return;
	short index = WeaponInfo.AvailableFireTypes.Find(WeaponInfo.FireType);
	if (index+1 < WeaponInfo.AvailableFireTypes.Num()) {
		WeaponInfo.FireType = WeaponInfo.AvailableFireTypes[++index];
	} else {
		WeaponInfo.FireType = WeaponInfo.AvailableFireTypes[0];
	}
}