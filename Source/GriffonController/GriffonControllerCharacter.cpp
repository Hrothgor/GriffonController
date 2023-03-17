// Copyright Epic Games, Inc. All Rights Reserved.

#include "GriffonControllerCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Chaos/Utilities.h"
#include "Kismet/KismetMathLibrary.h"


//////////////////////////////////////////////////////////////////////////
// AGriffonControllerCharacter

AGriffonControllerCharacter::AGriffonControllerCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)


	// Set the default values
	DefaultAirControlValue = GetCharacterMovement()->AirControl;
	DefaultBrakingFrictionValue = GetCharacterMovement()->BrakingFriction;
	DefaultRotationRateValue = GetCharacterMovement()->RotationRate;
	DefaultMaxAccelerationValue = GetCharacterMovement()->MaxAcceleration;
	DefaultMaxWalkSpeedValue = GetCharacterMovement()->MaxWalkSpeed;
}

void AGriffonControllerCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void AGriffonControllerCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AGriffonControllerCharacter::Move);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &AGriffonControllerCharacter::StopFlapping);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AGriffonControllerCharacter::Look);

		//Fly
		EnhancedInputComponent->BindAction(FlyAction, ETriggerEvent::Started, this, &AGriffonControllerCharacter::StartFlying);
	}

}

void AGriffonControllerCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);

		if (MovementVector.Y > 0)
			bIsFlapping = true;
	}
}

void AGriffonControllerCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AGriffonControllerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bIsFlying)
	{
		FlyPhysicsCompute();
	}
	
	DrawDebug();
}

void AGriffonControllerCharacter::StartFlying()
{
	if (GetCharacterMovement()->IsFalling())
	{
		if (!bIsFlying)
		{
			bIsFlying = true;
			
			GetCharacterMovement()->AirControl = AirControlValue;
			GetCharacterMovement()->BrakingFriction = BrakingFrictionValue;
			GetCharacterMovement()->RotationRate = RotationRateValue;
			GetCharacterMovement()->MaxAcceleration = MaxAccelerationValue;
			GetCharacterMovement()->MaxWalkSpeed = MaxWalkSpeedValue;
		} else
		{
			StopFlying();
		}
	}
}

void AGriffonControllerCharacter::StopFlying()
{
	bIsFlying = false;

	GetCharacterMovement()->AirControl = DefaultAirControlValue;
	GetCharacterMovement()->BrakingFriction = DefaultBrakingFrictionValue;
	GetCharacterMovement()->RotationRate = DefaultRotationRateValue;
	GetCharacterMovement()->MaxAcceleration = DefaultMaxAccelerationValue;
	GetCharacterMovement()->MaxWalkSpeed = DefaultMaxWalkSpeedValue;
}

void AGriffonControllerCharacter::StopFlapping()
{
	bIsFlapping = false;
}

void AGriffonControllerCharacter::FlyPhysicsCompute()
{
	FVector ActorVelocity = GetVelocity();
	FVector ActorVelocityNormalized = GetVelocity();
	ActorVelocityNormalized.Normalize();
	FRotator ControlRotation = GetControlRotation();
	FRotator ActorRotation = GetActorRotation();

	// INCLINATION // 22:00
	// Get how much the camera is aiming to the actor forward
	ControlInclination = FVector::DotProduct(UKismetMathLibrary::GetUpVector(ControlRotation),
											UKismetMathLibrary::GetForwardVector(ActorRotation)); // 1 parallel, 0 perpendicular, -1 parallel opposite

	// CALCULATE LIFT // 16:00
	// Get how much the camera is aiming to the actor forward
	FVector VelocityCurveLiftTime = ActorVelocity; // 19:00 check clamp -4000 Z axis
	VelocityCurveLiftTime.Z = UKismetMathLibrary::FClamp(VelocityCurveLiftTime.Z, -4000, 0);

	float ControlInclinationAngle = acos(ControlInclination) - 90;
	
	LiftNormalized =	FlightAngleLiftMultiplierCurve->GetFloatValue(ControlInclinationAngle) *
						FlightVelocityLiftMultiplierCurve->GetFloatValue(VelocityCurveLiftTime.Length());

	int32 roundedLiftNormalized = round(LiftNormalized);
	
	bCanFly = roundedLiftNormalized == 0 ? false : true;

	//if (BackwardActionIsPressed) // 25:00
	//else

	// GLIDE // 28:00
	// Add default Movement Input in the glide direction to glide longer
	// and it allow to go faster when going down and slower when going up
	float ScaleGlidingSpeed = UKismetMathLibrary::MapRangeClamped(ActorVelocity.Z,	-500, 0,
																						1.5, 0); // To get faster when going down and slower going up

	FlySpeedGliding = FMath::FInterpTo(FlySpeedGliding, ScaleGlidingSpeed, GetWorld()->GetDeltaSeconds(),
																			abs(ControlInclination) + 0.05);
	
	FVector GlidingDirection = UKismetMathLibrary::GetForwardVector(FRotator(0, ControlRotation.Yaw, 0)); // looking direction
	AddMovementInput(GlidingDirection, FlySpeedGliding);
	
	// ADD LIFT FORCE //
	// What make us fall down
	float LiftForce =	GetCharacterMovement()->Mass *
						abs(GetWorld()->GetGravityZ()) *
						LiftNormalized;

	GetCharacterMovement()->AddForce({0, 0, LiftForce});

	// VELOCITY // 39:00
	// If can fly we add velocity (pretty much the flying system)
	if (bCanFly)
	{
		FVector ActualCharacterVelocity = GetCharacterMovement()->Velocity;

		float TargetVelocityZ = ControlInclination *
								GetWorld()->GetGravityZ() *
								abs(ControlInclination) *
								10;
		
		float NextVelocityZ = FMath::FInterpTo(ActualCharacterVelocity.Z, TargetVelocityZ, GetWorld()->GetDeltaSeconds(), 4);
		float ClampedNextVelocityZ = UKismetMathLibrary::FClamp(NextVelocityZ, -4000, 2000);
		
		GetCharacterMovement()->Velocity = FVector(	ActualCharacterVelocity.X,
													ActualCharacterVelocity.Y,
													ClampedNextVelocityZ);
	}
	// Calculate Velocity Angle
	float DotVelocityDownVector = FVector::DotProduct(ActorVelocityNormalized,
													UKismetMathLibrary::GetUpVector(ActorRotation) * -1);
	VelocityAngle = acos(DotVelocityDownVector) - 90;

	FlyingVelocity = ActorVelocity.Length();

	// ROTATION // 42:00
	// Prevent drifting in the air
	FRotator NewRotation;
	NewRotation.Pitch = GetCharacterMovement()->Velocity.ToOrientationRotator().Pitch;
	NewRotation.Roll = ActorRotation.Roll;

	FVector VelocityForwardVector(GetCharacterMovement()->Velocity.X, GetCharacterMovement()->Velocity.Y, 0);
	FVector ActorRotationRightVector = UKismetMathLibrary::GetRightVector(ActorRotation);
	NewRotation.Yaw = UKismetMathLibrary::MakeRotationFromAxes(VelocityForwardVector, ActorRotationRightVector, FVector::UpVector).Yaw;
	
	SetActorRotation(FMath::RInterpTo(ActorRotation, NewRotation, GetWorld()->GetDeltaSeconds(), 3));

	//
	if (!GetCharacterMovement()->IsFalling())
		StopFlying();
}

void AGriffonControllerCharacter::DrawDebug()
{
	if (IsDebug == true && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("FlySpeedGliding: %f"), FlySpeedGliding));
	}
}





