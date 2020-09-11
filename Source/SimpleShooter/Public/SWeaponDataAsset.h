// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SWeaponDataAsset.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class SIMPLESHOOTER_API USWeaponDataAsset : public UDataAsset
{
	GENERATED_BODY()
	

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<FString, class UAnimMontage*> PlayerMontages;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<FString, class UAnimMontage*> GunMontages;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<class ASWeapon> WeaponClass;

};

/*
	Player Montages should have names: Reload_Empty, Reload_NotEmpty, Melee, Throw, BoltCheck, Shoot1, Shoot2, IronShoot1, IronShoot2
	GunMontages should have names: Reload, ReloadEmpty, Fire;
*/