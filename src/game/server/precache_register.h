// ============== Copyright (c) 2025 Monochrome Games ============== \\

#ifndef PRECACHE_REGISTER_SYSTEM_H
#define PRECACHE_REGISTER_SYSTEM_H

#if defined( CLIENT_DLL )

// Dummy macros for client, only the server needs to precache weapons/entities
#define PRECACHE_WEAPON_REGISTER( classname )
#define PRECACHE_REGISTER( classname )

#else

#define PRECACHE_WEAPON_REGISTER( classname ) static CPrecacheRegisterItem g_PrecacheRegisterItem_##classname( #classname, true );
#define PRECACHE_REGISTER( classname ) static CPrecacheRegisterItem g_PrecacheRegisterItem_##classname( #classname, false );

class CPrecacheRegisterItem
{
public:
	CPrecacheRegisterItem( const char *szClassname, bool bIsWeapon );
	bool IsWeapon() const { return m_bIsWeapon; }
	const char *GetClassname() const { return m_szClassname; }

private:
	char m_szClassname[64];
	bool m_bIsWeapon;
};

class CPrecacheRegisterSystem
{
public:
	static CPrecacheRegisterSystem *GetInstance();
	void PrecacheEntities();
	void AddPrecacheItem( CPrecacheRegisterItem *pItem );

private:
	std::vector<CPrecacheRegisterItem *> m_PrecacheItems;
};

#endif

#endif // PRECACHE_REGISTER_SYSTEM_H