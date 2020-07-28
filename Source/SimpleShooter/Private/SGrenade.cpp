// Fill out your copyright notice in the Description page of Project Settings.


#include "SGrenade.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "SCharacter.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"
#include "Net/UnrealNetwork.h"
#include "Particles/ParticleSystemComponent.h"

// Sets default values
ASGrenade::ASGrenade()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Grenade"));
	MeshComp->SetSimulatePhysics(true);
	RootComponent = MeshComp;

	SphereCol = CreateDefaultSubobject<USphereComponent>(TEXT("BlastCollision"));
	SphereCol->SetupAttachment(MeshComp);

	SphereCol->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereCol->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereCol->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SphereCol->SetSphereRadius(DamageRadius);

	

	GrenadeType = EGrenadeType::Lethal;
	bExploded = false;

	SetReplicates(true);
	SetReplicateMovement(true);
}

// Called when the game starts or when spawned
void ASGrenade::BeginPlay()
{
	Super::BeginPlay();
	MeshComp->AddImpulse(GetActorForwardVector() * 2000, NAME_None, true);
}

/*void ASGrenade::OnRep_Explode() {
	FTransform SpawnTransform = FTransform(GetActorRotation(), GetActorLocation(), FVector(DamageRadius / 100, DamageRadius / 100, 0.5f));
	if (ExplosionSound) {
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ExplosionSound, GetActorLocation());
	}
	if (ExplosionEffect) {
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, SpawnTransform, true);
	}
	MeshComp->SetHiddenInGame(true);
}*/

void ASGrenade::MultiExplode_Implementation() {
	FTransform SpawnTransform = FTransform(GetActorRotation(), GetActorLocation(), FVector(DamageRadius / 100, DamageRadius / 100, 0.5f));
	if (ExplosionSound) {
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ExplosionSound, GetActorLocation());
	}
	if (ExplosionEffect) {
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, SpawnTransform, true);
	}
	MeshComp->SetHiddenInGame(true);
}

void ASGrenade::Detonate() {
	bExploded = true;
	Explode();
	MultiExplode();
}

void ASGrenade::Explode() {
	//Override in child class
}

void ASGrenade::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASGrenade, bExploded);
}