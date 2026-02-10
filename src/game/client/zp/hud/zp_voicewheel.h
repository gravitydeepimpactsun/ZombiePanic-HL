// ============== Copyright (c) 2026 Monochrome Games ============== \\

#ifndef HUD_VOICE_WHEEL_H
#define HUD_VOICE_WHEEL_H
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/ImagePanel.h>
#include "../../hud/base.h"

class CHudVoiceWheel : public CHudElemBase<CHudVoiceWheel>, public vgui2::EditablePanel
{
public:
	DECLARE_CLASS_SIMPLE( CHudVoiceWheel, vgui2::EditablePanel );

	CHudVoiceWheel();

	void Init() override;
	void VidInit() override;
	void LoadVoiceOptions();

	virtual bool IsAllowedToDraw( const bool &bOnToggleCheck );
	virtual void Paint();
	virtual void ApplySchemeSettings( vgui2::IScheme *pScheme );

	void SetToggleState( bool bState );

protected:
	struct VoiceWheelOption_t
	{
		char text[32]; // The text to display on the wheel
		char command[32]; // The command to execute when this option is selected
	};
	std::vector<VoiceWheelOption_t> m_VoiceWheelOptions;

private:
	CPanelAnimationStringVar( 32, m_FontText, "Font", "ZPLives" );
	vgui2::HFont m_hFont;
	bool m_bActive = false;
	int m_iBackgroundTextureID = -1;
};

#endif
