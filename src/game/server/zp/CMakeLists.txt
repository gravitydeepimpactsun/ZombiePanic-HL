# Bot Support - WIP
#add_subdirectory( bot )

add_sources(
	CMakeLists.txt
	info_objective.h
	info_objective.cpp
	multi_relay.cpp
	info_beacon.cpp
	info_beacon.h
	prop_pushable.cpp
	prop_barricade.cpp
	trigger_spawnblocker.cpp
	trigger_endround.cpp
	info_random_base.cpp
	info_random_base.h
	info_random_item.cpp
	info_random_ammo.cpp
	info_random_weapon.cpp
	zp_spawnpoint_ent.cpp
	zp_spawnpoint_ent.h
	zp_gamerules.cpp
	zp_gamerules.h
	gamemodes/zp_gamemodebase.cpp
	gamemodes/zp_survival.cpp
	gamemodes/zp_survival.h
	gamemodes/zp_hardcore.cpp
	gamemodes/zp_hardcore.h
	gamemodes/zp_objective.cpp
	gamemodes/zp_objective.h
	gamemodes/zp_dev.cpp
	gamemodes/zp_dev.h
)
