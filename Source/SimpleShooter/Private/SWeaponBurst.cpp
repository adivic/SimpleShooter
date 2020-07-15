// Fill out your copyright notice in the Description page of Project Settings.

#include "SWeaponBurst.h"

ASWeaponBurst::ASWeaponBurst() {
	//Initializing default values
	WeaponInfo.CurrentAmmo = 30;
	WeaponInfo.FullClip = 30;
	WeaponInfo.MaxAmmo = 100;
	WeaponInfo.MaxAmmoRef = 100;
	WeaponInfo.Damage = 20;
	WeaponInfo.FireType = EFireType::Burst;
	WeaponInfo.FireRate = 500;
}

void ASWeaponBurst::BeginPlay() {
	Super::BeginPlay();
}

void ASWeaponBurst::StartFire() {
	//Single fire
	bIsFiring = true;
	ASWeapon::Fire();
	bIsFiring = false;
}
