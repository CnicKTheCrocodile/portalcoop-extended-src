#include "cbase.h"
#include "portal_mapsetdialog.h"
#include "vgui_controls/Button.h"
#include "vgui_controls/ImagePanel.h"
#include "ienginevgui.h"
#include "filesystem.h"

static CMapSetDialog *g_pMapSetDialog = NULL;

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CMapSetDialog::CMapSetDialog(Panel *parent, const char *panelName) : Frame(parent, panelName)
{
	SetDeleteSelfOnClose(true);
	SetSizeable( false );
	m_pMapSetList = NULL;
	m_pMapList = NULL;
	
	LoadControlSettings( "Resource/MapSetDialog.res" );
}

CMapSetDialog::~CMapSetDialog()
{
	g_pMapSetDialog = NULL;
}

void CMapSetDialog::SetupMapLists( void )
{
	int i = 0;
	KeyValues *mapsets = new KeyValues( "mapsets" );
	mapsets->LoadFromFile( g_pFullFileSystem, "scripts/mapsets/mapsets_official.txt" );
	// Parse the official mapsets first
	ParseMapSetKeyValues( mapsets, i );
	mapsets->deleteThis();
	// Now parse custom ones
	if ( true )
	{
		const char* pCurrentPath = "scripts/mapsets/";
	
		char szDirectory[_MAX_PATH];
		Q_snprintf( szDirectory, sizeof( szDirectory ), "%s*", pCurrentPath );

		FileFindHandle_t dirHandle;
		const char *pDirFileName = g_pFullFileSystem->FindFirst( szDirectory, &dirHandle );

		while (pDirFileName)
		{
			// Skip it if it's not a directory, is the root, is back, or is an invalid folder
			if ( !g_pFullFileSystem->FindIsDirectory( dirHandle ) || 
				 Q_strcmp( pDirFileName, "." ) == 0 || 
				 Q_strcmp( pDirFileName, ".." ) == 0 )
			{
				pDirFileName = g_pFullFileSystem->FindNext( dirHandle );
				continue;
			}

			mapsets = new KeyValues( "mapsets" );

			char szFullDirectory[_MAX_PATH];
			Q_snprintf( szFullDirectory, sizeof( szFullDirectory ), "scripts/mapsets/%s/mapsets.txt", pDirFileName );
			mapsets->LoadFromFile( g_pFullFileSystem, szFullDirectory );
			ParseMapSetKeyValues( mapsets, i );
			mapsets->deleteThis();

			pDirFileName = g_pFullFileSystem->FindNext( dirHandle );
		}

		g_pFullFileSystem->FindClose( dirHandle );
	}
}

void CMapSetDialog::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	m_pMapSetList = dynamic_cast<vgui::PanelListPanel*>( FindChildByName( "MapSetList" ) );
	m_pMapList = dynamic_cast<vgui::PanelListPanel*>( FindChildByName( "MapList" ) );

	SetupMapLists();
}

void CMapSetDialog::ParseMapSetKeyValues( KeyValues *mapsets, int &i )
{
	return; // Disabled until the map set list has proper panels to add
	// add chapters to combobox
	for ( KeyValues *mapset = mapsets->GetFirstSubKey(); mapset != NULL; mapset = mapset->GetNextKey() )
	{
		char chapterName[64];
		Q_snprintf(chapterName, sizeof(chapterName), mapset->GetString( "name" ) );

		int required_players = mapset->GetInt( "required_players" );
		const char *map = mapset->GetString( "map" );
		//CMapSetItemPanel *chapterPanel = SETUP_PANEL( new CMapSetItemPanel( this, NULL, chapterName, i, mapset->GetName(), required_players, map ) );
		//chapterPanel->SetVisible( true );
		//chapterPanel->InvalidateLayout( true );

		//m_pMapSetList->AddItem( NULL, chapterPanel );

		++i;
	}
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