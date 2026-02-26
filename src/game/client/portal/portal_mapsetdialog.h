#ifndef PORTAL_MAPSETDIALOG
#define PORTAL_MAPSETDIALOG
#ifdef _WIN32
#pragma once
#endif

#include "vgui_controls/Frame.h"
#include "vgui_controls/PanelListPanel.h"

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

private:

	vgui::PanelListPanel *m_pMapSetList;
	vgui::PanelListPanel *m_pMapList;
};


#endif // PORTAL_MAPSETDIALOG