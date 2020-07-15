// Fill out your copyright notice in the Description page of Project Settings.


#include "SCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/SpringArmComponent.h"
#include "SWeapon.h"

// Sets default values
ASCharacter::ASCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(GetMesh());


	PlayerCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("PlayerCamera"));
	PlayerCamera->SetupAttachment(SpringArm);

	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Arms"));
	MeshComp->SetupAttachment(PlayerCamera);

	DefaultFov = PlayerCamera->FieldOfView;
	GetCharacterMovement()->MaxWalkSpeed = 450.f;
	GetCharacterMovement()->MaxWalkSpeedCrouched = 200.f;
}

// Called when the game starts or when spawned
void ASCharacter::BeginPlay()
{
	Super::BeginPlay();

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	PlayerWeapon = GetWorld()->SpawnActor<ASWeapon>(WeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	if (PlayerWeapon) {
		FName WeaponSocket = FName("R_GunSocket");
		PlayerWeapon->SetOwner(this);
		PlayerWeapon->AttachToComponent(MeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponSocket);
	}
}

void ASCharacter::MoveForward(float speed) {
	AddMovementInput(GetActorForwardVector() * speed);
}

void ASCharacter::MoveRight(float speed) {
	AddMovementInput(GetActorRightVector() * speed);
}

void ASCharacter::Crouch() {
	ACharacter::Crouch();
	
	if (bIsCrouched) {
		UnCrouch();	
	}
}

void ASCharacter::Fire() {
	PlayerWeapon->StartFire();
}

void ASCharacter::StopFire() {
	PlayerWeapon->StopFire();
}

void ASCharacter::ReloadWeapon() {
	bReloading = !bReloading;
	if (bReloading) {
		FTimerHandle TimerHandle_Reload; 
		float Delay;
		if(PlayerWeapon->GetWeaponInfo().CurrentAmmo <= 0)
			Delay = FindAndPlayMontage("Reload_Empty");
		else 
			Delay = FindAndPlayMontage("Reload_NotEmpty");
		PlayerWeapon->Reload();
		GetWorldTimerManager().SetTimer(TimerHandle_Reload, this, &ASCharacter::ReloadWeapon, Delay);
	}
}

void ASCharacter::Aim() {
	bAiming = !bAiming;
}

void ASCharacter::Sprint() {
	bSprinting = !bSprinting;

	if (bSprinting) {
		GetCharacterMovement()->MaxWalkSpeed *= 1.5;
	} else {
		GetCharacterMovement()->MaxWalkSpeed /= 1.5;
	}
}

float ASCharacter::FindAndPlayMontage(FString MontageKey) {
	auto MontageToPlay = PlayerMontages.Find(MontageKey);
	if (MontageToPlay) {
		return MeshComp->GetAnimInstance()->Montage_Play(*MontageToPlay, 1.f, EMontagePlayReturnType::MontageLength, 0, false);
	}
	return 0;
}

// Called every frame
void ASCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//Crouch
	FVector DesiredLocation = bIsCrouched ? FVector(0, 0, -CrouchedEyeHeight*2) : FVector(0);
	PlayerCamera->SetRelativeLocation(FMath::VInterpTo(PlayerCamera->GetRelativeLocation(), DesiredLocation, DeltaTime, 5.f));
	
	//ADS
	float Fov = bAiming ? AimFov : DefaultFov;
	float NewFov = FMath::FInterpTo(PlayerCamera->FieldOfView, Fov, DeltaTime, 15.f);
	PlayerCamera->SetFieldOfView(NewFov);

	if (PlayerWeapon->GetIsFiring()) {
		FString Montage1, Montage2;
		if (bAiming) {
			Montage1 = "ShootISight_1";
			Montage2 = "ShootISight_2";
		} else {
			Montage1 = "Shoot_1";
			Montage2 = "Shoot_2";
		}
		FindAndPlayMontage(Montage1);
		FindAndPlayMontage(Montage2);
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
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ASCharacter::Crouch);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ASCharacter::Jump);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ASCharacter::Fire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ASCharacter::StopFire);

	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ASCharacter::ReloadWeapon);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ASCharacter::Sprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ASCharacter::Sprint);

	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &ASCharacter::Aim);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &ASCharacter::Aim);
}

