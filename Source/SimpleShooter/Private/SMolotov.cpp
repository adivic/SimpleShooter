// Fill out your copyright notice in the Description page of Project Settings.


#include "SMolotov.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "SCharacter.h"
#include "DrawDebugHelpers.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Volume.h"

ASMolotov::ASMolotov() {
	
	DamageRadius = 200;
	GrenadeType = EGrenadeType::Lethal;
	bActive = false;
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

	SphereCol->SetSphereRadius(DamageRadius);

	OverlappingActors.Empty();
	ActorHandles.Empty();
}

void ASMolotov::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
	if (bActive) {
		UE_LOG(LogTemp, Error, TEXT("Overlap begin"));
		OverlappingActors.Add(OtherActor);
		ActorHandles.Add(FTimerHandle());

		UGameplayStatics::ApplyDamage(OtherActor, 2, GetInstigatorController(), this, DamageType);
		FTimerDelegate Del = FTimerDelegate::CreateUObject(this, &ASMolotov::TakeFireDamage, OtherActor, 2.f);
		GetWorldTimerManager().SetTimer(ActorHandles.Last(), Del, 1.f, true);
	}
}

void ASMolotov::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) {
	UE_LOG(LogTemp, Warning, TEXT("Overlap End"));
	auto ss = OverlappingActors.Find(OtherActor);
	if (ss != INDEX_NONE) {
		OverlappingActors.RemoveAt(ss);
		GetWorldTimerManager().ClearTimer(ActorHandles[ss]);
		ActorHandles.RemoveAt(ss);
	}
	
}

void ASMolotov::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit) {
	TArray<TEnumAsByte <EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn));
	DrawDebugSphere(GetWorld(), GetActorLocation(), DamageRadius, 20, FColor::Green, false, 10);
	UKismetSystemLibrary::SphereOverlapActors(GetWorld(), GetActorLocation(), DamageRadius, ObjectTypes, nullptr, TArray<AActor*>(), OverlappingActors);
	if (OverlappingActors.Num() > 0) {
		for (short i = 0; i < OverlappingActors.Num(); ++i) {
			UE_LOG(LogTemp, Warning, TEXT("Num = %d"), OverlappingActors.Num());
			FTimerHandle Handle;
			ActorHandles.Add(Handle);
			FTimerDelegate Del = FTimerDelegate::CreateUObject(this, &ASMolotov::TakeFireDamage, OverlappingActors[i], 2.f);
			GetWorldTimerManager().SetTimer(ActorHandles[i], Del, 1.f, true);
		}
	}

	Explode();
}

void ASMolotov::TakeFireDamage(AActor* DamagedActor, float BaseDamage) {
	UGameplayStatics::ApplyDamage(DamagedActor, BaseDamage, GetInstigatorController(), this, DamageType);
}

void ASMolotov::Explode() {
	if (!bExploded) {
		bExploded = true;
		bActive = true;
		MultiExplode();
		SetLifeSpan(10.f);
		MeshComp->SetSimulatePhysics(false);
	}
}

