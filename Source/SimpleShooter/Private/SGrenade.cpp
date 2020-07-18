// Fill out your copyright notice in the Description page of Project Settings.


#include "SGrenade.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "SCharacter.h"

// Sets default values
ASGrenade::ASGrenade()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Grenade"));
	RootComponent = MeshComp;

	SphereCol = CreateDefaultSubobject<USphereComponent>(TEXT("BlastCollision"));
	SphereCol->SetupAttachment(MeshComp);

	SphereCol->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereCol->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereCol->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SphereCol->SetSphereRadius(DamageRadius);

	MeshComp->SetSimulatePhysics(true);

	GrenadeType = EGrenadeType::Lethal;
}

// Called when the game starts or when spawned
void ASGrenade::BeginPlay()
{
	Super::BeginPlay();
	MeshComp->AddImpulse(GetActorForwardVector() * 2000, NAME_None, true);

	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, this, &ASGrenade::Explode, 3.f);
}

void ASGrenade::Explode() {
	//Damage Player
	if (ExplosionSound) {
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ExplosionSound, GetActorLocation());
	}
	if (ExplosionEffect) {
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());
	}
	Destroy();
}
