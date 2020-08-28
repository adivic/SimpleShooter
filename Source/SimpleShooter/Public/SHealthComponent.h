// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SHealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FOnHealthChangeSignature, USHealthComponent*, HealthComp, float, Health, float, HealthDelta, const class UDamageType*, DamageType, class AController*, InstigatedBy, AActor*, DamageCauser);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SIMPLESHOOTER_API USHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USHealthComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadOnly, Category = HealthComp)
	bool bIsDead;

	UPROPERTY(ReplicatedUsing=OnRep_Health, BlueprintReadOnly, Category = HealthComponent)
	float Health;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = HealthComponent)
	float DefaultHealth;

	UFUNCTION()
	void OnRep_Health(float OldHealth);

	UFUNCTION()
	void HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

public:	
	float GetHealth() const;

	UFUNCTION(BlueprintCallable, Category = Player)
	void SetIncreaseHealt(float HealthDelta);

	UPROPERTY(BlueprintCallable, Category = Events)
	FOnHealthChangeSignature OnHealthChanged;
};
