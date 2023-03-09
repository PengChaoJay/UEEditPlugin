// Copyright Epic Games, Inc. All Rights Reserved.

#include "SuperManager.h"

#include "AssetToolsModule.h"
#include "DebugHeader.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "EditorAssetLibrary.h"
#include "ObjectTools.h"
#include "ContentBrowserModule.h"
#include "SlateWidgets/AdvanceDeletionWidget.h"


#define LOCTEXT_NAMESPACE "FSuperManagerModule"

void FSuperManagerModule::StartupModule()
{
	InitCBMenuExtention();
	RegisterAdvanceDeletionTab();
}

void FSuperManagerModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}


#pragma region ContentBrowserMenuExtention
void FSuperManagerModule::InitCBMenuExtention()
{
	//加载模块
	FContentBrowserModule& ContentBrowserModule =
		FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	//获得content持有的所有菜单扩展的路径
	TArray<FContentBrowserMenuExtender_SelectedPaths>& ContentBrowserModuleMenuExtenders =
		ContentBrowserModule.GetAllPathViewContextMenuExtenders();

	//将创建的菜单添加进获得MenuExtenter内

	/*FContentBrowserMenuExtender_SelectedPaths CustomCBMenuDelegate;
	CustomCBMenuDelegate.BindRaw(this,&FSuperManagerModule::CustomCBMenuExtender);
	ContentBrowserModuleMenuExtenders.Add(CustomCBMenuDelegate);*/

	ContentBrowserModuleMenuExtenders.Add(
		FContentBrowserMenuExtender_SelectedPaths::CreateRaw(this, &FSuperManagerModule::CustomCBMenuExtender));
}

TSharedRef<FExtender> FSuperManagerModule::CustomCBMenuExtender(const TArray<FString>& SelectedPaths/*选择的文件夹个数*/)
{
	TSharedRef<FExtender> MenuExtender(new FExtender());

	if (SelectedPaths.Num() > 0)
	{
		MenuExtender->AddMenuExtension(
			FName("Delete"),
			EExtensionHook::After,
			TSharedPtr<FUICommandList>(),
			FMenuExtensionDelegate::CreateRaw(this, &FSuperManagerModule::AddCBMenuEntry)
		);
		FolderPathsSelected = SelectedPaths;
	}
	return MenuExtender;
}

void FSuperManagerModule::AddCBMenuEntry(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.AddMenuEntry(
		FText::FromString(TEXT("Delete Unused Assets")),
		FText::FromString(TEXT("Salfly delete  all of Unused Assets under folder")),
		FSlateIcon(),
		FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnDeleteUnsuedAssetButtonClicked)
	);
	MenuBuilder.AddMenuEntry(
		FText::FromString(TEXT("Delete empyt Floder")),
		FText::FromString(TEXT("Salfly delete all of empty folder")),
		FSlateIcon(),
		FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnDeleteEmptyFlodersButtonClicked)
	);
	MenuBuilder.AddMenuEntry(
		FText::FromString(TEXT("dvance Deletion")),
		FText::FromString(TEXT("List assets by specific condition in a tab for deleting")),
		FSlateIcon(),
		FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnAdvanceDeletionButtonClicked)
	);
}

void FSuperManagerModule::OnDeleteUnsuedAssetButtonClicked()
{
	if (FolderPathsSelected.Num() > 1)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok,TEXT("only delete a folder"));
		return;
	}
	TArray<FString> AssetsPathNames = UEditorAssetLibrary::ListAssets(FolderPathsSelected[0]);
	if (AssetsPathNames.Num() == 0)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok,TEXT("No asset found under selected folder"), false);
		return;
	}
	EAppReturnType::Type ConfirmResult = DebugHeader::ShowMsgDialog(EAppMsgType::YesNo,
	                                                                TEXT("A total of ") + FString::FromInt(
		                                                                AssetsPathNames.Num())
	                                                                + TEXT(
		                                                                " assets need to be checked.\nWould you like to procceed?"),
	                                                                false);
	if (ConfirmResult == EAppReturnType::No) return;

	FixUpRedirectors();
	TArray<FAssetData> UnusedAssetsDataArray;

	for (const FString& AssetsPathName : AssetsPathNames)
	{
		//don't touch root folder
		if (AssetsPathName.Contains(TEXT("Developers")) || AssetsPathName.Contains(TEXT("Collections"))
			|| AssetsPathName.Contains(TEXT("__ExternalActors__")) || AssetsPathName.Contains(
				TEXT("__ExternalObjects__"))
		)
		{
			continue;
		}
		if (!UEditorAssetLibrary::DoesAssetExist(AssetsPathName)) continue;
		//删除的不仅仅是文件，也包括文件的引用
		TArray<FString> AssetReferences = UEditorAssetLibrary::FindPackageReferencersForAsset(AssetsPathName);
		if (AssetReferences.Num() == 0)
		{
			const FAssetData UnsedAssetsData = UEditorAssetLibrary::FindAssetData(AssetsPathName);
			UnusedAssetsDataArray.Add(UnsedAssetsData);
		}
	}
	if (UnusedAssetsDataArray.Num() > 0)
	{
		ObjectTools::DeleteAssets(UnusedAssetsDataArray);
	}
	else
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok,TEXT("No unsedasset found under selected folder"), false);
	}
}

