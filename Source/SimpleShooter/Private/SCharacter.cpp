// Fill out your copyright notice in the Description page of Project Settings.


#include "SCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/SpringArmComponent.h"
#include "SWeapon.h"
#include "SGrenade.h"
#include "Components/PostProcessComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ASCharacter::ASCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(GetMesh());

	PostProcess = CreateDefaultSubobject<UPostProcessComponent>(TEXT("PostPrcoess"));

	PlayerCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("PlayerCamera"));
	PlayerCamera->SetupAttachment(SpringArm);

	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Arms"));
	MeshComp->SetupAttachment(PlayerCamera);

	DefaultFov = PlayerCamera->FieldOfView;
	GetCharacterMovement()->MaxWalkSpeed = 450.f;
	GetCharacterMovement()->MaxWalkSpeedCrouched = 200.f;

	SetReplicates(true);
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
	SpringArm->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepWorldTransform, "neck_01");
}

void ASCharacter::MoveForward(float speed) {
	AddMovementInput(GetActorForwardVector() * speed* MovementSpeed);
}

void ASCharacter::MoveRight(float speed) {
	AddMovementInput(GetActorRightVector() * speed * MovementSpeed);
}

void ASCharacter::Crouch() {
	if (!bSprinting) {
		ACharacter::Crouch();

		if (bIsCrouched) {
			UnCrouch();
		}
	}
}

void ASCharacter::Fire() {
	PlayerWeapon->StartFire();
}

void ASCharacter::StopFire() {
	PlayerWeapon->StopFire();
}

void ASCharacter::ChangeFireMode() {
	PlayerWeapon->ChangeFireMode();
}

void ASCharacter::ReloadWeapon_Implementation() {
	if (bReloading) { 
		bReloading = false;
		return; 
	}

	if (!PlayerWeapon->IsFullClip()) {
		bReloading = true;
		if (PlayerWeapon->CanReload() && bReloading) {
			FTimerHandle TimerHandle_Reload;
			float Delay = 2.5f; //Get Delay From Animation
			if (PlayerWeapon->GetWeaponInfo().CurrentAmmo <= 0)
				ServerFindAndPlayMontage("Reload_Empty");
			else
				ServerFindAndPlayMontage("Reload_NotEmpty");
			PlayerWeapon->Reload();
			FTimerHandle Handle;
			GetWorldTimerManager().SetTimer(Handle, this, &ASCharacter::ReloadWeapon, Delay);
		}
	} else {
		bReloading = false;
		ServerFindAndPlayMontage("BoltCheck");
	}
}

void ASCharacter::Aim_Implementation() {
	if (!bAiming && !bReloading && !bSprinting)
		bAiming = !bAiming;
	else
		bAiming = false;
}

void ASCharacter::Sprint_Implementation() {
	if (!bIsCrouched && !bAiming && !bReloading) {
		bSprinting = !bSprinting;

		if (bSprinting) {
			GetCharacterMovement()->MaxWalkSpeed *= 1.5;
		} else {
			GetCharacterMovement()->MaxWalkSpeed /= 1.5;
		}
	}
}

void ASCharacter::ThrowGrenade() {
	ServerFindAndPlayMontage("Throwing");
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

void ASCharacter::ServerFindAndPlayMontage_Implementation(const FString& MontageKey) {
	FindAndPlayMontage(MontageKey);
}

void ASCharacter::Melee() {
	ServerFindAndPlayMontage("Melee"); // 
	FCollisionShape Shape;
	FHitResult Hit;
	Shape.SetCapsule(40, 60);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	DrawDebugCapsule(GetWorld(), GetActorLocation() + (GetActorForwardVector() * 100), 60, 40, FQuat::Identity, FColor::Red, false, 4.f);
	GetWorld()->SweepSingleByChannel(Hit, GetActorLocation(), GetActorLocation() + (GetActorForwardVector() * 100), FQuat::Identity, ECC_Visibility, Shape, Params);
	if (Hit.bBlockingHit) {
		//Damage other player
		UE_LOG(LogTemp, Warning, TEXT("Melee Hit"));
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
		FRotator NewRot = PlayerCamera->GetRelativeRotation();
		NewRot.Pitch = RemoteViewPitch * 360.f / 255.0f;
		PlayerCamera->SetRelativeRotation(NewRot);
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

	PlayerInputComponent->BindAction("ChangeFireMode", IE_Pressed, this, &ASCharacter::ChangeFireMode);
	PlayerInputComponent->BindAction("Throw", IE_Pressed, this, &ASCharacter::ThrowGrenade);
	PlayerInputComponent->BindAction("Melee", IE_Pressed, this, &ASCharacter::Melee);
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
