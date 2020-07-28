// Fill out your copyright notice in the Description page of Project Settings.


#include "SFlash.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "SGrenade.h"
#include "SCharacter.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"

ASFlash::ASFlash() {
	GrenadeType = EGrenadeType::Tactical;
}

void ASFlash::BeginPlay() {
	Super::BeginPlay();
}

void ASFlash::Explode() {

	//FlashBang - https://answers.unrealengine.com/questions/347804/flash-grenade.html
	//MultiExplode();

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

				FRotator DirectionRotation = PlayerChar->GetActorRotation();
				FVector DirectionVector = DirectionRotation.Vector();
				DirectionVector.Normalize();
				FVector LookDirection = GetActorLocation() - PlayerChar->GetActorLocation();

				if (FVector::DotProduct(DirectionVector, LookDirection) > 0) {
					PlayerChar->FlashbangEffect(true);
					FTimerHandle Handle;
					FTimerDelegate FlashDelegate = FTimerDelegate::CreateUObject(PlayerChar, &ASCharacter::FlashbangEffect, false);
					GetWorldTimerManager().SetTimer(Handle, FlashDelegate, 3.f, false);
				}
			}
		}
	}
	SetLifeSpan(1.f);
}