void FSuperManagerModule::OnDeleteEmptyFlodersButtonClicked()
{
	FixUpRedirectors();
	TArray<FString> FolderPathArray = UEditorAssetLibrary::ListAssets(FolderPathsSelected[0], true, true);
	uint32 Count = 0;
	FString EmptyFolderPathsNames;
	TArray<FString> EmptyFoldersPathsArray;
	for (const FString& FolderPath : FolderPathArray)
	{
		if (FolderPath.Contains(TEXT("Developers")) || FolderPath.Contains(TEXT("Collections"))
			|| FolderPath.Contains(TEXT("__ExternalActors__")) || FolderPath.Contains(
				TEXT("__ExternalObjects__"))
		)
		{
			continue;
		}
		if (!UEditorAssetLibrary::DoesDirectoryExist(FolderPath)) continue;

		if (!UEditorAssetLibrary::DoesDirectoryHaveAssets(FolderPath))
		{
			EmptyFolderPathsNames.Append(FolderPath);
			EmptyFolderPathsNames.Append(TEXT("\n"));

			EmptyFoldersPathsArray.Add(FolderPath);
		}
	}
	if (EmptyFoldersPathsArray.Num() == 0)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok,TEXT("No empty folder under selecte folder"), false);
		return;
	}
	EAppReturnType::Type ConfigrmResult = DebugHeader::ShowMsgDialog(EAppMsgType::OkCancel,
	                                                                 TEXT("Empty folder found in ") +
	                                                                 EmptyFolderPathsNames
	                                                                 + TEXT("\nWould you like to delete all?"), false);
	if (ConfigrmResult == EAppReturnType::Cancel) return;
	for (const FString& EmptyFolderPath : EmptyFoldersPathsArray)
	{
		UEditorAssetLibrary::DeleteDirectory(EmptyFolderPath)
			? Count++
			: DebugHeader::Print(TEXT("Failed to delete " + EmptyFolderPath), FColor::Red);
	}
	if (Count > 0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("Successfully deleted ") + FString::FromInt(Count) + TEXT(" folders"));
	}
}

void FSuperManagerModule::OnAdvanceDeletionButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(FName("AdvancedDeletion"));
}


void FSuperManagerModule::FixUpRedirectors()
{
	//创建重定向器数组
	TArray<UObjectRedirector*> RedirectorsToFixArray;

	FAssetRegistryModule& AssetRegistryMoudle = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(
		TEXT("AssetRegistry"));

	//过滤查找重定向器
	FARFilter Filter;
	Filter.bRecursivePaths = true;
	Filter.PackagePaths.Emplace("/Game");
	Filter.ClassNames.Emplace("ObjectRedirector");

	TArray<FAssetData> OutRedirectors;

	AssetRegistryMoudle.Get().GetAssets(Filter, OutRedirectors);

	//找到所有的重定向
	for (const FAssetData& RedirectorData : OutRedirectors)
	{
		if (UObjectRedirector* RedirectorToFix = Cast<UObjectRedirector>(RedirectorData.GetAsset()))
		{
			RedirectorsToFixArray.Add(RedirectorToFix);
		}
	}
	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
	AssetToolsModule.Get().FixupReferencers(RedirectorsToFixArray);
}
#pragma endregion

#pragma region CustomEditorTab

void FSuperManagerModule::RegisterAdvanceDeletionTab()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(FName("AdvancedDeletion"),
	                                                  FOnSpawnTab::CreateRaw(
		                                                  this, &FSuperManagerModule::OnSpawnAdvanceDeltionTab))
	                        .SetDisplayName(FText::FromString(TEXT("Advance Deletion")));
}


TSharedRef<SDockTab> FSuperManagerModule::OnSpawnAdvanceDeltionTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab).TabRole(ETabRole::NomadTab)[
		SNew(SAdvanceDeletionTab)
	];
}
#pragma  endregion


#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FSuperManagerModule, SuperManager)
