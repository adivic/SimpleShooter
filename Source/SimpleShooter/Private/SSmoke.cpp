// Fill out your copyright notice in the Description page of Project Settings.


#include "SSmoke.h"
#include "SGrenade.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "SCharacter.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Particles/ParticleSystem.h"


ASSmoke::ASSmoke() {
	GrenadeType = EGrenadeType::Tactical;
}

void ASSmoke::BeginPlay() {
	Super::BeginPlay();
	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, this, &ASSmoke::Detonate, 3.f);
	SetLifeSpan(5);
}

void ASSmoke::Explode() {

	//MultiExplode();
	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, this, &ASSmoke::DestroySelf, 3.f);
}

void ASSmoke::DestroySelf() {
	Destroy();
	Destroy(true);
	UE_LOG(LogTemp, Error, TEXT("Destroy"));
}
