#include "cbase.h"
#include "portal_mapsetdialog.h"
#include "ienginevgui.h"

static CMapSetDialog *g_pMapSetDialog = NULL;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CMapSetDialog::CMapSetDialog(Panel *parent, const char *panelName) : Frame(parent, panelName)
{
	SetDeleteSelfOnClose(true);
	SetSizeable( false );
	m_pMapSetList = NULL;
	
	LoadControlSettings( "Resource/MapSetDialog.res" );
}

CMapSetDialog::~CMapSetDialog()
{
	g_pMapSetDialog = NULL;
}

void CMapSetDialog::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );
}

static void CC_OpenMapSetDialog(const CCommand &args)
{
	vgui::Panel *parent = (vgui::Panel *)NULL;

	if (!g_pMapSetDialog)
		g_pMapSetDialog = SETUP_PANEL( new CMapSetDialog(parent, "MapSetDialog"));
	
	if (!g_pMapSetDialog)
		return;

	vgui::VPANEL GameUiDll = enginevgui->GetPanel(PANEL_GAMEUIDLL);
	//SetTitle("#GameUI_Options", true);
	g_pMapSetDialog->MoveToCenterOfScreen();
	g_pMapSetDialog->Activate();
	g_pMapSetDialog->SetParent(GameUiDll);
	
}

static ConCommand OpenMapSetDialog( "OpenMapSetDialog", CC_OpenMapSetDialog, "Open map set dialog" );