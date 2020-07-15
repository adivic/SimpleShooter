// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SCharacter.generated.h"

UCLASS()
class SIMPLESHOOTER_API ASCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = Component)
	class UCameraComponent* PlayerCamera;

	UPROPERTY(EditDefaultsOnly, Category = Component)
	USkeletalMeshComponent* MeshComp;

	UPROPERTY(EditDefaultsOnly, Category = Component)
	class USpringArmComponent* SpringArm;

	UPROPERTY(BlueprintReadOnly, Category = Weapon)
	class ASWeapon* PlayerWeapon;

	UPROPERTY(EditDefaultsOnly, Category = Weapon)
	TSubclassOf<ASWeapon> WeaponClass;

	UPROPERTY(EditDefaultsOnly, Category = Weapon)
	TMap<FString, class UAnimMontage*> PlayerMontages;

	UPROPERTY(BlueprintReadOnly, Category = Player)
	bool bReloading = false;

	UPROPERTY(BlueprintReadOnly, Category = Player)
	bool bSprinting = false;

	UPROPERTY(BlueprintReadOnly, Category = Player)
	bool bAiming = false;

	UPROPERTY(EditDefaultsOnly, Category = Player)
	float AimFov = 50.f;

	float DefaultFov;

	void MoveForward(float speed);

	void MoveRight(float speed);

	void Crouch();

	UFUNCTION()
	void Fire();

	UFUNCTION()
	void StopFire();

	UFUNCTION()
	void ReloadWeapon();

	UFUNCTION()
	void Aim();

	UFUNCTION()
	void Sprint();

	float FindAndPlayMontage(FString MontageKey);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	FORCEINLINE UCameraComponent* GetPlayerCamera() const { return PlayerCamera; }

};
