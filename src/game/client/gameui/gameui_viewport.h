#ifndef GAMEUI_VIEWPORT_H
#define GAMEUI_VIEWPORT_H
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/MessageBox.h>
#include "steam/steam_api.h"
#include "zp/ui/workshop/WorkshopItemList.h"
#include "filedialog/IFileDialogManager.h"
#include "CCreateWorkshopInfoBox.h"

class CGameUITestPanel;
class CAdvOptionsDialog;
class C_AchievementDialog;
class C_PlayerSelection;
class C_ZPCredits;
class CWorkshopDialog;
class CServerBrowser;
class CImageMenuButton;
class CBaseMenu;

enum GameUIDialogs
{
	UIDialog_Achievements = 0,
	UIDialog_Options,
	UIDialog_PlayerSelection,
	UIDialog_Workshop,
	UIDialog_Credits,
	UIDialog_ServerBrowser,

	UIDialog_MAX
};

class CGameUIViewport : public vgui2::EditablePanel
{
	DECLARE_CLASS_SIMPLE(CGameUIViewport, vgui2::EditablePanel);

public:
	static inline CGameUIViewport *Get()
	{
		return m_sInstance;
	}

	CGameUIViewport();
	~CGameUIViewport();

	/**
	 * If enabled, prevents ESC key from showing GameUI.
	 * In reality, hides GameUI whenever it is enabled.
	 */
	void PreventEscapeToShow(bool state);

	void OpenTestPanel();
	CAdvOptionsDialog *GetOptionsDialog();
	C_AchievementDialog *GetAchievementDialog();
	C_PlayerSelection *GetPlayerSelection();
	C_ZPCredits *GetCredits();
	CWorkshopDialog *GetWorkshopDialog();
	CServerBrowser *GetServerBrowser();
	CBaseMenu *GetMenu();

	vgui2::Panel *GetDialog( GameUIDialogs nDialog );

	virtual void OnThink() override;

	void GetCurrentItems( std::vector<vgui2::WorkshopItem> &items );
	void AutoMountWorkshopItem( vgui2::WorkshopItem &WorkshopFile );
	void MountWorkshopItem( vgui2::WorkshopItem WorkshopFile, const char *szPath, const char *szRootPath );
	bool HasConflictingFiles( vgui2::WorkshopItem WorkshopFile );
	vgui2::WorkshopItem GetWorkshopItem( PublishedFileId_t nWorkshopID );
	void SetConflictingFiles( PublishedFileId_t nWorkshopID, bool state );
	void SetMountedState( PublishedFileId_t nWorkshopID, bool state );

	WorkshopInfoBoxState GetWorkshopInfoBoxState();
	bool IsWorkshopInfoBoxVisible();
	void ShowWorkshopInfoBox( const char *szText, WorkshopInfoBoxState nState );
	void SetWorkshopInfoBoxProgress( float flProgress );
	void AddExtraWorkshopInfoBoxTime( float flTime, float flMaxTime );

	bool WorkshopIDIsMounted( PublishedFileId_t nWorkshopID );
	bool ShouldAutoMount( PublishedFileId_t nWorkshopID );

	void OpenFileExplorer( int eFilter, const char *szFolder, const char *szPathID, DialogSelected_t pFunction );
	void OpenFileExplorer( const char *szFolder, const char *szPathID, DialogSelected_t pFunction );

	void ShowMessageDialog( const char *szTitle, const char *szDescription );

	bool IsVACBanned() const;

	void DownloadWorkshopAddon( PublishedFileId_t nWorkshopID, const bool &bReconnect );

protected:
	void UpdateWorkshopMapsFile( const bool &bWorkshopFolder );
	void LoadWorkshop();
	void CheckWorkshopSubscriptions();
	void RemoveWorkshopItem( const int &nID );
	bool HasSubscribedToItem( PublishedFileId_t nWorkshopID );
	bool HasLoadedItem( PublishedFileId_t nWorkshopID );
	void LoadWorkshopItems( bool bWorkshopFolder );

	// Rebuilds the addonlist.txt
	void RebuiltAddonList();

	// Our subscribed items. If we subscribe to a new item, we want to mount it right away!
	std::vector<PublishedFileId_t> m_SubscribedItems;

	// list of our sources
	std::vector<vgui2::WorkshopItem> m_Items;

	void OnSendQueryUGCRequest( SteamUGCQueryCompleted_t *pCallback, bool bIOFailure );
	CCallResult<CGameUIViewport, SteamUGCQueryCompleted_t> m_SteamCallResultOnSendQueryUGCRequest;
	UGCQueryHandle_t	handle;

	CCallResult<CGameUIViewport, HTTPRequestCompleted_t> m_callHTTPResult;
	void UpdateHTTPCallback( HTTPRequestCompleted_t *arg, bool bFailed );

	struct PrepareForDownload
	{
		PublishedFileId_t WorkshopID = 0;
		char Title[k_cchPublishedDocumentTitleMax];
		bool IsDownloading;
		bool Reconnect;
	};
	std::vector<PrepareForDownload> m_QueryRequests;
	PrepareForDownload m_CurrentQueryItem;
	bool m_bNeedToReconnectAfterDownload;
	bool m_bDownloadedItemsReady;			// Called by OnDownloadItemResult
	float m_flQueryWait;
	void SetQueryWait( const float &flTime );
	bool m_bPrepareForQueryDownload;
	bool PrepareForQueryDownload();

private:
	bool m_bPreventEscape = false;
	int m_iDelayedPreventEscapeFrame = 0;
	vgui2::DHANDLE<CGameUITestPanel> m_hTestPanel;
	vgui2::DHANDLE<CAdvOptionsDialog> m_hOptionsDialog;
	vgui2::DHANDLE<C_AchievementDialog> m_hAchDialog;
	vgui2::DHANDLE<C_PlayerSelection> m_hPlayerSelection;
	vgui2::DHANDLE<C_ZPCredits> m_hCredits;
	vgui2::DHANDLE<CWorkshopDialog> m_hWorkshopDialog;
	vgui2::DHANDLE<CCreateWorkshopInfoBox> m_hWorkshopInfoBox;
	vgui2::DHANDLE<CServerBrowser> m_hServerBrowser;
	vgui2::DHANDLE<CBaseMenu> m_hMenu;

	template <typename T>
	inline T *GetDialog(vgui2::DHANDLE<T> &handle)
	{
		if (!handle.Get())
		{
			handle = new T(this);
		}

		return handle;
	}

	// Grab our stats on creation.
	STEAM_CALLBACK( CGameUIViewport, OnUserStatsReceived, UserStatsReceived_t, m_CallbackUserStatsReceived );
	STEAM_CALLBACK( CGameUIViewport, OnDownloadItemResult, DownloadItemResult_t, m_SteamCallbackResultOnDownloadItemResult );
	bool RequestStats();

	static inline CGameUIViewport *m_sInstance = nullptr;
};

#endif
