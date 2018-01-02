//  Copyright 2017 Roel Bartstra.

//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files(the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions :

//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.

//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DesignerSettings.generated.h"

class FDesignerEdMode;

UENUM()
enum class EPlacementType : uint8
{
	/** Place the object fixed */
	Fixed = 0 UMETA(DisplayName = "Fixed"),

	/** Place the object using the mouse */
	Cursor = 1 UMETA(DisplayName = "Mouse"),

	/** Place the object randomly within the given min max parameters */
	Random = 2 UMETA(DisplayName = "Random")
};

UENUM()
enum class EAxisType : uint8
{
	/** None */
	None = 0 UMETA(DisplayName = "None"),

	/** The forward vector */
	Forward = 1 UMETA(DisplayName = "Forward (+X)"),

	/** The backward vector */
	Backward = 2 UMETA(DisplayName = "Backward (-X)"),

	/** The right vector */
	Right = 3 UMETA(DisplayName = "Right (+Y)"),

	/** The left vector */
	Left = 4 UMETA(DisplayName = "Left (-Y)"),

	/** The up vector */
	Up = 5 UMETA(DisplayName = "Up (+Z)"),

	/** The down vector */
	Down = 6 UMETA(DisplayName = "Down (-Z)")
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
	UPROPERTY(Category = "PlacementSettings", NonTransactional, EditAnywhere)
	FVector SpawnLocationOffsetRelative;

	/** The spawn rotation offset in world space */
	UPROPERTY(Category = "PlacementSettings", NonTransactional, EditAnywhere)
	FVector SpawnLocationOffsetWorld;

	/** Actor axis vector to align with the hit surface direction */
	UPROPERTY(Category = "PlacementSettings", NonTransactional, EditAnywhere)
	EAxisType AxisToAlignWithNormal;

	/** Actor axis vector to align with the cursor direction */
	UPROPERTY(Category = "PlacementSettings", NonTransactional, EditAnywhere)
	EAxisType AxisToAlignWithCursor;

	/** Is the rotation x axis snapped to the grid set in the viewport grid settings */
	UPROPERTY(Category = "PlacementSettings", NonTransactional, EditAnywhere)
	bool bSnapToGridRotationX;

	/** Is the rotation y axis snapped to the grid set in the viewport grid settings */
	UPROPERTY(Category = "PlacementSettings", NonTransactional, EditAnywhere)
	bool bSnapToGridRotationY;

	/** Is the rotation z axis snapped to the grid set in the viewport grid settings */
	UPROPERTY(Category = "PlacementSettings", NonTransactional, EditAnywhere)
	bool bSnapToGridRotationZ;

	/** Choose the type of placement applied to the x axis of the rotation */
	UPROPERTY(Category = "PlacementSettings", NonTransactional, EditAnywhere)
	EPlacementType RotationTypeX;

	/** If the rotation type x is defined as random these values will be used to determine the minimum and maximum value */
	UPROPERTY(Category = "PlacementSettings", NonTransactional, EditAnywhere)
	FVector2D RandomRotationMinMaxX;

	/** Choose the type of placement applied to the y axis of the rotation */
	UPROPERTY(Category = "PlacementSettings", NonTransactional, EditAnywhere)
	EPlacementType RotationTypeY;

	/** If the rotation type y is defined as random these values will be used to determine the minimum and maximum value */
	UPROPERTY(Category = "PlacementSettings", NonTransactional, EditAnywhere)
	FVector2D RandomRotationMinMaxY;

	/** Choose the type of placement applied to the Z axis of the rotation */
	UPROPERTY(Category = "PlacementSettings", NonTransactional, EditAnywhere)
	EPlacementType RotationTypeZ;

	/** If the rotation type z is defined as random these values will be used to determine the minimum and maximum value */
	UPROPERTY(Category = "PlacementSettings", NonTransactional, EditAnywhere)
	FVector2D RandomRotationMinMaxZ;

private:
	FDesignerEdMode* ParentEdMode;

public:
	void SetParent(FDesignerEdMode* DesignerEdMode)
	{
		ParentEdMode = DesignerEdMode;
	}
};
