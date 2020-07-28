// Fill out your copyright notice in the Description page of Project Settings.


#include "SMolotov.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "SCharacter.h"
#include "DrawDebugHelpers.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Volume.h"

ASMolotov::ASMolotov() {
	
	DamageRadius = 600;
	GrenadeType = EGrenadeType::Lethal;
}

// Called when the game starts or when spawned
void ASMolotov::BeginPlay() {
	Super::BeginPlay();
	MeshComp->AddImpulse(GetActorForwardVector() * 2000, NAME_None, true);
	MeshComp->OnComponentHit.AddDynamic(this, &ASMolotov::OnHit);
	MeshComp->SetGenerateOverlapEvents(true);
	MeshComp->SetNotifyRigidBodyCollision(true);	

	SphereCol->SetNotifyRigidBodyCollision(true);
	SphereCol->OnComponentBeginOverlap.AddDynamic(this, &ASMolotov::OnOverlapBegin);
	SphereCol->OnComponentEndOverlap.AddDynamic(this, &ASMolotov::OnOverlapEnd);
}

void ASMolotov::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
	UE_LOG(LogTemp, Warning, TEXT("Overlap begin"));
	bIsOverlapp = true;
}

void ASMolotov::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) {
	UE_LOG(LogTemp, Warning, TEXT("Overlap End"));
	bIsOverlapp = false;
}

void ASMolotov::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit) {
	
	Explode();
}

void ASMolotov::Explode() {
	if (!bExploded) {
		bExploded = true;
		Explode();
		SetLifeSpan(5.f);
	}
}

