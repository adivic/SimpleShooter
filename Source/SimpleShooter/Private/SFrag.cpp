// Fill out your copyright notice in the Description page of Project Settings.


#include "SFrag.h"
#include "SGrenade.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "SCharacter.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"


ASFrag::ASFrag() {
	GrenadeType = EGrenadeType::Lethal;
}

void ASFrag::BeginPlay() {
	Super::BeginPlay();
	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, this, &ASGrenade::Detonate, 3.f);
}

void ASFrag::Explode() {
	
	OnRep_Explode();

	TArray<TEnumAsByte <EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_PhysicsBody));
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn));

	TArray<AActor*> OverlappingActors;
	UKismetSystemLibrary::SphereOverlapActors(GetWorld(), GetActorLocation(), DamageRadius, ObjectTypes, nullptr, TArray<AActor*>(), OverlappingActors);
	DrawDebugSphere(GetWorld(), GetActorLocation(), DamageRadius, 10, FColor::Red, false, 4);
	if (OverlappingActors.Num() > 0) {
		for (AActor* PlayerIn : OverlappingActors) {
			ASCharacter* PlayerChar = Cast<ASCharacter>(PlayerIn);
			if (PlayerChar) {
				const float BaseDamage = 100.f;
				const float MinimalDamage = 20.f;
				const float DamageFallout = 10.f;
				UGameplayStatics::ApplyRadialDamageWithFalloff(GetWorld(), BaseDamage, MinimalDamage, GetActorLocation(), DamageRadius * .7f, 
					DamageRadius, DamageFallout, TSubclassOf<class UDamageType>(), TArray<AActor*>(), GetInstigator(), GetInstigatorController(), ECC_WorldStatic);
			}
		}
	}
	SetLifeSpan(1.f);

}
