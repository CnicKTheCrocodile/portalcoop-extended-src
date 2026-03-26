//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: A volume in which no portal can be placed. Keeps a global list loaded in from the map
//			and provides an interface with which prop_portal can get this list and avoid successfully
//			creating portals wholly or partially inside the volume.
//
// $NoKeywords: $
//======================================================================================//

#include "cbase.h"
#include "func_portal_detector.h"
#include "prop_portal_shared.h"
#include "portal_shareddefs.h"
#include "portal_util_shared.h"


// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Spawnflags
#define SF_START_INACTIVE			0x01


LINK_ENTITY_TO_CLASS( func_portal_detector, CFuncPortalDetector );

BEGIN_DATADESC( CFuncPortalDetector )

	DEFINE_FIELD( m_bActive, FIELD_BOOLEAN ),
	DEFINE_KEYFIELD( m_iLinkageGroupID, FIELD_INTEGER, "LinkageGroupID" ),
	DEFINE_KEYFIELD( m_bShouldUseLinkageID, FIELD_BOOLEAN, "ShouldUseLinkageID" ),

	// Inputs
	DEFINE_INPUTFUNC( FIELD_VOID, "Disable", InputDisable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Enable", InputEnable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Toggle", InputToggle ),

	DEFINE_OUTPUT( m_OnStartTouchPortal, "OnStartTouchPortal" ),
	DEFINE_OUTPUT( m_OnStartTouchPortal1, "OnStartTouchPortal1" ),
	DEFINE_OUTPUT( m_OnStartTouchPortal2, "OnStartTouchPortal2" ),
	DEFINE_OUTPUT( m_OnStartTouchLinkedPortal, "OnStartTouchLinkedPortal" ),
	DEFINE_OUTPUT( m_OnStartTouchBothLinkedPortals, "OnStartTouchBothLinkedPortals" ),
	DEFINE_OUTPUT( m_OnEndTouchPortal, "OnEndTouchPortal" ),
	DEFINE_OUTPUT( m_OnEndTouchPortal1, "OnEndTouchPortal1" ),
	DEFINE_OUTPUT( m_OnEndTouchPortal2, "OnEndTouchPortal2" ),
	DEFINE_OUTPUT( m_OnEndTouchLinkedPortal, "OnEndTouchLinkedPortal" ),
	DEFINE_OUTPUT( m_OnEndTouchBothLinkedPortals, "OnEndTouchBothLinkedPortals" ),


	DEFINE_FUNCTION( IsActive ),

END_DATADESC()
/*
IMPLEMENT_SERVERCLASS_ST(CFuncPortalDetector, DT_FuncPortalDetector )

	SendPropBool(SENDINFO(m_bActive)),
	SendPropBool(SENDINFO(m_bShouldUseLinkageID)),
	SendPropInt(SENDINFO(m_iLinkageGroupID)),
	SendPropInt(SENDINFO(m_spawnflags), 0, SPROP_UNSIGNED),

END_NETWORK_TABLE()
*/
void CFuncPortalDetector::Spawn()
{
	BaseClass::Spawn();

	if ( m_spawnflags & SF_START_INACTIVE )
	{
		m_bActive = false;
	}
	else
	{
		m_bActive = true;
	}

	// Bind to our model, cause we need the extents for bounds checking
	SetModel( STRING( GetModelName() ) );
	SetRenderMode( kRenderNone );	// Don't draw
	SetSolid( SOLID_VPHYSICS );		// we may want slanted walls, so we'll use OBB
	AddSolidFlags( FSOLID_NOT_SOLID );

	m_bLastTouchedPortal1 = false;
	m_bLastTouchedPortal2 = false;
	m_bLastTouchedLinkedPortal = false;
	m_bLastTouchedBothLinked = false;

	SetThink( &CFuncPortalDetector::Think );
	SetNextThink( gpGlobals->curtime + 0.1f );
}

void CFuncPortalDetector::OnActivate( void )
{
	m_bActive = true;
	m_bLastTouchedPortal1 = false;
	m_bLastTouchedPortal2 = false;
	m_bLastTouchedLinkedPortal = false;
	m_bLastTouchedBothLinked = false;

	SetNextThink( gpGlobals->curtime );
}

