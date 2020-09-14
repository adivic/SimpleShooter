// Fill out your copyright notice in the Description page of Project Settings.


#include "SCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/SpringArmComponent.h"
#include "SWeapon.h"
#include "SGrenade.h"
#include "Kismet/GameplayStatics.h"
#include "SHealthComponent.h"
#include "Components/PostProcessComponent.h"
#include "Net/UnrealNetwork.h"
#include "Components/CapsuleComponent.h"
#include "SWeaponDataAsset.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/DataTable.h"
#include "Materials/MaterialInstanceDynamic.h"

// Sets default values
ASCharacter::ASCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(GetMesh());

	PostProcess = CreateDefaultSubobject<UPostProcessComponent>(TEXT("PostPrcoess"));

	HealthComponent = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComponent"));

	PlayerCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("PlayerCamera"));
	PlayerCamera->SetupAttachment(SpringArm);

	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Arms"));
	MeshComp->SetupAttachment(PlayerCamera);

	DefaultFov = PlayerCamera->FieldOfView;
	GetCharacterMovement()->MaxWalkSpeed = 450.f;
	GetCharacterMovement()->MaxWalkSpeedCrouched = 200.f;

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
	
	SetReplicates(true);
}

// Called when the game starts or when spawned
void ASCharacter::BeginPlay()
{
	Super::BeginPlay();

	HealthComponent->OnHealthChanged.AddDynamic(this, &ASCharacter::OnHealthChanged);

	if (GetLocalRole() == ROLE_Authority) {
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		PlayerWeapon = GetWorld()->SpawnActor<ASWeapon>(PrimaryWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		if (PlayerWeapon) {
			FName WeaponSocket = FName("R_GunSocket");
			PlayerWeapon->SetOwner(this);
			PlayerWeapon->AttachToComponent(MeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponSocket);
		
			if (PlayerWeapon->WeaponData) {
				PlayerMontages.Empty();
				PlayerMontages = PlayerWeapon->WeaponData->PlayerMontages;
			}
		}
	}
	SpringArm->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepWorldTransform, "head");
}

void ASCharacter::MoveForward(float speed) {
	AddMovementInput(GetActorForwardVector() * speed);
}

void ASCharacter::MoveRight(float speed) {
	AddMovementInput(GetActorRightVector() * speed);
}

void ASCharacter::Crouch() {
	if (bSprinting) return;
	if (!bIsCrouched)
		ACharacter::Crouch();
	else
		ACharacter::UnCrouch();
}

void ASCharacter::Fire() {
	if (bSprinting) return;
	PlayerWeapon->StartFire();
	FireAnimations();
}

void ASCharacter::StopFire() {
	PlayerWeapon->StopFire();
}

void ASCharacter::ChangeFireMode() {
	PlayerWeapon->ChangeFireMode();
}

void ASCharacter::ReloadWeapon_Implementation() {
	if (bSprinting) return;
	if (bReloading) { 
		bReloading = false;
		return; 
	}

	if (!PlayerWeapon->IsFullClip()) {
		bReloading = true;
		if (PlayerWeapon->CanReload() && bReloading) {
			FTimerHandle TimerHandle_Reload;
			FString GunName = PlayerWeapon->GetName();
			float Delay = 2.6f; //TODO Get Delay From Animation
			if (PlayerWeapon->GetWeaponInfo().CurrentAmmo <= 0) {
				ServerFindAndPlayMontage("Reload_Empty");
				PlayerWeapon->ServerPlayMontage("ReloadEmpty");
			}
			else {
				ServerFindAndPlayMontage("Reload_NotEmpty");
				PlayerWeapon->ServerPlayMontage("Reload");
			}
			bAiming = false;
			bSprinting = false;
			FTimerHandle Handle;
			GetWorldTimerManager().SetTimer(Handle, this, &ASCharacter::ReloadWeapon, Delay);
		}
	} else {
		bReloading = false;
		ServerFindAndPlayMontage("BoltCheck");
	}
}

void ASCharacter::Aim_Implementation() {
	if (!bAiming || !bReloading || !bSprinting)
		bAiming = !bAiming;
	else
		bAiming = false;
}

