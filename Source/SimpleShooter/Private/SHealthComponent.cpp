// Fill out your copyright notice in the Description page of Project Settings.


#include "SHealthComponent.h"
#include "Net/UnrealNetwork.h"
#include "SGameMode.h"
#include "Materials/MaterialInstanceDynamic.h"

// Sets default values for this component's properties
USHealthComponent::USHealthComponent()
{
	DefaultHealth = 100;
	bIsDead = false;
}


// Called when the game starts
void USHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	
	AActor* MyOwner = GetOwner();
	if (MyOwner) {
		MyOwner->OnTakeAnyDamage.AddDynamic(this, &USHealthComponent::HandleTakeAnyDamage);
	}
	Health = DefaultHealth;
}

void USHealthComponent::OnRep_Health(float OldHealth) {
	float Damage = Health - OldHealth;
	OnHealthChanged.Broadcast(this, Health, Damage, nullptr, nullptr, nullptr);
}

void USHealthComponent::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser) {
	if (Damage <= 0 || bIsDead) return;

	if (DamageCauser == DamagedActor) return;

	Health = FMath::Clamp(Health - Damage, 0.f, DefaultHealth);

	//UE_LOG(LogTemp, Warning, TEXT("Health: %f \nDamage = %f"), Health, Damage);
	bIsDead = Health <= 0.f;

	OnHealthChanged.Broadcast(this, Health, Damage, DamageType, InstigatedBy, DamageCauser);

	if (bIsDead) {
		//UE_LOG(LogTemp, Warning, TEXT("HealthCOmp///DEAD %f"), Health);
		ASGameMode* GM = Cast<ASGameMode>(GetWorld()->GetAuthGameMode());
		if(GM) {
			GM->OnActorKilled.Broadcast(GetOwner(), DamageCauser, InstigatedBy);
		}
	}
}

float USHealthComponent::GetHealth() const {
	return Health;
}

void USHealthComponent::SetIncreaseHealt(float HealthDelta) {
	Health = DefaultHealth = HealthDelta;
}

void USHealthComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USHealthComponent, Health);
}