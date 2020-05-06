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

#include "DesignerSettings.h"
#include "DesignerEdMode.h"

UDesignerSettings::UDesignerSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, RelativeLocationOffset(FVector::ZeroVector)
	, bScaleRelativeLocationOffset(false)
	, WorldLocationOffset(FVector::ZeroVector)
	, bScaleWorldLocationOffset(false)
	, AxisToAlignWithNormal(EAxisType::Up)
	, AxisToAlignWithCursor(EAxisType::Forward)
	, SnapRotationToGrid(FBool3())
	, bApplyRandomRotation(false)
	, RandomRotation(FRandomMinMaxFloat(0.F, 360.F), FRandomMinMaxFloat(0.F, 360.F), FRandomMinMaxFloat(0.F, 360.F))
	, bScaleBoundsTowardsCursor(true)
    , MinimalScale(0.3F)
	, bApplyRandomScale(false)
	, RandomScale(FRandomMinMaxFloat(0.8F, 1.2F, true), FRandomMinMaxFloat(0.8F, 1.2F, true), FRandomMinMaxFloat(0.8F, 1.2F, true))
{
}
