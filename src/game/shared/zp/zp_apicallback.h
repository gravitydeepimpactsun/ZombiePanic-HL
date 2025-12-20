// ============== Copyright (c) 2025 Monochrome Games ============== \\

#ifndef SHARED_ZOMBIEPANIC_API_CALLBACK
#define SHARED_ZOMBIEPANIC_API_CALLBACK
#pragma once


enum eHttpRequestTypes
{
	// Read our donator status from the API.
	READ_DONATOR_STATUS = 0,
};

enum eSupporterTier
{
	k_eSupporterTier_NONE = 0,
	k_eSupporterTier_5,
	k_eSupporterTier_10,
	k_eSupporterTier_15,
	k_eSupporterTier_20,
	k_eSupporterTier_25,
	k_eSupporterTier_50,
	k_eSupporterTier_100,
};

enum eGameAPIVersion
{
	k_eGameUnknown = -1,
	k_eGameZombiePanic = 0,
};

struct ClientAPIData_t
{
	eSupporterTier Tier = k_eSupporterTier_NONE;
	eGameAPIVersion Game = k_eGameUnknown;
	std::string Key = "";
};

#endif