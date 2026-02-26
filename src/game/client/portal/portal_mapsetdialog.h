#ifndef PORTAL_MAPSETDIALOG
#define PORTAL_MAPSETDIALOG
#ifdef _WIN32
#pragma once
#endif

#include "vgui_controls/Frame.h"
#include "vgui_controls/PanelListPanel.h"
#include "vgui_controls/TextEntry.h"
#include "vgui_controls/CheckButton.h"

class CMapSetItemPanelMapSet;

class CMapSetDialog : public vgui::Frame
{
public:
	DECLARE_CLASS_SIMPLE( CMapSetDialog, vgui::Frame );
	
	CMapSetDialog(Panel *parent, const char *panelName);
	~CMapSetDialog();

	void			SetupMapSetList( void );
	void			SetupMapList( CMapSetItemPanelMapSet *pMapPanel );

	virtual void	ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void	OnCommand( const char *command ) OVERRIDE;

	void ParseMapSetKeyValues( KeyValues *mapsets, int &i );

	char m_szMap[32];
	int m_nRequiredPlayers;

private:

	vgui::PanelListPanel *m_pMapSetList;
	vgui::PanelListPanel *m_pMapList;
	
	vgui::TextEntry *m_pHostnameTextEntry;
	vgui::TextEntry *m_pPasswordTextEntry;
	vgui::CheckButton *m_pSteamNetworkingCheck;
};


#endif // PORTAL_MAPSETDIALOG