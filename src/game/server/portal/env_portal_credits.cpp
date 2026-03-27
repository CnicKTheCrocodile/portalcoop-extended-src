//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implements visual effects entities: sprites, beams, bubbles, etc.
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "EnvMessage.h"
#include "engine/IEngineSound.h"
#include "KeyValues.h"
#include "filesystem.h"
#include "Color.h"
#include "gamestats.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


class CPortalCredits : public CPointEntity
{
public:
	DECLARE_CLASS( CMessage, CPointEntity );
	DECLARE_DATADESC();

	void	Spawn( void );
	void	InputRollCredits( inputdata_t &inputdata );
	void	InputRollOutroCredits( inputdata_t &inputdata );
	void	InputShowLogo( inputdata_t &inputdata );
	void	InputSetLogoLength( inputdata_t &inputdata );
	void	InputRollPortalOutroCredits( inputdata_t &inputdata );

	COutputEvent m_OnCreditsDone;

	virtual void Precache();
	virtual void OnRestore();
private:
	void		SendCreditsMessage( int nCreditsType );

	void		RollOutroCredits();
	void		RollPortalOutroCredits();

	bool		m_bRolledOutroCredits;
	float		m_flLogoLength;
};

LINK_ENTITY_TO_CLASS( env_portal_credits, CPortalCredits );

BEGIN_DATADESC( CPortalCredits )
	DEFINE_INPUTFUNC( FIELD_VOID, "RollCredits", InputRollCredits ),
	DEFINE_INPUTFUNC( FIELD_VOID, "RollOutroCredits", InputRollOutroCredits ),
	DEFINE_INPUTFUNC( FIELD_VOID, "RollPortalOutroCredits", InputRollPortalOutroCredits ),
	DEFINE_INPUTFUNC( FIELD_VOID, "ShowLogo", InputShowLogo ),
	DEFINE_INPUTFUNC( FIELD_FLOAT, "SetLogoLength", InputSetLogoLength ),
	DEFINE_OUTPUT( m_OnCreditsDone, "OnCreditsDone"),

	DEFINE_FIELD( m_bRolledOutroCredits, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flLogoLength, FIELD_FLOAT )
END_DATADESC()

void CPortalCredits::Spawn( void )
{
	SetSolid( SOLID_NONE );
	SetMoveType( MOVETYPE_NONE );
}

static void CreditsDone_f( void )
{
	CPortalCredits *pCredits = (CPortalCredits*)gEntList.FindEntityByClassname( NULL, "env_portal_credits" );

	if ( pCredits )
	{
		pCredits->m_OnCreditsDone.FireOutput( pCredits, pCredits );
	}
}

static ConCommand creditsdone("creditsdone", CreditsDone_f );

extern ConVar sv_unlockedchapters;


void CPortalCredits::Precache( void )
{
	PrecacheScriptSound( "Portal.song_credits" );

	BaseClass::Precache();
}

void CPortalCredits::OnRestore()
{
	BaseClass::OnRestore();

	if ( m_bRolledOutroCredits )
	{
		// Roll them again so that the client .dll will send the "creditsdone" message and we'll
		//  actually get back to the main menu
		RollOutroCredits();
	}
}

void CPortalCredits::RollOutroCredits()
{
	sv_unlockedchapters.SetValue( "15" );
	SendCreditsMessage( 3 );
}

void CPortalCredits::SendCreditsMessage( int nCreditsType )
{
	for ( int i = 1; i <= gpGlobals->maxClients; ++i )
	{
		CBasePlayer *pPlayer = UTIL_PlayerByIndex( i );
		if ( !pPlayer || !pPlayer->IsConnected() )
		{
			continue;
		}

		CSingleUserRecipientFilter user( pPlayer );
		user.MakeReliable();

		UserMessageBegin( user, "CreditsPortalMsg" );
			WRITE_BYTE( nCreditsType );
		MessageEnd();
	}
}

void CPortalCredits::InputRollOutroCredits( inputdata_t &inputdata )
{
		RollOutroCredits();

	// In case we save restore
	m_bRolledOutroCredits = true;

	gamestats->Event_Credits();
}

void CPortalCredits::RollPortalOutroCredits()
{
	sv_unlockedchapters.SetValue( "15" );
	SendCreditsMessage( 4 );
}

void CPortalCredits::InputRollPortalOutroCredits( inputdata_t &inputdata )
{
	RollPortalOutroCredits();

	// In case we save restore
	m_bRolledOutroCredits = true;

	gamestats->Event_Credits();
}


void CPortalCredits::InputShowLogo( inputdata_t &inputdata )
{
	if ( m_flLogoLength )
	{
		for ( int i = 1; i <= gpGlobals->maxClients; ++i )
		{
			CBasePlayer *pPlayer = UTIL_PlayerByIndex( i );
			if ( !pPlayer || !pPlayer->IsConnected() )
			{
				continue;
			}

			CSingleUserRecipientFilter user( pPlayer );
			user.MakeReliable();

			UserMessageBegin( user, "LogoTimeMsg" );
				WRITE_FLOAT( m_flLogoLength );
			MessageEnd();
		}
	}
	else
	{
		SendCreditsMessage( 1 );
	}
}

void CPortalCredits::InputSetLogoLength( inputdata_t &inputdata )
{
	m_flLogoLength = inputdata.value.Float();
}


void CPortalCredits::InputRollCredits( inputdata_t &inputdata )
{
	SendCreditsMessage( 2 );
}
