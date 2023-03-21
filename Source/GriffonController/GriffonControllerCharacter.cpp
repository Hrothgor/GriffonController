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
		FlyPhysicsCompute(DeltaSeconds);
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

	FRotator rotation = GetActorRotation();
	SetActorRotation(FRotator(0, rotation.Yaw, 0));

	GetCharacterMovement()->Velocity /= 3; // Slow when landing
}

void AGriffonControllerCharacter::StopFlapping()
{
	bIsFlapping = false;
}

void AGriffonControllerCharacter::FlyPhysicsCompute(float DeltaSeconds)
{
	FVector ActorVelocity = GetVelocity();
	FRotator ControlRotation = GetControlRotation();
	FRotator ActorRotation = GetActorRotation();

	// INCLINATION // 22:00
	// Get how much the camera is aiming to the actor forward in Z
	ControlInclination = FVector::DotProduct(UKismetMathLibrary::GetUpVector(ControlRotation),
											UKismetMathLibrary::GetForwardVector(FRotator(0, ActorRotation.Yaw, 0))); // 1 parallel, 0 perpendicular, -1 parallel opposite
	
	// CALCULATE LIFT // 16:00
	// Get how much the camera is aiming to the actor forward in Z
	FVector VelocityCurveLiftTime = ActorVelocity; // 19:00
	VelocityCurveLiftTime.Z = UKismetMathLibrary::FClamp(VelocityCurveLiftTime.Z, -4000, 0); // only take negative value

	float ControlInclinationAngle = UKismetMathLibrary::DegAcos(ControlInclination) - 90;
	GEngine->AddOnScreenDebugMessage(INDEX_NONE, 0, FColor::Red, FString::Printf(TEXT("ControlInclinationAngle: %f"), ControlInclinationAngle));

	
	LiftNormalized =	FlightVelocityAngleMultiplierCurve->GetFloatValue(ControlInclinationAngle) *
						FlightVelocityLiftMultiplierCurve->GetFloatValue(VelocityCurveLiftTime.Length());

	int32 roundedLiftNormalized = round(LiftNormalized);
	
	bCanFly = roundedLiftNormalized == 0 ? false : true;

	//if (BackwardActionIsPressed) // 25:00
	//else

	// GLIDE // 28:00
	// Add default Movement Input in the glide direction to glide
	// and it allow to go faster when going down and slower when going up
	float ScaleGlidingSpeed = UKismetMathLibrary::MapRangeClamped(ActorVelocity.Z,	-500, 0,
																					1.5, 0);
	FlySpeedGliding = FMath::FInterpTo(FlySpeedGliding, ScaleGlidingSpeed, DeltaSeconds,
																			abs(ControlInclination) + 0.5);
	
	FVector GlidingDirection = UKismetMathLibrary::GetForwardVector(FRotator(0, ControlRotation.Yaw, 0)); // looking direction
	GlidingDirection.Normalize();
	AddMovementInput(GlidingDirection, FlySpeedGliding);
	
	// ADD LIFT FORCE //
	// What make us glide better when faster
	float LiftForce =	GetCharacterMovement()->Mass *
						abs(GetWorld()->GetGravityZ()) *
						LiftNormalized;

	GetCharacterMovement()->AddForce({0, 0, LiftForce});

	// VELOCITY // 39:00
	// If can fly we add velocity UP/DOWN on Z (pretty much the flying system)
	if (bCanFly)
	{
		
		FVector ActualCharacterVelocity = GetCharacterMovement()->Velocity;

		float TargetVelocityZ = ControlInclination *
								GetWorld()->GetGravityZ() *
								abs(ControlInclination) *
								10;
		
		float NextVelocityZ = FMath::FInterpTo(ActualCharacterVelocity.Z, TargetVelocityZ, DeltaSeconds, 4);

		FVector NewVelocityXY = FMath::VInterpTo(ActualCharacterVelocity, GlidingDirection * ActualCharacterVelocity.Length(), DeltaSeconds, 3);
		
		GetCharacterMovement()->Velocity = FVector(	NewVelocityXY.X,
													NewVelocityXY.Y,
													NextVelocityZ);
	}

	// ROTATION // 42:00
	// Prevent drifting in the air
	FRotator NewRotation;
	NewRotation.Pitch = GetCharacterMovement()->Velocity.ToOrientationRotator().Pitch;

	FVector VelocityForwardVector(GetCharacterMovement()->Velocity.X, GetCharacterMovement()->Velocity.Y, 0);
	VelocityForwardVector.Normalize();

	float TurnInclination = FVector::DotProduct(UKismetMathLibrary::GetRightVector(GetCharacterMovement()->Velocity.ToOrientationRotator()),
												UKismetMathLibrary::GetForwardVector(FRotator(0, ActorRotation.Yaw, 0)));

	float TurnInclinationAngle = (UKismetMathLibrary::DegAcos(TurnInclination) - 90) * 3; // 3 constant value, just the roll feel way better, it's too small else
	TurnInclinationAngle = UKismetMathLibrary::FClamp(TurnInclinationAngle, -110, 110);
	
	NewRotation.Roll = TurnInclinationAngle;
	
	FVector ActorRotationRightVector = UKismetMathLibrary::GetRightVector(ActorRotation);
	NewRotation.Yaw = UKismetMathLibrary::MakeRotationFromAxes(VelocityForwardVector, ActorRotationRightVector, FVector::UpVector).Yaw;

	SetActorRotation(FMath::RInterpTo(ActorRotation, NewRotation, DeltaSeconds, 3));

	//
	if (!GetCharacterMovement()->IsFalling())
		StopFlying();
}

void AGriffonControllerCharacter::DrawDebug()
{
	if (IsDebug == true && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(INDEX_NONE, 0, FColor::Yellow, FString::Printf(TEXT("FlySpeedGliding: %f"), FlySpeedGliding));
		GEngine->AddOnScreenDebugMessage(INDEX_NONE, 0, FColor::Blue, FString::Printf(TEXT("Velocity: %f"), GetCharacterMovement()->Velocity.Length()));
		GEngine->AddOnScreenDebugMessage(INDEX_NONE, 0, FColor::Red, FString::Printf(TEXT("ControlInclination: %f"), ControlInclination));
	}
}





