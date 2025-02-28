#include "APosableCharacter.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/Engine.h"  // for logging

// Sets default values
AAPosableCharacter::AAPosableCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create and attach the PoseableMeshComponent.
	posableMeshComponent_reference = CreateDefaultSubobject<UPoseableMeshComponent>(TEXT("PoseableMesh"));
	posableMeshComponent_reference->SetMobility(EComponentMobility::Movable);
	posableMeshComponent_reference->SetVisibility(true);
	posableMeshComponent_reference->SetupAttachment(RootComponent);
	RootComponent = posableMeshComponent_reference;

	// Load the default skeletal mesh asset.
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MannequinMesh(TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple"));
	if (MannequinMesh.Succeeded())
	{
		default_skeletalMesh_reference = MannequinMesh.Object;
		posableMeshComponent_reference->SetSkinnedAssetAndUpdate(default_skeletalMesh_reference);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Mannequin not found, check the path."));
	}

	// Create and configure the target sphere.
	targetSphere = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("targetSphere"));
	targetSphere->SetMobility(EComponentMobility::Movable);
	targetSphere->SetVisibility(true);
	targetSphere->SetupAttachment(RootComponent);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> sphereMesh(TEXT("/Engine/BasicShapes/Sphere"));
	if (sphereMesh.Succeeded())
	{
		targetSphereAsset = sphereMesh.Object;
		targetSphere->SetStaticMesh(targetSphereAsset);
		targetSphere->SetWorldScale3D(FVector(targetSphereScaling));

		static ConstructorHelpers::FObjectFinder<UMaterialInterface> sphereMaterial(TEXT("/Game/material/M_targetSphere"));
		if (sphereMaterial.Succeeded())
		{
			targetSphereMaterial = UMaterialInstanceDynamic::Create(sphereMaterial.Object, this, FName("sphereTarget_dynamicMaterial"));
			if (targetSphereMaterial)
			{
				targetSphere->SetMaterial(0, targetSphereMaterial);
				targetSphereMaterial->SetVectorParameterValue(TEXT("Color"), targetSphereColor);
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Sphere not found, check the path."));
	}

	// --- New: Create a spline component for the IK target scripted animation ---
	IKTargetSpline = CreateDefaultSubobject<USplineComponent>(TEXT("IKTargetSpline"));
	IKTargetSpline->SetupAttachment(RootComponent);
	// Define a simple spline path (these points are in local space)
	IKTargetSpline->ClearSplinePoints();
	IKTargetSpline->AddSplinePoint(FVector(0, 0, 0), ESplineCoordinateSpace::Local);
	IKTargetSpline->AddSplinePoint(FVector(100, 50, 0), ESplineCoordinateSpace::Local);
	IKTargetSpline->AddSplinePoint(FVector(200, 0, 50), ESplineCoordinateSpace::Local);
	IKTargetSpline->SetClosedLoop(false);
}

bool AAPosableCharacter::initializePosableMesh()
{
	if (!posableMeshComponent_reference)
	{
		UE_LOG(LogTemp, Warning, TEXT("Posable mesh component not attached or registered"));
		return false;
	}
	if (!default_skeletalMesh_reference)
	{
		UE_LOG(LogTemp, Warning, TEXT("No skeletal mesh reference provided."));
		return false;
	}

	posableMeshComponent_reference->SetSkinnedAssetAndUpdate(default_skeletalMesh_reference);
	return true;
}

void AAPosableCharacter::waving_playStop()
{
	session1_isPlaying = !session1_isPlaying;
}

void AAPosableCharacter::testSetTargetSphereRelativePosition()
{
	setTargetSphereRelativePosition(targetSphereTestRelativePosition);
}

void AAPosableCharacter::setTargetSphereRelativePosition(FVector newPosition)
{
	if (targetSphere)
	{
		targetSphere->SetRelativeLocation(newPosition);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Target sphere not found!"));
	}
}

bool AAPosableCharacter::doesBoneOrSocketNameExists(FName inputName)
{
	if (!posableMeshComponent_reference)
	{
		UE_LOG(LogTemp, Warning, TEXT("Posable mesh component not attached or registered"));
		return false;
	}
	TArray<FName> BoneNames;
	posableMeshComponent_reference->GetBoneNames(BoneNames);
	return (BoneNames.Contains(inputName) || posableMeshComponent_reference->DoesSocketExist(inputName));
}

void AAPosableCharacter::setVisibility(bool visible)
{
	if (!posableMeshComponent_reference)
	{
		UE_LOG(LogTemp, Warning, TEXT("Posable mesh component not attached or registered"));
	}
	else
	{
		posableMeshComponent_reference->SetVisibility(visible);
	}
}

void AAPosableCharacter::storeCurrentPoseRotations(TArray<FRotator>& storedPose)
{
	if (!posableMeshComponent_reference)
	{
		UE_LOG(LogTemp, Warning, TEXT("Posable mesh component not attached or registered"));
		return;
	}

	const int32 NumBones = posableMeshComponent_reference->GetNumBones();
	storedPose.SetNum(NumBones);

	for (int32 BoneIndex = 0; BoneIndex < NumBones; ++BoneIndex)
	{
		FName BoneName = posableMeshComponent_reference->GetBoneName(BoneIndex);
		if (BoneName != NAME_None)
		{
			storedPose[BoneIndex] = posableMeshComponent_reference->GetBoneRotationByName(BoneName, EBoneSpaces::ComponentSpace);
		}
	}
}

void AAPosableCharacter::waving_initializeStartingPose()
{
	if (!posableMeshComponent_reference)
	{
		UE_LOG(LogTemp, Warning, TEXT("Posable mesh component not attached or registered"));
		return;
	}

	// Example: adjust two bones for starting pose.
	FName upperArmBoneName = FName("upperarm_r");
	FName clavicleBoneName = FName("clavicle_r");

	if (doesBoneOrSocketNameExists(upperArmBoneName))
	{
		FTransform boneCompTransform = posableMeshComponent_reference->GetBoneTransformByName(upperArmBoneName, EBoneSpaces::ComponentSpace);
		FTransform parentCompTransform = posableMeshComponent_reference->GetBoneTransformByName(posableMeshComponent_reference->GetParentBone(upperArmBoneName), EBoneSpaces::ComponentSpace);
		FTransform boneRelTransform = boneCompTransform.GetRelativeTransform(parentCompTransform);
		FRotator relativeBoneRotation(21.435965f, 21.709806f, -92.235083f);
		boneRelTransform.SetRotation(relativeBoneRotation.Quaternion());
		FTransform newBoneTransformWorld = boneRelTransform * parentCompTransform;
		posableMeshComponent_reference->SetBoneRotationByName(upperArmBoneName, newBoneTransformWorld.Rotator(), EBoneSpaces::ComponentSpace);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Bone %s not found!"), *upperArmBoneName.ToString());
	}

	if (doesBoneOrSocketNameExists(clavicleBoneName))
	{
		FTransform boneCompTransform = posableMeshComponent_reference->GetBoneTransformByName(clavicleBoneName, EBoneSpaces::ComponentSpace);
		FTransform parentCompTransform = posableMeshComponent_reference->GetBoneTransformByName(posableMeshComponent_reference->GetParentBone(clavicleBoneName), EBoneSpaces::ComponentSpace);
		FTransform boneRelTransform = boneCompTransform.GetRelativeTransform(parentCompTransform);
		FRotator relativeBoneRotation(-78.486128f, 177.309228f, 13.290207f);
		boneRelTransform.SetRotation(relativeBoneRotation.Quaternion());
		FTransform newBoneTransformWorld = boneRelTransform * parentCompTransform;
		posableMeshComponent_reference->SetBoneRotationByName(clavicleBoneName, newBoneTransformWorld.Rotator(), EBoneSpaces::ComponentSpace);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Bone %s not found!"), *clavicleBoneName.ToString());
	}

	posableMeshComponent_reference->RefreshBoneTransforms();
}

void AAPosableCharacter::waving_tickAnimation()
{
	if (!posableMeshComponent_reference)
	{
		UE_LOG(LogTemp, Warning, TEXT("Posable mesh component not attached or registered"));
		return;
	}

	const float currentTime = GetWorld()->GetTimeSeconds();
	const int32 numBones = posableMeshComponent_reference->GetNumBones();
	if (waving_initialBoneRotations.Num() != numBones)
	{
		UE_LOG(LogTemp, Warning, TEXT("You need to call storeCurrentPoseRotations first!"));
		return;
	}

	// Target the head bone for the nodding animation.
	FName headBoneName = FName("head");  // Adjust if your head bone has a different name.

	if (doesBoneOrSocketNameExists(headBoneName))
	{
		// Reset the head to its stored initial rotation.
		FTransform boneCompTransform = posableMeshComponent_reference->GetBoneTransformByName(headBoneName, EBoneSpaces::ComponentSpace);
		int currentBoneIndex = posableMeshComponent_reference->GetBoneIndex(headBoneName);
		FRotator storedRotation = waving_initialBoneRotations[currentBoneIndex];
		boneCompTransform.SetRotation(storedRotation.Quaternion());

		// Retrieve the parent's transform.
		FTransform parentCompTransform = posableMeshComponent_reference->GetBoneTransformByName(
			posableMeshComponent_reference->GetParentBone(headBoneName),
			EBoneSpaces::ComponentSpace);
		FTransform boneRelTransform = boneCompTransform.GetRelativeTransform(parentCompTransform);

		// Define a cycle for the head animation.
		float cycleDuration = 4.0f;  // total cycle duration in seconds (adjust as needed)
		float t = FMath::Fmod(currentTime * waving_animationSpeed, cycleDuration);

		FRotator rotationOffset = FRotator::ZeroRotator;
		if (t < cycleDuration / 2.0f)
		{
			// First half: head nods left to right (adjust yaw).
			float phase = t / (cycleDuration / 2.0f); // Phase from 0 to 1.
			rotationOffset.Yaw = FMath::Sin(phase * PI) * waving_amplitude;
		}
		else
		{
			// Second half: head nods front to back (adjust pitch).
			float phase = (t - cycleDuration / 2.0f) / (cycleDuration / 2.0f); // Phase from 0 to 1.
			rotationOffset.Pitch = FMath::Sin(phase * PI) * waving_amplitude;
		}

		// Combine the original relative rotation with the computed offset.
		FRotator relativeBoneRotation = boneRelTransform.Rotator() + rotationOffset;
		boneRelTransform.SetRotation(relativeBoneRotation.Quaternion());

		// Reconstruct the new world transform and apply the updated rotation.
		FTransform newBoneTransformWorld = boneRelTransform * parentCompTransform;
		posableMeshComponent_reference->SetBoneRotationByName(headBoneName, newBoneTransformWorld.Rotator(), EBoneSpaces::ComponentSpace);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Bone %s not found!"), *headBoneName.ToString());
	}
}

// --- NEW: Hand IK using a simple FABRIK algorithm for a 3-bone chain (upperarm, lowerarm, hand) ---
void AAPosableCharacter::handIK_tickAnimation()
{
	// Define bone names for the arm.
	FName upperArmBoneName = FName("upperarm_r");
	FName lowerArmBoneName = FName("lowerarm_r");
	FName handBoneName = FName("hand_r");

	if (!(doesBoneOrSocketNameExists(upperArmBoneName) &&
		doesBoneOrSocketNameExists(lowerArmBoneName) &&
		doesBoneOrSocketNameExists(handBoneName)))
	{
		UE_LOG(LogTemp, Warning, TEXT("One or more arm bones not found!"));
		return;
	}

	// Get current bone transforms (positions in component space).
	FTransform upperArmTransform = posableMeshComponent_reference->GetBoneTransformByName(upperArmBoneName, EBoneSpaces::ComponentSpace);
	FTransform lowerArmTransform = posableMeshComponent_reference->GetBoneTransformByName(lowerArmBoneName, EBoneSpaces::ComponentSpace);
	FTransform handTransform = posableMeshComponent_reference->GetBoneTransformByName(handBoneName, EBoneSpaces::ComponentSpace);

	FVector p0 = upperArmTransform.GetLocation();
	FVector p1 = lowerArmTransform.GetLocation();
	FVector p2 = handTransform.GetLocation();

	// Use the target sphere’s world location as the IK target.
	FVector targetPos = targetSphere->GetComponentLocation();

	// Compute bone lengths.
	float len1 = (p1 - p0).Size();
	float len2 = (p2 - p1).Size();
	float totalLength = len1 + len2;
	float distToTarget = (targetPos - p0).Size();

	// If the target is unreachable, fully extend the arm.
	if (distToTarget >= totalLength)
	{
		FVector dir = (targetPos - p0).GetSafeNormal();
		p1 = p0 + dir * len1;
		p2 = p1 + dir * len2;
	}
	else
	{
		// Use a simple FABRIK iteration.
		const int maxIterations = 10;
		const float tolerance = 0.1f;
		FVector originalRoot = p0;

		for (int iter = 0; iter < maxIterations; iter++)
		{
			// Backward pass: set end effector to target.
			p2 = targetPos;
			FVector dir = (p1 - p2).GetSafeNormal();
			p1 = p2 + dir * len2;

			// Forward pass: fix root and update intermediate joint.
			p0 = originalRoot;
			dir = (p1 - p0).GetSafeNormal();
			p1 = p0 + dir * len1;

			// Update end effector.
			dir = (p2 - p1).GetSafeNormal();
			p2 = p1 + dir * len2;

			if ((p2 - targetPos).Size() < tolerance)
			{
				break;
			}
		}
	}

	// Compute new rotations based on the new joint positions.
	// For upper arm: aim from p0 to p1.
	FVector newDirUpper = (p1 - p0).GetSafeNormal();
	FRotator newRotUpper = FRotationMatrix::MakeFromX(newDirUpper).Rotator();

	// For lower arm: aim from p1 to p2.
	FVector newDirLower = (p2 - p1).GetSafeNormal();
	FRotator newRotLower = FRotationMatrix::MakeFromX(newDirLower).Rotator();

	// --- Advanced Feature: Joint Limits and Natural Posing ---
	// Apply joint limits on the lower arm (e.g., restrict elbow bend around the pitch axis).
	if (bEnableJointLimits)
	{
		newRotLower.Pitch = FMath::Clamp(newRotLower.Pitch, ElbowMinAngle, ElbowMaxAngle);
	}

	// Optional natural posing: smoothly blend from the stored rotation to the new one.
	// Here we use the stored rotation for the lower arm as a base.
	int lowerArmIndex = posableMeshComponent_reference->GetBoneIndex(lowerArmBoneName);
	FRotator storedLowerRotation = waving_initialBoneRotations.IsValidIndex(lowerArmIndex) ? waving_initialBoneRotations[lowerArmIndex] : FRotator::ZeroRotator;
	newRotLower = FMath::RInterpTo(storedLowerRotation, newRotLower, GetWorld()->DeltaTimeSeconds, 5.0f);

	// --- Advanced Feature: Motion Capture Integration (Dummy) ---
	// If enabled, override the computed rotations with those from the motion capture data.
	if (bUseMotionCaptureData)
	{
		if (MotionCaptureBoneRotations.Contains(upperArmBoneName))
		{
			newRotUpper = MotionCaptureBoneRotations[upperArmBoneName];
		}
		if (MotionCaptureBoneRotations.Contains(lowerArmBoneName))
		{
			newRotLower = MotionCaptureBoneRotations[lowerArmBoneName];
		}
		// Optionally, update the hand bone as well if mocap data is provided.
	}

	// Convert world rotations to relative rotations based on parent bone transforms.
	FName upperArmParent = posableMeshComponent_reference->GetParentBone(upperArmBoneName);
	FTransform parentUpper = posableMeshComponent_reference->GetBoneTransformByName(upperArmParent, EBoneSpaces::ComponentSpace);
	FTransform relTransformUpper = FTransform(newRotUpper) * parentUpper.Inverse();
	posableMeshComponent_reference->SetBoneRotationByName(upperArmBoneName, relTransformUpper.Rotator(), EBoneSpaces::ComponentSpace);

	FName lowerArmParent = upperArmBoneName; // lower arm’s parent is the upper arm.
	FTransform parentLower = posableMeshComponent_reference->GetBoneTransformByName(lowerArmParent, EBoneSpaces::ComponentSpace);
	FTransform relTransformLower = FTransform(newRotLower) * parentLower.Inverse();
	posableMeshComponent_reference->SetBoneRotationByName(lowerArmBoneName, relTransformLower.Rotator(), EBoneSpaces::ComponentSpace);

	// Optionally, set the hand’s rotation (here we reset it to zero).
	posableMeshComponent_reference->SetBoneRotationByName(handBoneName, FRotator::ZeroRotator, EBoneSpaces::ComponentSpace);

	posableMeshComponent_reference->RefreshBoneTransforms();
}

// --- NEW: Scripted animation for the IK target using a spline and ease-in/ease-out ---
void AAPosableCharacter::handIK_animateTarget(float DeltaTime)
{
	if (!IKTargetSpline || !targetSphere)
	{
		return;
	}

	handIKAnimationTime += DeltaTime;
	float t = handIKAnimationTime / handIKAnimationDuration;
	if (t > 1.0f)
	{
		t = 1.0f;
		handIKScriptedAnimationPlaying = false; // Stop animation when finished (or reset to loop).
	}

	// Ease-in/ease-out using a smoothstep-like interpolation.
	float smoothT = t * t * (3 - 2 * t);
	float splineLength = IKTargetSpline->GetSplineLength();
	float distance = smoothT * splineLength;
	FVector newTargetPosition = IKTargetSpline->GetLocationAtDistanceAlongSpline(distance, ESplineCoordinateSpace::Local);

	// Update the target sphere’s relative position.
	setTargetSphereRelativePosition(newTargetPosition);
}

void AAPosableCharacter::ToggleHandIK()
{
	handIK_isPlaying = !handIK_isPlaying;
}

void AAPosableCharacter::StartHandIKScriptedAnimation()
{
	handIKScriptedAnimationPlaying = true;
	handIKAnimationTime = 0.0f;
}

// Called when the game starts or when spawned
void AAPosableCharacter::BeginPlay()
{
	Super::BeginPlay();
	initializePosableMesh();
	waving_initializeStartingPose();
	waving_initialBoneRotations.Empty();
	storeCurrentPoseRotations(waving_initialBoneRotations);
}

// Called every frame
void AAPosableCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (session1_isPlaying)
	{
		waving_tickAnimation();
	}
	/*if (legRaise_isPlaying)
	{
		legRaise_tickAnimation();
	}*/
	if (handIK_isPlaying)
	{
		handIK_tickAnimation();
	}
	if (handIKScriptedAnimationPlaying)
	{
		handIK_animateTarget(DeltaTime);
	}
}