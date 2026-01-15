// ============== Copyright (c) 2025 Monochrome Games ============== \\

#include "extdll.h"
#include "util.h"
#include "cbase.h"

#include "music_ui.h"
#include "music_manager.h"
#include "tier1/KeyValues.h"
// vgui2::VarArgs
#include "zp/ui/workshop/WorkshopItemList.h"

#include <vgui/ILocalize.h>

#include <FileSystem.h>
#include <filesystem>

// for std::vector random_shuffle
#include <iterator>
#include <random>
#include <algorithm>

CMusicManager *g_MusicManagerInstance = nullptr;

CMusicManager::CMusicManager()
{
	g_MusicManagerInstance = this;
	m_flPlayTime = -1;
	m_ItemIndex = 0;
	m_bHasStarted = false;
}

CMusicManager *CMusicManager::GetInstance()
{
	return g_MusicManagerInstance;
}

void CMusicManager::OnMapShutdown()
{
	m_ItemIndex = 0;
	StopTrack();
	m_bHasStarted = false;
}

void CMusicManager::OnMapStart()
{
	if ( m_bHasStarted ) return;
	m_flPlayTime = GetEngineTime() + 3.5f;
	// Clear the list, and add new ones.
	BuildList();
	if ( !HasPlayableMusic() ) return;
	ScrambleList();
	m_ItemIndex = -1;
	m_bHasStarted = true;
}

void CMusicManager::OnThink()
{
	if ( !m_bHasStarted ) return;
	if ( !HasPlayableMusic() ) return;
	if ( m_flPlayTime - GetEngineTime() > 0 ) return;
	PlayNextTrack();
}

void CMusicManager::StopTrack()
{
	gEngfuncs.pfnClientCmd( "mp3 stop" );
}

void CMusicManager::PlayTrack()
{
	if ( !HasPlayableMusic() ) return;
	// Play the track, and then notify the CMusicUI that a track has begun
	gEngfuncs.pfnClientCmd( vgui2::VarArgs( "mp3 play \"media/music/%s\"", GetTrackFile() ) );
	m_flPlayTime = GetEngineTime() + GetTrackTime() + 4.55f;
	CMusicUI::Get()->NewTrackPlaying();
}

void CMusicManager::PlayPrevTrack()
{
	if ( !HasPlayableMusic() ) return;
	m_ItemIndex--;
	if ( m_ItemIndex < 0 )
		m_ItemIndex = m_List.size() - 1;
	PlayTrack();
}

void CMusicManager::PlayNextTrack()
{
	if ( !HasPlayableMusic() ) return;
	m_ItemIndex++;
	if ( m_ItemIndex >= m_List.size() )
		m_ItemIndex = 0;
	PlayTrack();
}

float CMusicManager::GetEngineTime()
{
	return gEngfuncs.GetClientTime();
}

bool CMusicManager::HasPlayableMusic()
{
	return m_List.size() > 0 ? true : false;
}

void CMusicManager::BuildList()
{
	m_ItemIndex = -1;
	m_List.clear();

	FileFindHandle_t fh;
	char const *fn = g_pFullFileSystem->FindFirst( "media/music/*.txt", &fh, "GAME" );
	if ( fn )
	{
		// Get our current language
		std::string strLocalizationString = g_pVGuiLocalize->GetLocalizationFileName( 0 );
		// Take our filename, and remove everything from start to _
		// and then we remove the .txt at the end, then we get our localization name.
		// We are kinda limited on what to do without engine access,
		// so this is the best hack possibly at this very moment.
		size_t nLocalizationPos = strLocalizationString.find( '_' );
		if ( nLocalizationPos > 0 )
		{
			strLocalizationString = strLocalizationString.substr( nLocalizationPos + 1, strLocalizationString.size() );
			nLocalizationPos = strLocalizationString.find( "." );
			if ( nLocalizationPos > 0 )
				strLocalizationString = strLocalizationString.substr( 0, nLocalizationPos );
		}

		do
		{
			std::string szMP3File = fn;
			szMP3File = szMP3File.substr( 0, szMP3File.size() - 3 );
			szMP3File.append( "mp3" );

			std::string file = "media/music/" + szMP3File;
			if ( g_pFullFileSystem->FileExists( file.c_str() ) )
			{
				file = "media/music/";
				file.append( fn );
				KeyValuesAD pKV( "MusicData" );
				if ( pKV->LoadFromFile( g_pFullFileSystem, file.c_str(), "GAME" ) )
				{
					const char *szName = pKV->GetString( vgui2::VarArgs( "Name_%s", strLocalizationString ), nullptr );
					// Fallback to english
					if ( !szName ) szName = pKV->GetString( vgui2::VarArgs( "Name_english" ), nullptr );
					// Nothing found, use default text.
					if ( !szName ) szName = "Unknown Title";

					MusicItem_s item;
					Q_snprintf( item.szFile, sizeof( item.szFile ), "%s", szMP3File.c_str() );
					Q_snprintf( item.szName, sizeof( item.szName ), "%s", szName );
					item.flTrackTime = pKV->GetFloat( "TrackLength", 1.0 );
					m_List.push_back( item );
				}
			}
			fn = g_pFullFileSystem->FindNext(fh);
		} while (fn);
	}

	// We have a list, so set our index to 0
	if ( m_List.size() > 0 )
		m_ItemIndex = 0;
}

void CMusicManager::ScrambleList()
{
	std::random_device rd;
	std::mt19937 g( rd() );
	std::shuffle( m_List.begin(), m_List.end(), g );
}

const char *CMusicManager::GetTrackName()
{
	return m_List[ m_ItemIndex ].szName;
}

const char *CMusicManager::GetTrackFile()
{
	return m_List[ m_ItemIndex ].szFile;
}

float CMusicManager::GetTrackTime()
{
	return m_List[ m_ItemIndex ].flTrackTime;
}


// Client Commands
CON_COMMAND( mp3_play, "Play Track" )
{
	if ( !CMusicManager::GetInstance() ) return;
	CMusicManager::GetInstance()->PlayTrack();
}

CON_COMMAND( mp3_stop, "Stop Track" )
{
	if ( !CMusicManager::GetInstance() ) return;
	CMusicManager::GetInstance()->StopTrack();
}

CON_COMMAND( mp3_prev, "Previous Track" )
{
	if ( !CMusicManager::GetInstance() ) return;
	CMusicManager::GetInstance()->PlayPrevTrack();
}

CON_COMMAND( mp3_next, "Next Track" )
{
	if ( !CMusicManager::GetInstance() ) return;
	CMusicManager::GetInstance()->PlayNextTrack();
}
