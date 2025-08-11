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
class CWorkshopDialog;
class CServerBrowser;
class CImageMenuButton;

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
	CWorkshopDialog *GetWorkshopDialog();
	CServerBrowser *GetServerBrowser();

	virtual void OnThink() override;

	void GetCurrentItems( std::vector<vgui2::WorkshopItem> &items );
	void AutoMountWorkshopItem( vgui2::WorkshopItem WorkshopFile );
	void MountWorkshopItem( vgui2::WorkshopItem WorkshopFile, const char *szPath, const char *szRootPath );
	bool HasConflictingFiles( vgui2::WorkshopItem WorkshopFile );
	vgui2::WorkshopItem GetWorkshopItem( PublishedFileId_t nWorkshopID );
	void SetConflictingFiles( PublishedFileId_t nWorkshopID, bool state );
	void SetMountedState( PublishedFileId_t nWorkshopID, bool state );

	void ShowWorkshopInfoBox( const char *szText, WorkshopInfoBoxState nState );
	void SetWorkshopInfoBoxProgress( float flProgress );

	bool WorkshopIDIsMounted( PublishedFileId_t nWorkshopID );

	void OpenFileExplorer( int eFilter, const char *szFolder, const char *szPathID, DialogSelected_t pFunction );
	void OpenFileExplorer( const char *szFolder, const char *szPathID, DialogSelected_t pFunction );

	void ShowMessageDialog( const char *szTitle, const char *szDescription );

	bool IsVACBanned() const;

protected:
	void UpdateAddonList();
	void LoadWorkshop();
	bool HasLoadedItem( PublishedFileId_t nWorkshopID );
	void LoadWorkshopItems( bool bWorkshopFolder );

	// list of our sources
	std::vector<vgui2::WorkshopItem> m_Items;

	void OnSendQueryUGCRequest( SteamUGCQueryCompleted_t *pCallback, bool bIOFailure );
	CCallResult<CGameUIViewport, SteamUGCQueryCompleted_t> m_SteamCallResultOnSendQueryUGCRequest;
	UGCQueryHandle_t	handle;

	struct PrepareForDownload
	{
		PublishedFileId_t WorkshopID = 0;
		char Title[k_cchPublishedDocumentTitleMax];
		bool IsDownloading;
	};
	std::vector<PrepareForDownload> m_QueryRequests;
	PrepareForDownload m_CurrentQueryItem;
	float m_flQueryWait;
	bool m_bPrepareForQueryDownload;
	bool PrepareForQueryDownload();

private:
	bool m_bPreventEscape = false;
	int m_iDelayedPreventEscapeFrame = 0;
	vgui2::DHANDLE<CGameUITestPanel> m_hTestPanel;
	vgui2::DHANDLE<CAdvOptionsDialog> m_hOptionsDialog;
	vgui2::DHANDLE<C_AchievementDialog> m_hAchDialog;
	vgui2::DHANDLE<CWorkshopDialog> m_hWorkshopDialog;
	vgui2::DHANDLE<CCreateWorkshopInfoBox> m_hWorkshopInfoBox;
	vgui2::DHANDLE<CServerBrowser> m_hServerBrowser;
	vgui2::DHANDLE<CImageMenuButton> m_hPatreonButton;

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
	bool RequestStats();

	static inline CGameUIViewport *m_sInstance = nullptr;
};

#endif