void ASCharacter::Sprint_Implementation() {
	if (bIsCrouched || bAiming || bReloading) return;
	
	FVector Vel = GetVelocity();
	Vel.Normalize();
	auto DotProduct = FVector::DotProduct(GetActorForwardVector(), Vel);
	if (DotProduct <= 0) { bSprinting = false; return; }
	bSprinting = true;
	GetCharacterMovement()->MaxWalkSpeed *= 1.5;
}

void ASCharacter::SprintStop_Implementation() {
	if (!bSprinting) return;
	bSprinting = false;
	GetCharacterMovement()->MaxWalkSpeed /= 1.5;
}

void ASCharacter::ThrowGrenade_Implementation() {
	ServerFindAndPlayMontage("Throw");
	FTimerHandle Timer;
	GetWorldTimerManager().SetTimer(Timer, this, &ASCharacter::SpawnGrenade, 0.45f);
}

void ASCharacter::SpawnGrenade_Implementation() {
	if (GetLocalRole() < ROLE_Authority) return;
	FActorSpawnParameters SpawnParams;
	SpawnParams.Instigator = this;
	FVector HandLocation = MeshComp->GetSocketLocation(FName("hand_l"));
	GetWorld()->SpawnActor<ASGrenade>(GrenadeActor, HandLocation, GetControlRotation(), SpawnParams);
}

void ASCharacter::FindAndPlayMontage_Implementation(const FString& MontageKey) {
	auto MontageToPlay = PlayerMontages.Find(MontageKey);
	if (MontageToPlay) {
		USkeletalMeshComponent* SkeletalMesh = Cast<USkeletalMeshComponent>(MeshComp);
		if (SkeletalMesh) {
			 SkeletalMesh->GetAnimInstance()->Montage_Play(*MontageToPlay, 1.f, EMontagePlayReturnType::MontageLength, 0);
		}
	}
}

void ASCharacter::ServerFindAndPlayMontage(const FString& MontageKey) {
	FindAndPlayMontage(MontageKey);
}

void ASCharacter::Melee_Implementation() {
	ServerFindAndPlayMontage("Melee");
	FCollisionShape Shape;
	FHitResult Hit;
	Shape.SetCapsule(40, 60);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	DrawDebugCapsule(GetWorld(), GetActorLocation() + (GetActorForwardVector() * 100), 60, 40, FQuat::Identity, FColor::Red, false, 4.f);
	GetWorld()->SweepSingleByChannel(Hit, GetActorLocation(), GetActorLocation() + (GetActorForwardVector() * 100), FQuat::Identity, ECC_Visibility, Shape, Params);
	if (Hit.bBlockingHit) {
		const float HitDamage = 45.f;
		UGameplayStatics::ApplyDamage(Hit.GetActor(), HitDamage, GetController(), this, TSubclassOf<class UDamageType>());
	}
}

void ASCharacter::UpdateBloodyHands_Implementation(float Health) {
	if (!IsPlayerControlled()) return;
	UMaterialInstanceDynamic* MI = MeshComp->CreateDynamicMaterialInstance(0);
	if (MI) {
		MeshComp->SetMaterial(0, MI);
		MI->SetScalarParameterValue(TEXT("HealthValue"), 100 - Health);
	}
}

void ASCharacter::OnHealthChanged(USHealthComponent* HealthComp, float Health, float HealthDelta, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser) {
	
	UpdateBloodyHands(Health);

	//UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0)->StartCameraFade(0, 0.5f, .8f, FLinearColor::Red);
	
	if (Health <= 0.f && !bDead) {
		bDead = true;
		GetMovementComponent()->StopMovementImmediately();
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		DetachFromControllerPendingDestroy();

		GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
		SetActorEnableCollision(true);
		GetMesh()->SetAllBodiesSimulatePhysics(true);
		GetMesh()->SetSimulatePhysics(true);
		GetMesh()->AddImpulse(GetMesh()->GetComponentLocation() + GetMesh()->GetForwardVector() * -100);
		GetMesh()->WakeAllRigidBodies();
		GetMesh()->bBlendPhysics = true;

		if (IsPlayerControlled()) {
			GetMesh()->UnHideBoneByName("upperarm_r");
			GetMesh()->UnHideBoneByName("upperarm_l");
		}

		SetLifeSpan(10.f);
	}
}

