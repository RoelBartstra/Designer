/**
 * MIT License
 * 
 * Copyright(c) 2018 RoelBartstra
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files(the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions :
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DesignerSettings.generated.h"

class FDesignerEdMode;

UENUM()
enum class EAxisType : uint8
{
	/** None */
	None = 0x00 UMETA(DisplayName = "None"),

	/** The forward vector */
	Forward = 0x02 UMETA(DisplayName = "Forward (+X)"),

	/** The backward vector */
	Backward = 0x03 UMETA(DisplayName = "Backward (-X)"),

	/** The right vector */
	Right = 0x04 UMETA(DisplayName = "Right (+Y)"),

	/** The left vector */
	Left = 0x05 UMETA(DisplayName = "Left (-Y)"),

	/** The up vector */
	Up = 0x08 UMETA(DisplayName = "Up (+Z)"),

	/** The down vector */
	Down = 0x09 UMETA(DisplayName = "Down (-Z)")
};

/**
 * A random float within a min max range
 * Option for randomly negating the value
 */
USTRUCT(BlueprintType)
struct FRandomMinMaxFloat
{
	GENERATED_BODY()

	/** The minimal value */
	UPROPERTY(Category = "Random", EditAnywhere)
	float Min;

	/** The maximum value */
	UPROPERTY(Category = "Random", EditAnywhere)
	float Max;

	/**
	 * Is this value allowed to randomly flip the sign of the generated value?
	 * i.e. if Min = 30 and Max = 30 the outcome can be either 30 or -30 when this is set to true
	 */
	UPROPERTY(Category = "Random", EditAnywhere)
	bool bRandomlyNegate;

private:
	/** The randomly generated value */
	UPROPERTY()
	float RandomValue;

public:
	FRandomMinMaxFloat()
	{
		this->Min = 0.F;
		this->Max = 1.F;
		this->bRandomlyNegate = false;

		RegenerateRandomValue();
	}

	FRandomMinMaxFloat(float Min, float Max, bool bRandomlyNegateValue = false)
	{
		this->Min = Min;
		this->Max = Max;
		this->bRandomlyNegate = bRandomlyNegateValue;

		RegenerateRandomValue();
	}

	/** Get the random value currently stored in this struct */
	FORCEINLINE float GetCurrentRandomValue() { return RandomValue; }

	/** Regenerates the random value and returns it. The value can also be retrieved later as well using GetCurrentRandomValue */
	FORCEINLINE float RegenerateRandomValue() { return RandomValue = FMath::RandRange(Min, Max) * (FMath::RandBool() && bRandomlyNegate ? -1.F : 1.F); }
};

/**
 * A random vector within a min max range per component
 * Option for randomly negating the value per component
 */
USTRUCT(BlueprintType)
struct FRandomMinMaxVector
{
	GENERATED_BODY()

	/** X value */
	UPROPERTY(Category = "Random", EditAnywhere)
	FRandomMinMaxFloat X;

	/** Y value */
	UPROPERTY(Category = "Random", EditAnywhere)
	FRandomMinMaxFloat Y;

	/** Z value */
	UPROPERTY(Category = "Random", EditAnywhere)
	FRandomMinMaxFloat Z;

public:
	FRandomMinMaxVector()
	{
		this->X = FRandomMinMaxFloat();
		this->Y = FRandomMinMaxFloat();
		this->Z = FRandomMinMaxFloat();

		RegenerateRandomValue();
	}

	FRandomMinMaxVector(FRandomMinMaxFloat X, FRandomMinMaxFloat Y, FRandomMinMaxFloat Z)
	{
		this->X = FRandomMinMaxFloat(X);
		this->Y = FRandomMinMaxFloat(Y);
		this->Z = FRandomMinMaxFloat(Z);

		RegenerateRandomValue();
	}

	/** Get the random value currently stored in this struct */
	FORCEINLINE FVector GetCurrentRandomValue() { return FVector(X.GetCurrentRandomValue(), Y.GetCurrentRandomValue(), Z.GetCurrentRandomValue()); }

