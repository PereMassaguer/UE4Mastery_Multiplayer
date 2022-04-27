// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSBlackHole.h"
#include "Components/SphereComponent.h"

// Sets default values
AFPSBlackHole::AFPSBlackHole()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	blackHoleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	blackHoleMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RootComponent = blackHoleMesh;

	attractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AttractionSphere"));
	attractionSphere->SetSphereRadius(3000.0f);
	attractionSphere->SetupAttachment(blackHoleMesh);
	
	destructionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DestructionSphere"));
	destructionSphere->SetSphereRadius(100.0f);
	destructionSphere->SetupAttachment(blackHoleMesh);

	destructionSphere->OnComponentBeginOverlap.AddDynamic(this, &AFPSBlackHole::OverlapInnerSphere);
}

// Called when the game starts or when spawned
void AFPSBlackHole::BeginPlay()
{
	Super::BeginPlay();
	
}

void AFPSBlackHole::OverlapInnerSphere(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		OtherActor->Destroy();
	}
}

// Called every frame
void AFPSBlackHole::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TArray<UPrimitiveComponent*> OverlappedComponents;

	attractionSphere->GetOverlappingComponents(OverlappedComponents);
	
	for (UPrimitiveComponent* component : OverlappedComponents)
	{
		if (component && component->IsSimulatingPhysics())
		{
			const float SphereRadius = attractionSphere->GetScaledSphereRadius();
			const float Force = -2000;

			component->AddRadialForce(GetActorLocation(), SphereRadius, Force, ERadialImpulseFalloff::RIF_Constant, true);

			/*FVector forceDirection = (this->GetActorLocation() - component->GetOwner()->GetActorLocation());
			forceDirection.Normalize();
		
			component->AddForce(AttractForce * forceDirection);*/
		}
	}
}