// Called every frame
void ASCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//Crouch
	FVector DesiredLocation = bIsCrouched ? FVector(0, 0, -CrouchedEyeHeight) : FVector(0);
	PlayerCamera->SetRelativeLocation(FMath::VInterpTo(PlayerCamera->GetRelativeLocation(), DesiredLocation, DeltaTime, 5.f));
	
	//ADS
	float Fov = bAiming ? AimFov : DefaultFov;
	float NewFov = FMath::FInterpTo(PlayerCamera->FieldOfView, Fov, DeltaTime, 15.f);
	PlayerCamera->SetFieldOfView(NewFov);

	if (!IsLocallyControlled()) {
		AimPitch = PlayerCamera->GetRelativeRotation();
		AimPitch.Pitch = FMath::ClampAngle(RemoteViewPitch * 360.f / 255.0f, -90, 90);
		PlayerCamera->SetRelativeRotation(AimPitch);
	}
}

// Called to bind functionality to input
void ASCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &ASCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASCharacter::MoveRight);

	PlayerInputComponent->BindAxis("LookRight", this, &ASCharacter::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &ASCharacter::AddControllerPitchInput);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ASCharacter::Crouch);
	//PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ASCharacter::Crouch);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ASCharacter::Jump);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ASCharacter::Fire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ASCharacter::StopFire);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ASCharacter::ReloadWeapon);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ASCharacter::Sprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ASCharacter::SprintStop);

	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &ASCharacter::Aim);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &ASCharacter::Aim);

	PlayerInputComponent->BindAction("ChangeFireMode", IE_Pressed, this, &ASCharacter::ChangeFireMode);
	PlayerInputComponent->BindAction("Throw", IE_Pressed, this, &ASCharacter::ThrowGrenade);
	PlayerInputComponent->BindAction("Melee", IE_Pressed, this, &ASCharacter::Melee);
	//PlayerInputComponent->BindAction("Switch", IE_Pressed, this, &ASCharacter::SwitchWeapon);
}

void ASCharacter::FireAnimations() {
	FString Key = bAiming ? "IronShoot" : "Shoot";
	auto MontageToPlay = PlayerMontages.Find(Key + "1");
	auto MontageToPlaySecond = PlayerMontages.Find(Key + "2");
	float Delay;
	if (MontageToPlay && MontageToPlaySecond) {
		USkeletalMeshComponent* SkeletalMesh = Cast<USkeletalMeshComponent>(MeshComp);
		if (SkeletalMesh) {
			if (FMath::RandBool()) {
				Delay = SkeletalMesh->GetAnimInstance()->Montage_Play(*MontageToPlay, 1.f, EMontagePlayReturnType::MontageLength);
				if (PlayerWeapon->GetIsFiring()) {
					GetWorldTimerManager().SetTimer(FireHandle, this, &ASCharacter::FireAnimations, Delay*0.7f , true);
				}
			} else {
				Delay = SkeletalMesh->GetAnimInstance()->Montage_Play(*MontageToPlay, 1.f, EMontagePlayReturnType::MontageLength);
				if (PlayerWeapon->GetIsFiring()) {
					GetWorldTimerManager().SetTimer(FireHandle, this, &ASCharacter::FireAnimations, Delay*.7f, true);
				}
			}
		}
	}
}

void ASCharacter::FlashbangEffect_Implementation(bool IsFlashed) {
	if (IsFlashed) {
		float FlashEffect = FMath::FInterpTo(PostProcess->BlendWeight, 1, GetWorld()->GetDeltaSeconds(), 20.f);
		PostProcess->BlendWeight = 1;
	} else {
		PostProcess->BlendWeight = 0;
	}
}

void ASCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASCharacter, PlayerWeapon);
	DOREPLIFETIME(ASCharacter, bAiming);
	DOREPLIFETIME(ASCharacter, bReloading);
	DOREPLIFETIME(ASCharacter, bSprinting);
}
