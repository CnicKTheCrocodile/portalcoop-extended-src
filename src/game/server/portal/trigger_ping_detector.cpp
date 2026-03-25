//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Brush trigger that fires OnTrigger when a coop ping lands inside.
//
//=============================================================================//

#include "cbase.h"
#include "triggers.h"
#include "GameEventListener.h"
#include "igameevents.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CTriggerPingDetector : public CBaseTrigger, public CGameEventListener
{
public:
	DECLARE_CLASS( CTriggerPingDetector, CBaseTrigger );
	DECLARE_DATADESC();

	CTriggerPingDetector() : m_iPlayerIndex( 0 ) {}

	virtual void Spawn( void );
	virtual void UpdateOnRemove( void );
	virtual void FireGameEvent( IGameEvent *event );

	void InputEnable( inputdata_t &inputdata );
	void InputDisable( inputdata_t &inputdata );
	void InputToggle( inputdata_t &inputdata );

private:
	int m_iPlayerIndex;
	COutputEvent m_OnTrigger;
};

LINK_ENTITY_TO_CLASS( trigger_ping_detector, CTriggerPingDetector );

BEGIN_DATADESC( CTriggerPingDetector )

	DEFINE_KEYFIELD( m_bDisabled, FIELD_BOOLEAN, "StartDisabled" ),
	DEFINE_KEYFIELD( m_iPlayerIndex, FIELD_INTEGER, "PlayerIndex" ),

	DEFINE_INPUTFUNC( FIELD_VOID, "Enable", InputEnable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Disable", InputDisable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Toggle", InputToggle ),

	DEFINE_OUTPUT( m_OnTrigger, "OnTrigger" ),

END_DATADESC()

void CTriggerPingDetector::Spawn( void )
{
	BaseClass::Spawn();

	InitTrigger();
	ListenForGameEvent( "portal_player_ping" );
}

void CTriggerPingDetector::UpdateOnRemove( void )
{
	StopListeningForAllEvents();
	BaseClass::UpdateOnRemove();
}

void CTriggerPingDetector::FireGameEvent( IGameEvent *event )
{
	if ( m_bDisabled )
		return;

	if ( !event || Q_strcmp( event->GetName(), "portal_player_ping" ) != 0 )
		return;

	const int iUserID = event->GetInt( "userid", 0 );
	CBasePlayer *pActivator = UTIL_PlayerByUserId( iUserID );
	if ( !pActivator )
		return;

	// 0 means any player; otherwise match only the selected player index.
	if ( m_iPlayerIndex > 0 && pActivator->entindex() != m_iPlayerIndex )
		return;

	if ( !PassesTriggerFilters( pActivator ) )
		return;

	Vector vecPingPos;
	vecPingPos.x = event->GetFloat( "ping_x", 0.0f );
	vecPingPos.y = event->GetFloat( "ping_y", 0.0f );
	vecPingPos.z = event->GetFloat( "ping_z", 0.0f );

	if ( !PointIsWithin( vecPingPos ) )
		return;

	m_OnTrigger.FireOutput( pActivator, this );
}

void CTriggerPingDetector::InputEnable( inputdata_t &inputdata )
{
	BaseClass::InputEnable( inputdata );
}

void CTriggerPingDetector::InputDisable( inputdata_t &inputdata )
{
	BaseClass::InputDisable( inputdata );
}

void CTriggerPingDetector::InputToggle( inputdata_t &inputdata )
{
	BaseClass::InputToggle( inputdata );
}
