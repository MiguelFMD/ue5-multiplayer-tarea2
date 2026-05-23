// Fill out your copyright notice in the Description page of Project Settings.

#include "MilestoneActor.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "ShooterPawn.h" // Incluimos tu clase de jugador
#include "NiagaraFunctionLibrary.h"

// Sets default values
AMilestoneActor::AMilestoneActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;
	
	// 1. Este actor debe ser replicable para sincronizarse con todos los clientes
	bReplicates = true;

	RootComponent = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("RootComponent"));

	// 2. Malla Estática
	MeshComponent = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshFinder(TEXT("/Script/Engine.StaticMesh'/Engine/EngineMeshes/Cube.Cube'"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder(TEXT("/Script/Engine.Material'/Game/Materials/M_Target.M_Target'"));
	
	if (MeshFinder.Succeeded()) MeshComponent->SetStaticMesh(MeshFinder.Object);
	if (MaterialFinder.Succeeded()) MeshComponent->SetMaterial(0, MaterialFinder.Object);

	// 3. Caja de Colisión
	CollisionBox = ObjectInitializer.CreateDefaultSubobject<UBoxComponent>(this, TEXT("CollisionBox"));
	CollisionBox->SetupAttachment(RootComponent);
	CollisionBox->SetBoxExtent(FVector(200.f, 200.f, 200.f));
	CollisionBox->SetCollisionProfileName(TEXT("Trigger"));

	bIsAchieved = false;
}

void AMilestoneActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (MeshComponent)
	{
		MeshMID = MeshComponent->CreateDynamicMaterialInstance(0);
	}

	// Solo el Servidor necesita escuchar los eventos de colisión
	if (HasAuthority() && CollisionBox)
	{
		CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AMilestoneActor::OnOverlapBegin);
		CollisionBox->OnComponentEndOverlap.AddDynamic(this, &AMilestoneActor::OnOverlapEnd);
	}
}

void AMilestoneActor::BeginPlay()
{
	Super::BeginPlay();
}

void AMilestoneActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	// Replicar nuestra variable de estado
	DOREPLIFETIME(AMilestoneActor, bIsAchieved);
}

void AMilestoneActor::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Si no somos el servidor o el hito ya fue conseguido, ignoramos
	if (!HasAuthority() || bIsAchieved) return;

	AShooterPawn* Pawn = Cast<AShooterPawn>(OtherActor);
	if (Pawn)
	{
		OverlappingPlayers.Add(Pawn);

		// Comprobamos si hay al menos dos jugadores
		if (OverlappingPlayers.Num() >= 2)
		{
			TriggerMilestone();
		}
	}
}

void AMilestoneActor::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!HasAuthority() || bIsAchieved) return;

	AShooterPawn* Pawn = Cast<AShooterPawn>(OtherActor);
	if (Pawn)
	{
		OverlappingPlayers.Remove(Pawn);
	}
}

void AMilestoneActor::TriggerMilestone()
{
	bIsAchieved = true;
	OnRep_IsAchieved(); // Lo llamamos manualmente en el servidor para que también vea los efectos visuales
}

void AMilestoneActor::OnRep_IsAchieved()
{
	if (bIsAchieved)
	{
		// 4. Modificar su aspecto (Ej: Color Verde brillante o Dorado)
		if (MeshMID) MeshMID->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.0f, 1.0f, 0.0f, 1.0f));

		// 5. Spawn de un efecto (Si tienes un NiagaraSystem asignado en el Blueprint/Editor)
		if (AchievementEffect) UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), AchievementEffect, GetActorLocation());
	}
}