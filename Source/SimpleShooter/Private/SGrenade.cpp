// Fill out your copyright notice in the Description page of Project Settings.


#include "SGrenade.h"
#include "Components/SphereComponent.h"

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

	MeshComp->SetSimulatePhysics(true);
}

// Called when the game starts or when spawned
void ASGrenade::BeginPlay()
{
	Super::BeginPlay();
	MeshComp->AddImpulse(GetActorForwardVector() * 5000, NAME_None, true);
}

void ASGrenade::Explode() {

}

// Called every frame
void ASGrenade::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASGrenade::NotifyActorBeginOverlap(AActor* OtherActor) {
	
}

