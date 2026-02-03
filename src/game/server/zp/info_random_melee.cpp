// ============== Copyright (c) 2026 Monochrome Games ============== \\

#include "extdll.h"
#include "util.h"
#include "zp/info_random_base.h"

class CRandomItemMelee : public CRandomItemBase
{
public:
	ItemType GetType() const { return ItemType::TypeMelee; }
};
LINK_ENTITY_TO_CLASS( info_random_melee, CRandomItemMelee );
