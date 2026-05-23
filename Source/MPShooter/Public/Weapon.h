// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"


USTRUCT(BlueprintType)
struct FWeaponFirePacket
{
	GENERATED_BODY();

	UPROPERTY()
		float ServerFireTime;

	UPROPERTY()
		bool bCausedDamage;

	UPROPERTY()
		FVector_NetQuantize ImpactPoint;

	UPROPERTY()
		FVector_NetQuantizeNormal ImpactNormal;
};

UCLASS()
class MPSHOOTER_API AWeapon : public AActor
{
	GENERATED_BODY()

public:	
	// Sets default values for this actor's properties
	AWeapon(const FObjectInitializer& ObjectInitializer);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		class UStaticMeshComponent* MeshComponent;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		class USceneComponent* MuzzleHandle; //punta del canon

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aiming")
		float AimInterpSpeed; //velocidad de animacion

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aiming")
		float DropInterpSpeed; //velocidad de animacion

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aiming")
		FRotator DropRotation; //rotacion cuando no estamos apuntando

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Aiming|State")
		FVector AimLocation; //ultima ubicacion de objetivo

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Aiming|State")
		bool bAimLocationIsValid; //un objetivo es valido

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing")
		float FireCooldown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing")
		float LastFireTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing")
		class UNiagaraSystem* FireEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing")
		class UNiagaraSystem* ImpactEffect;

	UPROPERTY(ReplicatedUsing = OnRep_LastFirePacket, VisibleAnywhere, BlueprintReadOnly, Category = "Firing|State")
		FWeaponFirePacket LastFirePacket;

public:
	void UpdateAimLocation(const FVector& InWorldAimLocation, const FVector& InViewAimLocation);
	
	void HandleFireInput();

	UFUNCTION(Server, Reliable)
		void Server_TryFire(const FVector& MuzzleLocation, const FVector& Direction);

	UFUNCTION()
		void OnRep_LastFirePacket();
	
private:
	void PlayFireEffects();
	void PlayImpactEffects(const FVector& ImpactPoint, const FVector& ImpactNormal, bool bCausedDamage);

	bool RunFireTrace(FHitResult& OutHit);
};
