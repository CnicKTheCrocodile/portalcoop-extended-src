#include "cbase.h"
#include "portal_mapsetdialog.h"
#include "vgui_controls/Button.h"
#include "vgui_controls/ImagePanel.h"
#include "gameui/MouseMessageForwardingPanel.h"
#include "ienginevgui.h"
#include "filesystem.h"
#include "portal_shareddefs.h"

static CMapSetDialog *g_pMapSetDialog = NULL;

using namespace vgui;

class CMapSetItemPanel : public vgui::EditablePanel
{
public:
	DECLARE_CLASS_SIMPLE( CMapSetItemPanel, EditablePanel );

	CMapSetItemPanel( PanelListPanel *parent, const char *name, const char *titlename, const char *imagename ) : BaseClass( parent, name )
	{
		m_pLevelPicBorder = SETUP_PANEL( new ImagePanel( this, "LevelPicBorder" ) );
		m_pLevelPic = SETUP_PANEL( new ImagePanel( this, "LevelPic" ) );
		
		m_pChapterNameLabel = new Label( this, "NameLabel", titlename );

		m_pParent = parent;

		SetPaintBackgroundEnabled( false );

		m_pLevelPic->SetImage( imagename );
		
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

protected:
	vgui::Label *m_pChapterNameLabel;
	vgui::PanelListPanel *m_pParent;
	
	vgui::ImagePanel *m_pLevelPicBorder;
	vgui::ImagePanel *m_pLevelPic;

	Color m_TextColor;
	Color m_DisabledColor;
	Color m_SelectedColor;
	Color m_FillColor;
};

class CMapSetItemPanelMapSet : public CMapSetItemPanel
{
public:
	DECLARE_CLASS_SIMPLE( CMapSetItemPanelMapSet, CMapSetItemPanel );

	CMapSetItemPanelMapSet( PanelListPanel *parent, const char* name, const char *imagename, KeyValues *mapset ) : BaseClass( parent, name, mapset->GetString( "name" ), imagename )
	{
		m_pMapSet = mapset;

		m_pParent = parent;
	}

	~CMapSetItemPanelMapSet()
	{
		if ( m_pMapSet )
		{
			m_pMapSet->deleteThis();
			m_pMapSet = NULL;
		}
	}
	
	MESSAGE_FUNC_INT( OnPanelSelected, "PanelSelected", state )
	{
		if ( state )
		{
			//m_pChapterLabel->SetFgColor( m_SelectedColor );
			m_pLevelPicBorder->SetFillColor( m_SelectedColor );
			m_pChapterNameLabel->SetFgColor( m_SelectedColor );

			CMapSetDialog *pMapSetDialog = dynamic_cast<CMapSetDialog*>( m_pParent->GetParent() );
			if ( pMapSetDialog )
			{
				pMapSetDialog->SetupMapList( this );
				vgui::Button *pStartButton = (vgui::Button *)pMapSetDialog->FindChildByName( "Play" );
				if ( pStartButton )
				{
					pStartButton->SetEnabled( false );
				}
			}
		}
		else
		{
			//m_pChapterLabel->SetFgColor( m_TextColor );
			m_pLevelPicBorder->SetFillColor( m_FillColor );
			m_pChapterNameLabel->SetFgColor( m_TextColor );
		}

		PostMessage( m_pParent->GetVParent(), new KeyValues("PanelSelected") );
	}
	
	virtual void OnMouseDoublePressed( vgui::MouseCode code )
	{
		// call the panel
		OnMousePressed( code );
		//PostMessage( m_pParent->GetParent(), new KeyValues("Command", "command", "play") );
	}

	int GetRequiredPlayers() { return m_pMapSet->GetInt( "required_players" ); }

	KeyValues *m_pMapSet;
};

class CMapSetItemPanelMap : public CMapSetItemPanel
{
public:
	DECLARE_CLASS_SIMPLE( CMapSetItemPanelMap, CMapSetItemPanel );

	CMapSetItemPanelMap( PanelListPanel *parent, const char *name, const char *titlename, const char *imagename, const char *map ) : BaseClass( parent, name, titlename, imagename )
	{
		V_strcpy( m_szMap, map );
	}
	
