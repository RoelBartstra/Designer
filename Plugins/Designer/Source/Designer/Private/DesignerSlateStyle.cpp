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

#include "DesignerSlateStyle.h"

#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "HAL/FileManager.h"
#include "Interfaces/IPluginManager.h"

#define IMAGE_BRUSH(RelativePath, ...) FSlateImageBrush(StyleSet->RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)

FString FDesignerSlateStyle::InContent(const FString& RelativePath, const ANSICHAR* Extension)
{
	static FString ContentDir = IPluginManager::Get().FindPlugin(TEXT("Designer"))->GetContentDir();
	return (ContentDir / RelativePath) + Extension;
}

TSharedPtr<FSlateStyleSet> FDesignerSlateStyle::StyleSet = nullptr;
TSharedPtr<class ISlateStyle> FDesignerSlateStyle::Get() { return StyleSet; }

FName FDesignerSlateStyle::GetStyleSetName()
{
	static FName DesignerStyleName(TEXT("DesignerStyle"));
	return DesignerStyleName;
}

void FDesignerSlateStyle::Initialize()
{
	// Icon sizes
	const FVector2D IconSize20(20.0f, 20.0f);
	const FVector2D IconSize40(40.0f, 40.0f);

	// Only register once
	if (StyleSet.IsValid())
	{
		return;
	}

	StyleSet = MakeShareable(new FSlateStyleSet(GetStyleSetName()));

	const FString ProjectResourceDir = FPaths::ProjectPluginsDir() / TEXT("Designer/Resources");
	const FString EngineResourceDir = FPaths::EnginePluginsDir() / TEXT("Designer/Resources");
	
	if (IFileManager::Get().DirectoryExists(*ProjectResourceDir)) //Is the plugin in the project? In that case, use those resources
	{
		StyleSet->SetContentRoot(ProjectResourceDir);
		StyleSet->SetCoreContentRoot(ProjectResourceDir);
	}
	else //Otherwise, use the global ones
	{
		StyleSet->SetContentRoot(EngineResourceDir);
		StyleSet->SetCoreContentRoot(EngineResourceDir);
	}

	{
		StyleSet->Set("Designer.Icon", new IMAGE_BRUSH("Icon40", IconSize40));
		StyleSet->Set("Designer.Icon.Small", new IMAGE_BRUSH("Icon20", IconSize20));
		StyleSet->Set("Designer.Icon.Selected", new IMAGE_BRUSH("Icon40", IconSize40));
		StyleSet->Set("Designer.Icon.Selected.Small", new IMAGE_BRUSH("Icon20", IconSize20));
	}

	FSlateStyleRegistry::RegisterSlateStyle(*StyleSet.Get());
};

void FDesignerSlateStyle::Shutdown()
{
	if (StyleSet.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*StyleSet.Get());
		ensure(StyleSet.IsUnique());
		StyleSet.Reset();
	}
}
