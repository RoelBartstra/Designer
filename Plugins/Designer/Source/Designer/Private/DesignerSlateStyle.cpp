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

FString FDesignerSlateStyle::InContent(const FString& RelativePath, const ANSICHAR* Extension)
{
	auto myself = IPluginManager::Get().FindPlugin(TEXT("Designer"));
	check(myself.IsValid());
	static FString ContentDir = myself->GetBaseDir() / TEXT("Resources");
	return (ContentDir / RelativePath) + Extension;
}

TSharedPtr<FSlateStyleSet> FDesignerSlateStyle::StyleSet = nullptr;
TSharedPtr<class ISlateStyle> FDesignerSlateStyle::Get() { return StyleSet; }

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

	StyleSet = MakeShareable(new FSlateStyleSet("DesignerSlateStyle"));

	FString ProjectResourceDir = FPaths::ProjectPluginsDir() / TEXT("Designer/Resources");
	FString EngineResourceDir = FPaths::EnginePluginsDir() / TEXT("Designer/Resources");

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
		StyleSet->Set("Designer.Icon", new FSlateImageBrush(FDesignerSlateStyle::InContent("Icon40", ".png"), IconSize40));
		StyleSet->Set("Designer.Icon.Small", new FSlateImageBrush(FDesignerSlateStyle::InContent("Icon20", ".png"), IconSize20));
		StyleSet->Set("Designer.Icon.Selected", new FSlateImageBrush(FDesignerSlateStyle::InContent("Icon40", ".png"), IconSize40));
		StyleSet->Set("Designer.Icon.Selected.Small", new FSlateImageBrush(FDesignerSlateStyle::InContent("Icon20", ".png"), IconSize20));
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
