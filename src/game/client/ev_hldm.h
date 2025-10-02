//========= Copyright (c) 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================

#if !defined(EV_HLDMH)
#define EV_HLDMH

// bullet types
typedef enum
{
	BULLET_NONE = 0,
	BULLET_PLAYER_9MM, // glock
	BULLET_PLAYER_MP5, // mp5
	BULLET_PLAYER_357, // python
	BULLET_PLAYER_BUCKSHOT, // shotgun
	BULLET_PLAYER_CROWBAR, // crowbar swipe
	BULLET_PLAYER_SWIPE, // zombie swipe

	BULLET_MONSTER_9MM,
	BULLET_MONSTER_MP5,
	BULLET_MONSTER_12MM,
} Bullet;

enum sig_e
{
	SIG_IDLE = 0,
	SIG_IDLE_EMPTY,
	SIG_SHOOT,
	SIG_SHOOT_EMPTY,
	SIG_RELOAD,
	SIG_RELOAD_EMPTY,
	SIG_DRAW,
	SIG_HOLSTER,
	SIG_DRAW_EMPTY,
	SIG_HOLSTER_EMPTY,
};

enum glock_e
{
	GLOCK_IDLE1 = 0,
	GLOCK_IDLE2,
	GLOCK_IDLE3,
	GLOCK_SHOOT,
	GLOCK_SHOOT_EMPTY,
	GLOCK_RELOAD,
	GLOCK_RELOAD_NOT_EMPTY,
	GLOCK_DRAW,
	GLOCK_HOLSTER,
	GLOCK_ADD_SILENCER
};

enum shotgun_e
{
	SHOTGUN_IDLE = 0,
	SHOTGUN_IDLE2,
	SHOTGUN_FIRE,
	SHOTGUN_PUMP,
	SHOTGUN_RELOAD_START,
	SHOTGUN_RELOAD,
	SHOTGUN_RELOAD_END,
	SHOTGUN_DRAW
};

enum mp5_e
{
	MP5_LONGIDLE = 0,
	MP5_IDLE1,
	MP5_RELOAD,
	MP5_DEPLOY,
	MP5_FIRE1,
	MP5_FIRE2,
	MP5_FIRE3,
};

enum mp16_e
{
	AR556_LONGIDLE = 0,
	AR556_IDLE1,
	AR556_LAUNCH,
	AR556_RELOAD,
	AR556_DEPLOY,
	AR556_FIRE1,
	AR556_FIRE2,
	AR556_FIRE3,
	AR556_HOLSTER
};

enum python_e
{
	PYTHON_IDLE1 = 0,
	PYTHON_FIDGET,
	PYTHON_FIRE1,
	PYTHON_RELOAD,
	PYTHON_HOLSTER,
	PYTHON_DRAW,
	PYTHON_IDLE2,
	PYTHON_IDLE3
};

#define GAUSS_PRIMARY_CHARGE_VOLUME 256 // how loud gauss is while charging
#define GAUSS_PRIMARY_FIRE_VOLUME   450 // how loud gauss is when discharged

enum gauss_e
{
	GAUSS_IDLE = 0,
	GAUSS_IDLE2,
	GAUSS_FIDGET,
	GAUSS_SPINUP,
	GAUSS_SPIN,
	GAUSS_FIRE,
	GAUSS_FIRE2,
	GAUSS_HOLSTER,
	GAUSS_DRAW
};

void EV_HLDM_GunshotDecalTrace(pmtrace_t *pTrace, char *decalName);
void EV_HLDM_DecalGunshot(pmtrace_t *pTrace, const Vector &vDir, int iBulletType);
int EV_HLDM_CheckTracer(int idx, float *vecSrc, float *end, float *forward, float *right, int iBulletType, int iTracerFreq, int *tracerCount);
void EV_HLDM_FireBullets(int idx, float *forward, float *right, float *up, int cShots, float *vecSrc, float *vecDirShooting, float flDistance, int iBulletType, int iTracerFreq, int *tracerCount, float flSpreadX, float flSpreadY);

#endif // EV_HLDMH
