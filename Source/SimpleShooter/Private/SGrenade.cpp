// Fill out your copyright notice in the Description page of Project Settings.


#include "SGrenade.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "SCharacter.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"
#include "Net/UnrealNetwork.h"

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
	bExploded = false;

	SetReplicates(true);
	SetReplicateMovement(true);
}

// Called when the game starts or when spawned
void ASGrenade::BeginPlay()
{
	Super::BeginPlay();
	MeshComp->AddImpulse(GetActorForwardVector() * 2000, NAME_None, true);

	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, this, &ASGrenade::Detonate, 3.f);
}

void ASGrenade::OnRep_Explode() {
	FTransform SpawnTransform = FTransform(GetActorRotation(), GetActorLocation(), FVector(4));
	if (ExplosionSound) {
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ExplosionSound, GetActorLocation());
	}
	if (ExplosionEffect) {
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, SpawnTransform);
	}
	MeshComp->SetHiddenInGame(true);
}

void ASGrenade::Detonate() {
	bExploded = true;
	ServerExplode();
	OnRep_Explode();
}

void ASGrenade::ServerExplode_Implementation() {
	Explode();
}

void ASGrenade::Explode() {
	
	//FlashBang - https://answers.unrealengine.com/questions/347804/flash-grenade.html
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

void ASGrenade::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASGrenade, bExploded);
}