void CFuncPortalDetector::Think( void )
{
	if ( !m_bActive )
	{
		SetNextThink( gpGlobals->curtime + 0.1f );
		return;
	}

	Vector vMin, vMax;
	CollisionProp()->WorldSpaceAABB( &vMin, &vMax );

	Vector vBoxCenter = ( vMin + vMax ) * 0.5f;
	Vector vBoxExtents = ( vMax - vMin ) * 0.5f;

	bool bTouchedPortal1 = false;
	bool bTouchedPortal2 = false;
	bool bTouchedLinked = false;

	int iPortalCount = CProp_Portal_Shared::AllPortals.Count();
	if ( iPortalCount != 0 )
	{
		CProp_Portal **pPortals = CProp_Portal_Shared::AllPortals.Base();
		for ( int i = 0; i < iPortalCount; ++i )
		{
			CProp_Portal *pTempPortal = pPortals[i];
			if ( !pTempPortal )
				continue;

			if ( !pTempPortal->IsActive() )
				continue;

			if ( m_bShouldUseLinkageID && pTempPortal->GetLinkageGroup() != m_iLinkageGroupID )
				continue;

			if ( UTIL_IsBoxIntersectingPortal( vBoxCenter, vBoxExtents, pTempPortal ) )
			{
				if ( pTempPortal->IsPortal2() )
				{
					bTouchedPortal2 = true;
				}
				else
				{
					bTouchedPortal1 = true;
				}

				if ( pTempPortal->IsActivedAndLinked() )
				{
					bTouchedLinked = true;
				}
			}
		}
	}

	bool bTouchedBoth = ( bTouchedPortal1 && bTouchedPortal2 );
	bool bTouchedAny = ( bTouchedPortal1 || bTouchedPortal2 );
	bool bLastTouchedAny = ( m_bLastTouchedPortal1 || m_bLastTouchedPortal2 );

	if ( bTouchedAny && !bLastTouchedAny )
	{
		m_OnStartTouchPortal.FireOutput( this, this );
	}
	else if ( !bTouchedAny && bLastTouchedAny )
	{
		m_OnEndTouchPortal.FireOutput( this, this );
	}

	if ( bTouchedPortal1 && !m_bLastTouchedPortal1 )
	{
		m_OnStartTouchPortal1.FireOutput( this, this );
	}
	else if ( !bTouchedPortal1 && m_bLastTouchedPortal1 )
	{
		m_OnEndTouchPortal1.FireOutput( this, this );
	}

	if ( bTouchedPortal2 && !m_bLastTouchedPortal2 )
	{
		m_OnStartTouchPortal2.FireOutput( this, this );
	}
	else if ( !bTouchedPortal2 && m_bLastTouchedPortal2 )
	{
		m_OnEndTouchPortal2.FireOutput( this, this );
	}

	if ( bTouchedLinked && !m_bLastTouchedLinkedPortal )
	{
		m_OnStartTouchLinkedPortal.FireOutput( this, this );
	}
	else if ( !bTouchedLinked && m_bLastTouchedLinkedPortal )
	{
		m_OnEndTouchLinkedPortal.FireOutput( this, this );
	}

	if ( bTouchedBoth && !m_bLastTouchedBothLinked )
	{
		m_OnStartTouchBothLinkedPortals.FireOutput( this, this );
	}
	else if ( !bTouchedBoth && m_bLastTouchedBothLinked )
	{
		m_OnEndTouchBothLinkedPortals.FireOutput( this, this );
	}

	m_bLastTouchedPortal1 = bTouchedPortal1;
	m_bLastTouchedPortal2 = bTouchedPortal2;
	m_bLastTouchedLinkedPortal = bTouchedLinked;
	m_bLastTouchedBothLinked = bTouchedBoth;

	SetNextThink( gpGlobals->curtime + 0.1f );
}

void CFuncPortalDetector::InputDisable( inputdata_t &inputdata )
{
	m_bActive = false;
}

void CFuncPortalDetector::InputEnable( inputdata_t &inputdata )
{
	m_bActive = true;

	OnActivate();
}

void CFuncPortalDetector::InputToggle( inputdata_t &inputdata )
{
	m_bActive = !m_bActive;

	if ( m_bActive )
	{
		OnActivate();
	}
}