	MESSAGE_FUNC_INT( OnPanelSelected, "PanelSelected", state )
	{
		if ( state )
		{
			//m_pChapterLabel->SetFgColor( m_SelectedColor );
			m_pLevelPicBorder->SetFillColor( m_SelectedColor );
			m_pChapterNameLabel->SetFgColor( m_SelectedColor );

			CMapSetDialog *pMapSetDialog = dynamic_cast<CMapSetDialog*>( m_pParent->GetParent() );
			if ( pMapSetDialog )
			{
				V_strcpy( pMapSetDialog->m_szMap, m_szMap );
				vgui::Button *pStartButton = (vgui::Button *)pMapSetDialog->FindChildByName( "Play" );
				if ( pStartButton )
				{
					pStartButton->SetEnabled( true );
				}
			}
		}
		else
		{
			//m_pChapterLabel->SetFgColor( m_TextColor );
			m_pLevelPicBorder->SetFillColor( m_FillColor );
			m_pChapterNameLabel->SetFgColor( m_TextColor );
		}

		PostMessage( m_pParent->GetVParent(), new KeyValues("PanelSelected") );
	}

	char m_szMap[32];
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

void CMapSetDialog::SetupMapSetList( void )
{
	int i = 0;
	KeyValues *mapsets = new KeyValues( "mapsets" );
	mapsets->LoadFromFile( g_pFullFileSystem, "scripts/mapsets/mapsets_official.txt" );
	// Parse the official mapsets first
	ParseMapSetKeyValues( mapsets, i );
	//mapsets->deleteThis();
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
			//mapsets->deleteThis();

			pDirFileName = g_pFullFileSystem->FindNext( dirHandle );
		}

		g_pFullFileSystem->FindClose( dirHandle );
	}
}

void CMapSetDialog::SetupMapList( CMapSetItemPanelMapSet *pMapPanel )
{
	m_pMapList->DeleteAllItems();
	memset( m_szMap, 0, sizeof( m_szMap ) );

	// add chapters to combobox
	KeyValues *maps = NULL;
	for ( KeyValues *subkey = pMapPanel->m_pMapSet->GetFirstSubKey(); subkey != NULL; subkey = subkey->GetNextKey() )
	{
		if ( !V_strcmp( subkey->GetName(), "maps" ) )
		{
			maps = subkey;
			break;
		}
	}

	if ( !maps )
		return;
	
	for ( KeyValues *map = maps->GetFirstSubKey(); map != NULL; map = map->GetNextKey() )
	{
		char szImage[64];
		V_snprintf( szImage, sizeof( szImage ), "maps/menu_thumb_%s", map->GetName() );
		CMapSetItemPanelMap *chapterPanel = SETUP_PANEL( new CMapSetItemPanelMap( m_pMapSetList, NULL, map->GetString(), szImage, map->GetName() ) );
		chapterPanel->SetVisible( true );
		chapterPanel->InvalidateLayout( true );

		m_pMapList->AddItem( NULL, chapterPanel );
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
	SetupMapSetList();
}

void CMapSetDialog::OnCommand( const char *command )
{
	if ( !V_stricmp( command, "play" ) )
	{
		if ( m_szMap[0] != 0 )
		{
			KeyValues *mapdata = LoadMapDataForMap( m_szMap );
			int nRequiredPlayers = mapdata->GetInt("required_players");
		
			// Set commentary
			ConVarRef commentary( "commentary" );
			commentary.SetValue( false );

			ConVarRef sv_cheats( "sv_cheats" );
			sv_cheats.SetValue( false );
		
			// Also set certain convars that are necessary for a vanilla experience
			{
				ConVarRef pcoop_spectate_after_past_required_players( "pcoop_spectate_after_past_required_players" );
				pcoop_spectate_after_past_required_players.SetValue( true );
		
				ConVarRef pcoop_require_all_players( "pcoop_require_all_players" );
				pcoop_require_all_players.SetValue( true );
		
				ConVarRef pcoop_require_all_players_force_amount( "pcoop_require_all_players_force_amount" );
				pcoop_require_all_players_force_amount.SetValue( -2 );
			}

			char szCommand[128];
			V_snprintf( szCommand, sizeof( szCommand ), "disconnect\nwait\nwait\nmaxplayers %i\nmap %s\n", nRequiredPlayers, m_szMap );
			engine->ClientCmd_Unrestricted( szCommand );

			mapdata->deleteThis();
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
		char szImage[32];
		V_snprintf( szImage, sizeof( szImage ), "mapsets/%s", mapset->GetName() );
		CMapSetItemPanelMapSet *chapterPanel = SETUP_PANEL( new CMapSetItemPanelMapSet( m_pMapSetList, NULL, szImage, mapset ) );
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