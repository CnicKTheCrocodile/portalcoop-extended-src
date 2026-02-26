#ifndef PORTAL_MAPSETDIALOG
#define PORTAL_MAPSETDIALOG
#ifdef _WIN32
#pragma once
#endif

#include "vgui_controls/Frame.h"
#include "vgui_controls/PanelListPanel.h"

class CMapSetDialog : public vgui::Frame
{
public:
	DECLARE_CLASS_SIMPLE( CMapSetDialog, vgui::Frame );
	
	CMapSetDialog(Panel *parent, const char *panelName);
	~CMapSetDialog();

	virtual void	ApplySchemeSettings( vgui::IScheme *pScheme );

private:

	vgui::PanelListPanel *m_pMapSetList;
};


#endif // PORTAL_MAPSETDIALOG