	/** Regenerates the random value and returns it. The value can also be retrieved later as well using GetCurrentRandomValue */
	FORCEINLINE void RegenerateRandomValue()
	{
		X.RegenerateRandomValue();
		Y.RegenerateRandomValue();
		Z.RegenerateRandomValue();
	}
};

/**
 * XYZ booleans.
 */
USTRUCT(BlueprintType)
struct FBool3
{
	GENERATED_BODY()

	/** X value */
	UPROPERTY(Category = "Bool3", EditAnywhere)
	bool X;

	/** Y value */
	UPROPERTY(Category = "Bool3", EditAnywhere)
	bool Y;

	/** Z value */
	UPROPERTY(Category = "Bool3", EditAnywhere)
	bool Z;

public:
	FBool3()
	{
		this->X = false;
		this->Y = false;
		this->Z = false;
	}

	FBool3(bool X, bool Y, bool Z)
	{
		this->X = X;
		this->Y = Y;
		this->Z = Z;
	}
};

/**
 * The settings shown the in editor mode details panel
 */
UCLASS()
class DESIGNER_API UDesignerSettings : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	/** The spawn location offset in relative space */
	UPROPERTY(Category = "SpawnSettings", EditAnywhere)
	FVector RelativeLocationOffset;

	/** Scale the relative location offset according to the scale of the mesh. A scale of (1,1,1) = the exact offset filled in. */
	UPROPERTY(Category = "SpawnSettings", EditAnywhere)
	bool bScaleRelativeLocationOffset;

	/** The spawn rotation offset in world space */
	UPROPERTY(Category = "SpawnSettings", EditAnywhere)
	FVector WorldLocationOffset;

	/** Scale the world location offset according to the scale of the mesh. A scale of (1,1,1) = the exact offset filled in. */
	UPROPERTY(Category = "SpawnSettings", EditAnywhere)
	bool bScaleWorldLocationOffset;

	/** Actor axis vector to align with the hit surface direction */
	UPROPERTY(Category = "SpawnSettings", EditAnywhere)
	EAxisType AxisToAlignWithNormal;

	/** Actor axis vector to align with the cursor direction */
	UPROPERTY(Category = "SpawnSettings", EditAnywhere)
	EAxisType AxisToAlignWithCursor;

	/** Is the rotation x axis snapped to the grid set in the viewport grid settings */
	UPROPERTY(Category = "SpawnSettings", EditAnywhere)
	FBool3 SnapRotationToGrid;
	
	/** Randomly rotates the mesh */
	UPROPERTY(Category = "SpawnSettings", EditAnywhere)
	bool bApplyRandomRotation;
	
	/** Random rotation offset applied to the x axis rotation matrix on spawn */
	UPROPERTY(Category = "SpawnSettings", EditAnywhere, meta = (EditCondition = "bApplyRandomRotation"))
	FRandomMinMaxVector RandomRotation;

	/** Scale the bounds of the mesh towards the cursor location */
	UPROPERTY(Category = "SpawnSettings", EditAnywhere)
	bool bScaleBoundsTowardsCursor;

	/** The minimal scale which is applied to the mesh when spawning */
	UPROPERTY(Category = "SpawnSettings", EditAnywhere, meta = (UIMin = "0.1", UIMax = "1.0", ClampMin = "0.01", ClampMax = "100.0"))
	float MinimalScale;
	
	/** Randomly scale the mesh */
	UPROPERTY(Category = "SpawnSettings", EditAnywhere)
	bool bApplyRandomScale;

	/** Random scale for x axis */
	UPROPERTY(Category = "SpawnSettings", EditAnywhere, meta = (EditCondition = "bApplyRandomScale"))
	FRandomMinMaxVector RandomScale;

	/**
	 * Always returns the positive axis of the current selected AxisToAlignWithCursor
	  * i.e. Backward becomes Forward while Up stays Up.
	*/
	FORCEINLINE EAxisType GetPositiveAxisToAlignWithCursor() { return (EAxisType)(~1 & (int)AxisToAlignWithCursor); }

private:
	FDesignerEdMode* ParentEdMode;

public:
	void SetParent(FDesignerEdMode* DesignerEdMode)
	{
		ParentEdMode = DesignerEdMode;
	}
};
