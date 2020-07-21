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
	
	UPROPERTY(EditDefaultsOnly, Category = Component)
	class UPostProcessComponent* PostProcess;
	
	UPROPERTY(Replicated, BlueprintReadOnly)
	class ASWeapon* PlayerWeapon;

	UPROPERTY(EditDefaultsOnly, Category = Weapon)
	TSubclassOf<ASWeapon> WeaponClass;

	UPROPERTY(EditDefaultsOnly, Category = Weapon)
	TMap<FString, class UAnimMontage*> PlayerMontages;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = Player)
	bool bReloading = false;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = Player)
	bool bSprinting = false;

	UPROPERTY(EditDefaultsOnly, Category = Player)
	float AimFov = 50.f;

	UPROPERTY(EditDefaultsOnly, Category = Player)
	float MovementSpeed = 1.f;

	float DefaultFov;

	void MoveForward(float speed);

	void MoveRight(float speed);

	void Crouch();

	UFUNCTION()
	void Fire();

	UFUNCTION()
	void StopFire();

	UFUNCTION()
	void ChangeFireMode();

	UFUNCTION(Server, Reliable)
	void ReloadWeapon();

	UFUNCTION(Server, Reliable)
	void Aim();

	UFUNCTION(Server, Reliable)
	void Sprint();

	UFUNCTION()
	void ThrowGrenade();

	UFUNCTION(Server, Reliable)
	void SpawnGrenade();

	UPROPERTY(EditDefaultsOnly, Category = Player)
	TSubclassOf <class ASGrenade> GrenadeActor;

	UFUNCTION(NetMulticast, Reliable)
	void FindAndPlayMontage(const FString& MontageKey);

	UFUNCTION(Server, Reliable)
	void ServerFindAndPlayMontage(const FString& MontageKey);

	UFUNCTION()
	void Melee();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	FORCEINLINE UCameraComponent* GetPlayerCamera() const { return PlayerCamera; }

	UFUNCTION(Client, Reliable)
	void FlashbangEffect(bool IsFlashed = true);

	UPROPERTY(Replicated, BlueprintReadOnly, Category = Player)
	bool bAiming = false;

	FORCEINLINE void SetCharacterMovementSpeed(float Speed) { MovementSpeed = Speed; }
};
