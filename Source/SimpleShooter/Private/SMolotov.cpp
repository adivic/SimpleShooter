// Fill out your copyright notice in the Description page of Project Settings.


#include "SMolotov.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Kismet/KismetMathLibrary.h"

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
}

void ASMolotov::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit) {
	Explode();
	if (ExplosionEffect) {
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation(), UKismetMathLibrary::MakeRotFromX(Hit.Normal));
	}
}

void ASMolotov::Explode() {
	//Effects
	if (ExplosionSound) {
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ExplosionSound, GetActorLocation());
	}
	
	Destroy();
}
