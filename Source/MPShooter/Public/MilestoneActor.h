// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MilestoneActor.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class UNiagaraSystem;

UCLASS()
class MPSHOOTER_API AMilestoneActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMilestoneActor(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;

public:	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* CollisionBox;

	// Efecto opcional a reproducir cuando se alcance el hito
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	UNiagaraSystem* AchievementEffect;

protected:
	// Funciones para manejar los solapamientos de la caja
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// Variable replicada que indica si se consiguió el hito
	UPROPERTY(ReplicatedUsing = OnRep_IsAchieved)
	bool bIsAchieved;

	// Función que se ejecutará en clientes (y llamaremos manualmente en servidor) al cambiar bIsAchieved
	UFUNCTION()
	void OnRep_IsAchieved();

	void TriggerMilestone();

private:
	UPROPERTY(Transient)
	UMaterialInstanceDynamic* MeshMID;

	// Set para llevar la cuenta exacta de qué jugadores están dentro del área
	TSet<AActor*> OverlappingPlayers;
};