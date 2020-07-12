// Fill out your copyright notice in the Description page of Project Settings.


#include "SCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"

// Sets default values
ASCharacter::ASCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	PlayerCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("PlayerCamera"));
	PlayerCamera->SetupAttachment(GetMesh());
}

// Called when the game starts or when spawned
void ASCharacter::BeginPlay()
{
	Super::BeginPlay();


	
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


// Called every frame
void ASCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	auto CameraLocation = bIsCrouched ? GetActorLocation().Z + CrouchedEyeHeight : GetActorLocation().Z + BaseEyeHeight;
	float CamPos = FMath::FInterpTo(PlayerCamera->GetRelativeLocation().Z, CameraLocation, DeltaTime, 5.f);

	PlayerCamera->SetRelativeLocation(FVector(PlayerCamera->GetRelativeLocation().X, PlayerCamera->GetRelativeLocation().Y, CamPos));
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
}

