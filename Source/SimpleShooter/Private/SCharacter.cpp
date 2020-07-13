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
	PlayerWeapon->Reload();
}

// Called every frame
void ASCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	FVector DesiredLocation = bIsCrouched ? FVector(0, 0, -CrouchedEyeHeight) : FVector(0);
	PlayerCamera->SetRelativeLocation(FMath::VInterpTo(PlayerCamera->GetRelativeLocation(), DesiredLocation, DeltaTime, 5.f));
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
}

