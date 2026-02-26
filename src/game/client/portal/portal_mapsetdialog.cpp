#include "cbase.h"
#include "portal_mapsetdialog.h"
#include "vgui_controls/Button.h"
#include "vgui_controls/ImagePanel.h"
#include "gameui/MouseMessageForwardingPanel.h"
#include "ienginevgui.h"
#include "filesystem.h"

static CMapSetDialog *g_pMapSetDialog = NULL;

using namespace vgui;

class CMapSetItemPanel : public vgui::EditablePanel
{
public:
	DECLARE_CLASS_SIMPLE( CMapSetItemPanel, EditablePanel );

	CMapSetItemPanel( PanelListPanel *parent, const char* name, KeyValues *mapset ) : BaseClass( parent, name )
	{
		m_MapSetInfo.iRequiredPlayers = mapset->GetInt("required_players");
		V_strcpy( m_MapSetInfo.szMap, mapset->GetString( "map" ) );
		
		m_pLevelPicBorder = SETUP_PANEL( new ImagePanel( this, "LevelPicBorder" ) );
		m_pLevelPic = SETUP_PANEL( new ImagePanel( this, "LevelPic" ) );
		
		m_pChapterNameLabel = new Label( this, "NameLabel", mapset->GetString( "name" ) );

		m_pParent = parent;

		SetPaintBackgroundEnabled( false );

		char szImage[32];
		V_snprintf( szImage, sizeof( szImage ), "mapsets/%s", mapset->GetName() );
		m_pLevelPic->SetImage( szImage );
		
		LoadControlSettings( "Resource/MapSetDialogItemPanel.res" );
				
		CMouseMessageForwardingPanel *panel = new CMouseMessageForwardingPanel(this, NULL);
		panel->SetZPos(2);
		
		int px, py;
		m_pLevelPicBorder->GetPos( px, py );
		SetSize( m_pLevelPicBorder->GetWide(), py + m_pLevelPicBorder->GetTall() );
		
		m_pLevelPicBorder->SetFillColor( m_FillColor );
	}
	
	MESSAGE_FUNC_INT( OnPanelSelected, "PanelSelected", state )
	{
		if ( state )
		{
			//m_pChapterLabel->SetFgColor( m_SelectedColor );
			m_pLevelPicBorder->SetFillColor( m_SelectedColor );
			m_pChapterNameLabel->SetFgColor( m_SelectedColor );
		}
		else
		{
			//m_pChapterLabel->SetFgColor( m_TextColor );
			m_pLevelPicBorder->SetFillColor( m_FillColor );
			m_pChapterNameLabel->SetFgColor( m_TextColor );
		}

		PostMessage( m_pParent->GetVParent(), new KeyValues("PanelSelected") );
	}

	virtual void OnMousePressed( vgui::MouseCode code )
	{
		m_pParent->SetSelectedPanel( this );
	}
	
	virtual void OnMouseDoublePressed( vgui::MouseCode code )
	{
		// call the panel
		OnMousePressed( code );
		PostMessage( m_pParent->GetParent(), new KeyValues("Command", "command", "play") );
	}
	
	virtual void ApplySchemeSettings( IScheme *pScheme )
	{
		m_TextColor = pScheme->GetColor( "NewGame.TextColor", Color(255, 255, 255, 255) );
		m_FillColor = pScheme->GetColor( "NewGame.FillColor", Color(255, 255, 255, 255) );
		m_DisabledColor = pScheme->GetColor( "NewGame.DisabledColor", Color(255, 255, 255, 255) );
		m_SelectedColor = pScheme->GetColor( "NewGame.SelectionColor", Color(255, 255, 255, 255) );

		BaseClass::ApplySchemeSettings( pScheme );
	}

	struct
	{
		int iRequiredPlayers;
		char szMap[MAX_MAP_NAME];
	} m_MapSetInfo;

private:
	vgui::Label *m_pChapterNameLabel;
	vgui::PanelListPanel *m_pParent;
	
	vgui::ImagePanel *m_pLevelPicBorder;
	vgui::ImagePanel *m_pLevelPic;

	Color m_TextColor;
	Color m_DisabledColor;
	Color m_SelectedColor;
	Color m_FillColor;
};

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

	m_pMapSetList->SetFirstColumnWidth( 25 );
	m_pMapSetList->SetVerticalBufferPixels( 0 );
	
	m_pMapList->SetFirstColumnWidth( 25 );
	m_pMapList->SetVerticalBufferPixels( 0 );
	
	vgui::Button *pPlayButton = dynamic_cast<vgui::Button*>( FindChildByName( "Play" ) );
	if ( pPlayButton )
	{
		pPlayButton->SetCommand( "play" );
	}
	SetupMapLists();
}

void CMapSetDialog::OnCommand( const char *command )
{
	if ( !V_stricmp( command, "play" ) )
	{
		CMapSetItemPanel *pMapPanel = (CMapSetItemPanel *)m_pMapSetList->GetSelectedPanel();
		if ( pMapPanel )
		{
			const char *pszMap = pMapPanel->m_MapSetInfo.szMap;
			int nRequiredPlayers = pMapPanel->m_MapSetInfo.iRequiredPlayers;

			char szCommand[128];
			V_snprintf( szCommand, sizeof( szCommand ), "disconnect\nwait\nwait\nmaxplayers %i\nmap %s\n", nRequiredPlayers, pszMap );
			engine->ClientCmd_Unrestricted( szCommand );
		}
		return;
	}
	BaseClass::OnCommand( command );
}

void CMapSetDialog::ParseMapSetKeyValues( KeyValues *mapsets, int &i )
{
	// add chapters to combobox
	for ( KeyValues *mapset = mapsets->GetFirstSubKey(); mapset != NULL; mapset = mapset->GetNextKey() )
	{
		CMapSetItemPanel *chapterPanel = SETUP_PANEL( new CMapSetItemPanel( m_pMapSetList, NULL, mapset ) );
		chapterPanel->SetVisible( true );
		chapterPanel->InvalidateLayout( true );

		m_pMapSetList->AddItem( NULL, chapterPanel );

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