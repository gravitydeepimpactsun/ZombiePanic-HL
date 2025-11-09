// ============== Copyright (c) 2025 Monochrome Games ============== \\

#ifndef MUSIC_MANAGER_H
#define MUSIC_MANAGER_H

struct MusicItem_s
{
	char szName[32];
	char szFile[32];
	double dTrackTime;
};

class CMusicManager
{
public:
	CMusicManager();

	static CMusicManager *GetInstance();
	void OnMapShutdown();
	void OnMapStart();

	void OnThink();

	const char *GetTrackName();
	const char *GetTrackFile();
	double GetTrackTime();

	void StopTrack();
	void PlayTrack();
	void PlayPrevTrack();
	void PlayNextTrack();

private:
	bool HasPlayableMusic();
	void BuildList();
	void ScrambleList();

	int32 m_ItemIndex;
	std::vector<MusicItem_s> m_List;
	float m_flPlayTime;
};

#endif