// ============== Copyright (c) 2025 Monochrome Games ============== \\

#include "precache_register.h"

extern void UTIL_PrecacheOtherWeapon( const char *szClassname );

CPrecacheRegisterSystem g_PrecacheRegisterSystemInstance;

CPrecacheRegisterItem::CPrecacheRegisterItem( const char *szClassname, bool bIsWeapon )
{
	strncpy( m_szClassname, szClassname, sizeof( m_szClassname ) - 1 );
	m_szClassname[ sizeof( m_szClassname ) - 1 ] = '\0';
	m_bIsWeapon = bIsWeapon;
	CPrecacheRegisterSystem::GetInstance()->AddPrecacheItem( this );
}

CPrecacheRegisterSystem *CPrecacheRegisterSystem::GetInstance()
{
	return &g_PrecacheRegisterSystemInstance;
}

void CPrecacheRegisterSystem::PrecacheEntities()
{
	for ( CPrecacheRegisterItem *pItem : m_PrecacheItems )
	{
		if ( pItem->IsWeapon() )
		    UTIL_PrecacheOtherWeapon( pItem->GetClassname() );
		else
		    UTIL_PrecacheOther( pItem->GetClassname() );
	}
	m_PrecacheItems.clear();
}
