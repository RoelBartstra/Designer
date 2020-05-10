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

#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"

#include "DesignerEdModeToolkit.h"

#include "Toolkits/IToolkitHost.h"
#include "AssetThumbnail.h"
#include "Toolkits/BaseToolkit.h"

class Error;
class IDetailsView;
class SErrorText;
struct FPropertyAndParent;
class UDesignerSettings;

/**
 * 
 */
class DESIGNER_API SDesignerSettings : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SDesignerSettings) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, TSharedRef<FDesignerEdModeToolkit> InParentToolkit);
	
protected:
	class FDesignerEdMode* GetEditorMode() const;

	FText GetErrorText() const;

	bool GetDesignerSettingsIsEnabled() const;

	bool GetIsPropertyVisible(const FPropertyAndParent& PropertyAndParent) const;
    
protected:
	TSharedPtr<SErrorText> Error;

	TSharedPtr<IDetailsView> DetailsPanel;
};
