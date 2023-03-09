// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Modules/ModuleManager.h"

class FSuperManagerModule : public IModuleInterface
{
public:
	
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
private:
#pragma region ContentBrowserMenuExtention
	TArray<FString> FolderPathsSelected;
	void InitCBMenuExtention();
	TSharedRef<FExtender> CustomCBMenuExtender(const TArray<FString>& SelectedPaths);
	void AddCBMenuEntry(FMenuBuilder& MenuBuilder);
	void OnDeleteUnsuedAssetButtonClicked();
	void OnDeleteEmptyFlodersButtonClicked();
	void OnAdvanceDeletionButtonClicked();
	void FixUpRedirectors();
#pragma endregion

#pragma  region CustomEditorTab

	TSharedRef<SDockTab> OnSpawnAdvanceDeltionTab(const FSpawnTabArgs& SpawnTabArgs);
	void RegisterAdvanceDeletionTab();
#pragma endregion

#pragma region ProccessDataForAdvanceDeletionTab

	bool DeleteSingleAssetForAssetList(const FAssetData& AssetDataToDelete);
	bool DeleteMultipleAssetsForAssetList(const TArray<FAssetData>& AssetsToDelete);
	void ListUnusedAssetsForAssetList(const TArray< TSharedPtr <FAssetData> >& AssetsDataToFilter,TArray< TSharedPtr <FAssetData> >& OutUnusedAssetsData);
	void ListSameNameAssetsForAssetList(const TArray< TSharedPtr <FAssetData> >& AssetsDataToFilter, TArray< TSharedPtr <FAssetData> >& OutSameNameAssetsData);
	void SyncCBToClickedAssetForAssetList(const FString& AssetPathToSync);

#pragma endregion
};                                                                                                              
