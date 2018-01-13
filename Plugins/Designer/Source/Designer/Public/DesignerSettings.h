//  Copyright 2018 Roel Bartstra.

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

//UENUM()
//enum class EPlacementType : uint8
//{
//	/** Place the object fixed */
//	Fixed = 0 UMETA(DisplayName = "Fixed"),
//
//	/** Place the object using the mouse */
//	Cursor = 1 UMETA(DisplayName = "Mouse"),
//
//	/** Place the object randomly within the given min max parameters */
//	Random = 2 UMETA(DisplayName = "Random")
//};

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
 * The settings shown the in editor mode details panel
 */
UCLASS()
class DESIGNER_API UDesignerSettings : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	/** The spawn location offset in relative space */
	UPROPERTY(Category = "SpawnSettings", NonTransactional, EditAnywhere)
	FVector SpawnLocationOffsetRelative;

	/** The spawn rotation offset in world space */
	UPROPERTY(Category = "SpawnSettings", NonTransactional, EditAnywhere)
	FVector SpawnLocationOffsetWorld;

	/** Actor axis vector to align with the hit surface direction */
	UPROPERTY(Category = "SpawnSettings", NonTransactional, EditAnywhere)
	EAxisType AxisToAlignWithNormal;

	/** Actor axis vector to align with the cursor direction */
	UPROPERTY(Category = "SpawnSettings", NonTransactional, EditAnywhere)
	EAxisType AxisToAlignWithCursor;

	/** Is the rotation x axis snapped to the grid set in the viewport grid settings */
	UPROPERTY(Category = "SpawnSettings", NonTransactional, EditAnywhere)
	bool bSnapToGridRotationX;

	/** Is the rotation y axis snapped to the grid set in the viewport grid settings */
	UPROPERTY(Category = "SpawnSettings", NonTransactional, EditAnywhere)
	bool bSnapToGridRotationY;

	/** Is the rotation z axis snapped to the grid set in the viewport grid settings */
	UPROPERTY(Category = "SpawnSettings", NonTransactional, EditAnywhere)
	bool bSnapToGridRotationZ;
	
	/** Scale the bounds of the mesh towards the cursor location */
	UPROPERTY(Category = "SpawnSettings", NonTransactional, EditAnywhere)
	bool bScaleBoundsTowardsCursor;

private:
	FDesignerEdMode* ParentEdMode;

public:
	void SetParent(FDesignerEdMode* DesignerEdMode)
	{
		ParentEdMode = DesignerEdMode;
	}
};
