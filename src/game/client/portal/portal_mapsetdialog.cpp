#include "cbase.h"
#include "portal_mapsetdialog.h"
#include "vgui_controls/Button.h"
#include "vgui_controls/ImagePanel.h"
#include "ienginevgui.h"
#include "filesystem.h"

static CMapSetDialog *g_pMapSetDialog = NULL;

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: selectable item with screenshot for an individual chapter in the dialog
//-----------------------------------------------------------------------------
class CMapSetItemPanel : public EditablePanel
{
	DECLARE_CLASS_SIMPLE( CMapSetItemPanel, EditablePanel );

	ImagePanel *m_pLevelPicBorder;
	ImagePanel *m_pLevelPic;
	ImagePanel *m_pCommentaryIcon;
	//Label *m_pChapterLabel;
	Label *m_pChapterNameLabel;

	Color m_TextColor;
	Color m_DisabledColor;
	Color m_SelectedColor;
	Color m_FillColor;

	char m_szMapSetName[_MAX_PATH];
	char m_szSelectedMap[_MAX_PATH];
	int m_iRequiredPlayers;

	bool m_bTeaserChapter;
	bool m_bHasBonus;

	bool m_bIsSelected;

public:
	CMapSetItemPanel( CMapSetDialog *parent, const char *name, const char *chapterName,
						int chapterIndex, const char *mapsetName,
						int iRequiredPlayers, const char *mapname ) : BaseClass( parent, name )
	{
		Q_strncpy( m_szMapSetName, mapsetName, sizeof(m_szMapSetName) );
		Q_strncpy( m_szSelectedMap, mapname, sizeof(m_szSelectedMap) );		

		m_pLevelPicBorder = SETUP_PANEL( new ImagePanel( this, "LevelPicBorder" ) );
		m_pLevelPic = SETUP_PANEL( new ImagePanel( this, "LevelPic" ) );
		m_pCommentaryIcon = NULL;
		m_bIsSelected = false;

		m_iRequiredPlayers = iRequiredPlayers;

		//m_pChapterLabel = new Label( this, "ChapterLabel", text );
		m_pChapterNameLabel = new Label( this, "ChapterNameLabel", chapterName );

		SetPaintBackgroundEnabled( false );

		// the image has the same name as the config file
		char szMaterial[ MAX_PATH ];
		Q_snprintf( szMaterial, sizeof(szMaterial), "mapsets/%s", mapsetName );
		char *ext = strstr( szMaterial, "." );
		if ( ext )
		{
			*ext = 0;
		}
		m_pLevelPic->SetImage( szMaterial );

		KeyValues *pKeys = NULL;

		LoadControlSettings( "Resource/PortalNewGameChapterPanel.res", NULL, pKeys );

		int px, py;
		m_pLevelPicBorder->GetPos( px, py );
		SetSize( m_pLevelPicBorder->GetWide(), py + m_pLevelPicBorder->GetTall() );

		// create a selection panel the size of the page
		//CSelectionOverlayPanel *overlay = new CSelectionOverlayPanel( this, parent, chapterIndex );
		//overlay->SetBounds(0, 0, GetWide(), GetTall());
		//overlay->MoveToFront();

		// HACK: Detect new episode teasers by the "Coming Soon" text
		wchar_t w_szStrTemp[256];
		m_pChapterNameLabel->GetText( w_szStrTemp, sizeof(w_szStrTemp)  );
		m_bTeaserChapter = !wcscmp(w_szStrTemp, L"Coming Soon");
		m_bHasBonus = false;
	}

	virtual void ApplySchemeSettings( IScheme *pScheme )
	{
		m_TextColor = pScheme->GetColor( "NewGame.TextColor", Color(255, 255, 255, 255) );
		m_FillColor = pScheme->GetColor( "NewGame.FillColor", Color(255, 255, 255, 255) );
		m_DisabledColor = pScheme->GetColor( "NewGame.DisabledColor", Color(255, 255, 255, 255) );
		m_SelectedColor = pScheme->GetColor( "NewGame.SelectionColor", Color(255, 255, 255, 255) );

		BaseClass::ApplySchemeSettings( pScheme );

		// Hide chapter numbers for new episode teasers
		//if ( m_bTeaserChapter )
		//{
			//m_pChapterLabel->SetVisible( false );
		//}
		
		m_pCommentaryIcon = dynamic_cast<ImagePanel*>( FindChildByName( "CommentaryIcon" ) );
		if ( m_pCommentaryIcon )
			m_pCommentaryIcon->SetVisible( false );
	}

	bool IsSelected( void ) const { return m_bIsSelected; }

	void SetSelected( bool state )
	{
		m_bIsSelected = state;

		// update the text/border colors
		if ( !IsEnabled() )
		{
			//m_pChapterLabel->SetFgColor( m_DisabledColor );
			m_pChapterNameLabel->SetFgColor( Color(0, 0, 0, 0) );
			m_pLevelPicBorder->SetFillColor( m_DisabledColor );
			m_pLevelPic->SetAlpha( 128 );
			return;
		}

		if ( state )
		{
			//m_pChapterLabel->SetFgColor( m_SelectedColor );
			m_pChapterNameLabel->SetFgColor( m_SelectedColor );
			m_pLevelPicBorder->SetFillColor( m_SelectedColor );
		}
		else
		{
			//m_pChapterLabel->SetFgColor( m_TextColor );
			m_pChapterNameLabel->SetFgColor( m_TextColor );
			m_pLevelPicBorder->SetFillColor( m_FillColor );
		}
		m_pLevelPic->SetAlpha( 255 );
	}

	const char *GetMapSetName()
	{
		return m_szMapSetName;
	}

	const char *GetSelectedMap()
	{
		return m_szSelectedMap;
	}
	
	int GetRequiredPlayers()
	{
		return m_iRequiredPlayers;
	}

	bool IsTeaserChapter()
	{
		return m_bTeaserChapter;
	}

	bool HasBonus()
	{
		return m_bHasBonus;
	}

	void SetCommentaryMode( bool bCommentaryMode )
	{
		if ( m_pCommentaryIcon )
			m_pCommentaryIcon->SetVisible( bCommentaryMode );
	}
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

	SetupMapLists();
}

void CMapSetDialog::ParseMapSetKeyValues( KeyValues *mapsets, int &i )
{
	// add chapters to combobox
	for ( KeyValues *mapset = mapsets->GetFirstSubKey(); mapset != NULL; mapset = mapset->GetNextKey() )
	{
		char chapterName[64];
		Q_snprintf(chapterName, sizeof(chapterName), mapset->GetString( "name" ) );

		int required_players = mapset->GetInt( "required_players" );
		const char *map = mapset->GetString( "map" );
		CMapSetItemPanel *chapterPanel = SETUP_PANEL( new CMapSetItemPanel( this, NULL, chapterName, i, mapset->GetName(), required_players, map ) );
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