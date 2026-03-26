#include "cbase.h"
#include "trigger_tractorbeam_shared.h"
#include "c_portal_player.h"
#include "soundinfo.h"

#undef CProjectedTractorBeamEntity // Just in case

#define PORTAL_TRIGGERTRACTOR_FORWARD_BEAM_COLOR			"10 160 255"
#define PORTAL_TRIGGERTRACTOR_REVERSED_BEAM_COLOR			"255 160 32"

ConVar cl_projected_tractor_beam_speed("cl_projected_tractor_beam_speed", "45", FCVAR_CLIENTDLL);

IMPLEMENT_CLIENTCLASS_DT(C_ProjectedTractorBeamEntity, DT_ProjectedTractorBeamEntity, CProjectedTractorBeamEntity)
RecvPropEHandle(RECVINFO(m_hTractorBeamTrigger)),
END_RECV_TABLE()

BEGIN_PREDICTION_DATA(C_ProjectedTractorBeamEntity)

DEFINE_FIELD(m_hTractorBeamTrigger, FIELD_EHANDLE),

END_PREDICTION_DATA()

C_ProjectedTractorBeamEntity::C_ProjectedTractorBeamEntity()
{

}

C_ProjectedTractorBeamEntity::~C_ProjectedTractorBeamEntity()
{

}

void C_ProjectedTractorBeamEntity::GetProjectionExtents(Vector& outMins, Vector& outMaxs)
{
	outMins.x = -2.0;
	outMins.y = -2.0;
	outMins.z = 0.0;
	outMaxs.x = 2.0;
	outMaxs.y = 2.0;
	outMaxs.z = 0.0;
}

void UpdateAllTractorBeams()
{
	for (int i = 0; i < ITriggerTractorBeamAutoList::AutoList().Count(); ++i)
	{
		C_Trigger_TractorBeam* pBeam = static_cast<C_Trigger_TractorBeam*>(ITriggerTractorBeamAutoList::AutoList()[i]);
		if (!pBeam)
			continue;

		const Vector& newStart = pBeam->m_vStart;
		const Vector& newEnd = pBeam->m_vEnd;
		const float newForce = pBeam->m_linearForce;
		const bool newReversed = (newForce < 0.0f);
		const bool visibleNow = pBeam->ShouldDraw();

		const bool bJustBecameVisible = (!pBeam->m_bWasVisible && visibleNow);

		const bool bChanged =
			(pBeam->m_lastUpdatedStart != newStart) ||
			(pBeam->m_lastUpdatedEnd != newEnd) ||
			(pBeam->m_lastUpdatedForce != newForce) ||
			(pBeam->m_lastUpdatedReversed != newReversed);

		if (bChanged || bJustBecameVisible)
		{
			pBeam->UpdateBeam(newStart, newEnd, newForce);

			pBeam->CreateParticles();

			pBeam->m_lastUpdatedStart = newStart;
			pBeam->m_lastUpdatedEnd = newEnd;
			pBeam->m_lastUpdatedForce = newForce;
			pBeam->m_lastUpdatedReversed = newReversed;
		}

		pBeam->m_bWasVisible = visibleNow;
	}
}

void ForceUpdateAllTractorBeams()
{
	for (int i = 0; i < ITriggerTractorBeamAutoList::AutoList().Count(); ++i)
	{
		C_Trigger_TractorBeam* pBeam = static_cast<C_Trigger_TractorBeam*>(ITriggerTractorBeamAutoList::AutoList()[i]);
		if (!pBeam)
			continue;

		const Vector& start = pBeam->m_vStart;
		const Vector& end = pBeam->m_vEnd;
		const float force = pBeam->m_linearForce;
		const bool reversed = pBeam->m_bReversed;


		pBeam->UpdateBeam(start, end, force);


		pBeam->m_lastUpdatedStart = start;
		pBeam->m_lastUpdatedEnd = end;
		pBeam->m_lastUpdatedForce = force;
		pBeam->m_lastUpdatedReversed = reversed;

		pBeam->m_bWasVisible = pBeam->ShouldDraw();
	}
}



void C_ProjectedTractorBeamEntity::OnProjected(void)
{
	BaseClass::OnProjected();
	if (m_hTractorBeamTrigger)
	{
		m_hTractorBeamTrigger->SetPredictionEligible(GetPredictionEligible());

		if (IsPlayerSimulated())
		{
			if (GetSimulatingPlayer())
				m_hTractorBeamTrigger->SetPlayerSimulated(GetSimulatingPlayer());
			else
				m_hTractorBeamTrigger->SetPlayerSimulated(NULL);
		}
		else
		{
			m_hTractorBeamTrigger->UnsetPlayerSimulated();
		}

		UpdateAllTractorBeams();
	}
}

#undef CTrigger_TractorBeam

IMPLEMENT_CLIENTCLASS_DT(C_Trigger_TractorBeam, DT_Trigger_TractorBeam, CTrigger_TractorBeam)

RecvPropFloat(RECVINFO(m_gravityScale)),
RecvPropFloat(RECVINFO(m_addAirDensity)),
RecvPropFloat(RECVINFO(m_linearLimit)),
RecvPropFloat(RECVINFO(m_linearLimitDelta)),
RecvPropFloat(RECVINFO(m_linearLimitTime)),
RecvPropFloat(RECVINFO(m_linearLimitStart)),
RecvPropFloat(RECVINFO(m_linearLimitStartTime)),
RecvPropFloat(RECVINFO(m_linearScale)),
RecvPropFloat(RECVINFO(m_angularLimit)),
RecvPropFloat(RECVINFO(m_angularScale)),
RecvPropFloat(RECVINFO(m_linearForce)),
RecvPropFloat(RECVINFO(m_flRadius)),

RecvPropQAngles(RECVINFO(m_linearForceAngles)),
RecvPropVector(RECVINFO(m_vStart)),
RecvPropVector(RECVINFO(m_vEnd)),

RecvPropBool(RECVINFO(m_bDisabled)),
RecvPropBool(RECVINFO(m_bReversed)),
RecvPropBool(RECVINFO(m_bFromPortal)),
RecvPropBool(RECVINFO(m_bToPortal)),
RecvPropBool(RECVINFO(m_bDisablePlayerMove)),

END_RECV_TABLE()

LINK_ENTITY_TO_CLASS(trigger_tractorbeam, C_Trigger_TractorBeam)


BEGIN_PREDICTION_DATA(C_Trigger_TractorBeam)

DEFINE_PRED_FIELD(m_linearForceAngles, FIELD_VECTOR, FTYPEDESC_INSENDTABLE),
DEFINE_PRED_FIELD(m_vStart, FIELD_VECTOR, FTYPEDESC_INSENDTABLE),
DEFINE_PRED_FIELD(m_vEnd, FIELD_VECTOR, FTYPEDESC_INSENDTABLE),

END_PREDICTION_DATA()


IMPLEMENT_AUTO_LIST(ITriggerTractorBeamAutoList)

C_Trigger_TractorBeam::C_Trigger_TractorBeam()
{
	m_blobs.Purge();
	m_hProxyEntity = NULL;
	m_hCoreEffect = NULL;
	m_bDisabled = false;

	m_flLength = 0.0f;
	m_flCurDisplayLength = 0.0f;

	int r1 = 0xFF, g1 = 0x00, b1 = 0x00;
	int r2 = 0xFF, g2 = 0x00, b2 = 0x00;
	const char* szColours1 = PORTAL_TRIGGERTRACTOR_FORWARD_BEAM_COLOR;
	const char* szColours2 = PORTAL_TRIGGERTRACTOR_REVERSED_BEAM_COLOR;
	if (szColours1 != NULL && Q_strlen(szColours1) > 0) {
		sscanf(szColours1, "%i%i%i", &r1, &g1, &b1);
	}
	if (szColours2 != NULL && Q_strlen(szColours2) > 0) {
		sscanf(szColours2, "%i%i%i", &r2, &g2, &b2);
	}

	m_TBeamForwardColor.Init(r1, g1, b1);
	m_TBeamReversedColor.Init(r2, g2, b2);

}

C_Trigger_TractorBeam::~C_Trigger_TractorBeam()
{
	m_hCoreEffect();
}

void C_Trigger_TractorBeam::Spawn(void)
{
	BaseClass::Spawn();
	if (!m_pMaterial1)
		m_pMaterial1 = materials->FindMaterial("effects/tractor_beam", NULL, false);
	if (!m_pMaterial2)
		m_pMaterial2 = materials->FindMaterial("effects/tractor_beam2", NULL, false);
	if (!m_pMaterial3)
		m_pMaterial3 = materials->FindMaterial("effects/tractor_beam3", NULL, false);

	SetNextClientThink(CLIENT_THINK_ALWAYS);

}

void C_Trigger_TractorBeam::UpdateOnRemove(void)
{
	if (m_pController)
	{
		IPhysicsObject* pList[1024];
		int objectcount = m_pController->CountObjects();
		m_pController->GetObjects(pList);
		for (int i = 0; i < objectcount; ++i)
		{
			if (pList[i])
				pList[i]->Wake();
		}

		physenv->DestroyMotionController(m_pController);
		m_pController = NULL;
	}

	if (m_hCoreEffect)
	{
		ParticleProp()->StopEmission(m_hCoreEffect, false, true);
	}

	for (int i = 1; i <= MAX_PLAYERS; ++i)
	{
		C_Portal_Player* pPlayer = (C_Portal_Player*)UTIL_PlayerByIndex(i);

		if (!pPlayer)
			continue;

		C_Trigger_TractorBeam* pTrigger = pPlayer->GetTractorBeam();

		if (pTrigger == this)
			pPlayer->SetLeaveTractorBeam(this, false);
	}
	BaseClass::UpdateOnRemove();
}

void C_Trigger_TractorBeam::StartTouch(C_BaseEntity* pOther)
{
	C_Portal_Player* pPlayer = ToPortalPlayer(pOther);

	if (pPlayer)
	{
		pPlayer->SetInTractorBeam(this);
	}
	else
	{
		if (m_pController && m_pPhysicsObject)
		{
			m_pController->AttachObject(m_pPhysicsObject, true);
		}
	}

	BaseClass::StartTouch(pOther);
}

void C_Trigger_TractorBeam::ClientThink()
{
	BaseClass::ClientThink();

	Vector vDir = m_vEnd - m_vStart;
	m_flLength = VectorNormalize(vDir);

	float flTargetLength = m_flLength;
	trace_t tr;
	UTIL_TraceLine(m_vStart, m_vEnd, MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr);
	if (tr.fraction < 1.0f)
	{
		m_vTargetDisplayPoint = tr.endpos;
		flTargetLength = (tr.endpos - m_vStart).Length();
	}
	else
	{
		m_vTargetDisplayPoint = m_vEnd;
	}

	float speed = cl_projected_tractor_beam_speed.GetFloat();
	float flGrow = fminf(flTargetLength, speed);
	float flNext = m_flCurDisplayLength + flGrow;
	if (flNext >= 0.0f)
		m_flCurDisplayLength = fminf(flTargetLength, flNext);

	SetNextClientThink(gpGlobals->curtime + 0.016f);
}

int C_Trigger_TractorBeam::DrawModel(int flags)
{

	Vector vDir = m_vEnd - m_vStart;

	m_flLength = VectorNormalize(vDir);

	// Smooth display length (interpolation)
	const float flApproachSpeed = 10.0f;
	m_flCurDisplayLength = Approach(m_flLength, m_flCurDisplayLength, gpGlobals->frametime * flApproachSpeed);

	float flLength = m_flCurDisplayLength;

#if 0 // TODO: This is probably supposed to be the code that makes the funnel do its "extend" animation - disabling for now until we get it working
	if ((gpGlobals->curtime - m_flStartTime) < 0.5)
	{
		float flMod1;

		// This code doesn't make any fucking sense
		if (m_flStartTime == (m_flStartTime + 0.5))
		{
			if (gpGlobals->curtime < (m_flStartTime + 0.5))
				flMod1 = 0.0;
			else
				flMod1 = 1.0;
		}
		else
		{
			float flMod2;
			float flRemainingTime = (gpGlobals->curtime - m_flStartTime) / ((m_flStartTime + 0.5) - m_flStartTime);
			if (flRemainingTime >= 0.0)
			{
				if (flRemainingTime <= 1.0)
					flMod2 = flRemainingTime;
				else
					flMod2 = 1.0;
			}
			else
			{
				flMod2 = 0.0;
			}
			flMod1 = ((flMod2 * flMod2) * 3.0) - (((flMod2 * flMod2) * 2.0) * flMod2);
		}
		flLength = flMod1 * flLength;
	}
#endif

	matrix3x4_t xform;
	QAngle angles;

	VectorAngles(vDir, angles);
	AngleMatrix(angles, m_vStart, xform);

	Vector xAxis;
	Vector yAxis;
	MatrixGetColumn(xform, 2, xAxis);
	MatrixGetColumn(xform, 1, yAxis);

	C_Trigger_TractorBeam::DrawColumn(m_pMaterial1, m_vStart, vDir, flLength, xAxis, yAxis, 58.0, 1.0, m_bFromPortal, m_bToPortal, 0.0);

	return 1;
}

void C_Trigger_TractorBeam::DrawColumn(IMaterial* pMaterial, Vector& vecStart, Vector vDir, float flLength,
	Vector& vecXAxis, Vector& vecYAxis, float flRadius, float flAlpha, bool bPinchIn, bool bPinchOut, float flTextureOffset)
{
	CMatRenderContextPtr pRenderContext(materials);
	IMesh* pMesh = pRenderContext->GetDynamicMesh(false, NULL, NULL, pMaterial);

	CMeshBuilder meshBuilder;
	meshBuilder.Begin(pMesh, MATERIAL_QUADS, 256);

	DrawColumnSegment(meshBuilder, vecStart, vDir, flLength, vecXAxis, vecYAxis, flRadius, flAlpha, flTextureOffset, pMaterial->GetVertexFormat());

	meshBuilder.End();

	pMesh->Draw();
}

void C_Trigger_TractorBeam::DrawColumnSegment(CMeshBuilder& meshBuilder, Vector& vecStart, Vector& vDir, float flLength, Vector& vecXAxis,
	Vector& vecYAxis, float flRadius, float flAlpha, float flTextureOffset, VertexFormat_t vertexFormat)
{
	bool bSetTangentS = (vertexFormat & VERTEX_TANGENT_S) != 0;
	bool bSetTangentT = (vertexFormat & VERTEX_TANGENT_T) != 0;

	Vector vecPosition = vecStart + (vecXAxis * flRadius);

	int colorR;
	int colorG;
	int colorB;
	if (m_bReversed)
	{
		colorR = m_TBeamReversedColor->r;
		colorG = m_TBeamReversedColor->g;
		colorB = m_TBeamReversedColor->b;
	}
	else
	{
		colorR = m_TBeamForwardColor->r;
		colorG = m_TBeamForwardColor->g;
		colorB = m_TBeamForwardColor->b;
	}

	// U scale based on beam length
	float flUScale = flLength * (1.0f / 256.0f);
	float flScrollOffset = gpGlobals->curtime * (m_linearForce * 0.00192) * -1;

	float flLastV = 0.0f;

	for (int i = 1; i <= 64; ++i)
	{
		Vector vecLastPosition = vecPosition;

		float fl = i * 0.098174773f;
		float flCos = cos(fl);
		float flSin = sin(fl);

		vecPosition = (vecStart + (vecXAxis * flCos) * flRadius) + ((vecYAxis * flSin) * flRadius);
		Vector normal = vecStart - vecPosition;

		Vector tangents = vDir;
		VectorNormalize(tangents);
		Vector tangentt = CrossProduct(tangents, normal);
		VectorNormalize(normal);
		VectorNormalize(tangentt);

		float flV = (float)i / 64.0f;

		// Vertex 1
		meshBuilder.Color3ub(colorR, colorG, colorB);
		meshBuilder.TexCoord2f(0, flScrollOffset, flV);
		meshBuilder.Position3fv(vecPosition.Base());
		if (vertexFormat & VERTEX_NORMAL) meshBuilder.Normal3fv(normal.Base());
		if (bSetTangentS) meshBuilder.TangentS3fv(tangents.Base());
		if (bSetTangentT) meshBuilder.TangentT3fv(tangentt.Base());
		meshBuilder.AdvanceVertex();

		// Vertex 2
		Vector vert2 = vDir * flLength + vecPosition;
		meshBuilder.Color3ub(colorR, colorG, colorB);
		meshBuilder.TexCoord2f(0, flScrollOffset + flUScale, flV);
		meshBuilder.Position3fv(vert2.Base());
		if (vertexFormat & VERTEX_NORMAL) meshBuilder.Normal3fv(normal.Base());
		if (bSetTangentS) meshBuilder.TangentS3fv(tangents.Base());
		if (bSetTangentT) meshBuilder.TangentT3fv(tangentt.Base());
		meshBuilder.AdvanceVertex();

		// Vertex 3
		Vector vert3 = vDir * flLength + vecLastPosition;
		normal = vecStart - vecLastPosition;
		VectorNormalize(normal);
		tangentt = CrossProduct(tangents, normal);
		VectorNormalize(tangentt);
		meshBuilder.Color3ub(colorR, colorG, colorB);
		meshBuilder.TexCoord2f(0, flScrollOffset + flUScale, flLastV);
		meshBuilder.Position3fv(vert3.Base());
		if (vertexFormat & VERTEX_NORMAL) meshBuilder.Normal3fv(normal.Base());
		if (bSetTangentS) meshBuilder.TangentS3fv(tangents.Base());
		if (bSetTangentT) meshBuilder.TangentT3fv(tangentt.Base());
		meshBuilder.AdvanceVertex();

		// Vertex 4
		meshBuilder.Color3ub(colorR, colorG, colorB);
		meshBuilder.TexCoord2f(0, flScrollOffset, flLastV);
		meshBuilder.Position3fv(vecLastPosition.Base());
		if (vertexFormat & VERTEX_NORMAL) meshBuilder.Normal3fv(normal.Base());
		if (bSetTangentS) meshBuilder.TangentS3fv(tangents.Base());
		if (bSetTangentT) meshBuilder.TangentT3fv(tangentt.Base());
		meshBuilder.AdvanceVertex();

		flLastV = flV;
	}
}

bool C_Trigger_TractorBeam::GetSoundSpatialization(SpatializationInfo_t& info)
{
	bool bResult = false;
	float outT[4]; // [esp+2Ch] [ebp-1Ch] BYREF

	if (IsDormant())
		return 0;

	if (info.pOrigin)
	{
		CalcClosestPointOnLine(info.info.vListenerOrigin, m_vStart, m_vEnd, *info.pOrigin, outT);
		if (outT[0] >= 0.0)
		{
			if (outT[0] > 1.0)
			{
				bResult = true;
				*info.pOrigin = m_vEnd;
				if (!info.pAngles)
					return bResult;
				goto LABEL_6;
			}
		}
		else
		{
			*info.pOrigin = m_vStart;
		}
	}
	bResult = true;
	if (info.pAngles)
	{
	LABEL_6:

		QAngle qAng = CollisionProp()->GetCollisionAngles();

		*info.pAngles = qAng;
	}

	return bResult;
}

void C_Trigger_TractorBeam::CreateParticles(void)
{
	m_flCurDisplayLength = 0.0;

	if (m_hCoreEffect)
	{
		ParticleProp()->StopEmission(m_hCoreEffect, false, true);
		m_hCoreEffect = NULL;
	}

	if (IsReversed())
		m_hCoreEffect = ParticleProp()->Create("tractor_beam_reversed_core", PATTACH_CUSTOMORIGIN);
	else
		m_hCoreEffect = ParticleProp()->Create("tractor_beam_core", PATTACH_CUSTOMORIGIN);


	if (m_hCoreEffect)
	{
		ParticleProp()->AddControlPoint(m_hCoreEffect, 1, this, PATTACH_CUSTOMORIGIN, NULL, vec3_origin);
		ParticleProp()->AddControlPoint(m_hCoreEffect, 2, this, PATTACH_CUSTOMORIGIN, NULL, vec3_origin);
		ParticleProp()->AddControlPoint(m_hCoreEffect, 3, this, PATTACH_CUSTOMORIGIN, NULL, vec3_origin);

		Vector vDir;
		vDir = m_vEnd - m_vStart;
		VectorNormalize(vDir);

		Vector vRight;
		Vector vUp;
		VectorVectors(vDir, vRight, vUp);

		m_hCoreEffect->SetControlPoint(0, m_vStart);
		m_hCoreEffect->SetControlPointOrientation(0, vDir, vRight, vUp);
		m_hCoreEffect->SetControlPoint(1, m_vEnd);

		Vector forward;
		forward = -vDir;

		m_hCoreEffect->SetControlPointOrientation(1, forward, vRight, vUp);

		matrix3x4_t matWorldTransform = EntityToWorldTransform();

		Vector vVelocity;
		vVelocity.x = matWorldTransform.m_flMatVal[0][0] * m_linearForce;
		vVelocity.y = matWorldTransform.m_flMatVal[1][0] * m_linearForce;
		vVelocity.z = matWorldTransform.m_flMatVal[2][0] * m_linearForce;

		m_hCoreEffect->SetControlPoint(2, vVelocity);

		Vector color;
		if (m_bReversed)
		{
			color.x = 255;
			color.y = 56;
			color.z = 8;
		}
		else
		{
			color.x = 0.0;
			color.y = 49;
			color.z = 189;
		}
		m_hCoreEffect->SetControlPoint(3, color);
	}
}

void C_Trigger_TractorBeam::PhysicsSimulate(void)
{
	BaseClass::PhysicsSimulate();
}

void C_Trigger_TractorBeam::OnDataChanged(DataUpdateType_t updateType)
{
	BaseClass::OnDataChanged(updateType);
	if (updateType == DATA_UPDATE_DATATABLE_CHANGED)
	{

		if (m_lastUpdatedReversed != m_bReversed)
			UpdateAllTractorBeams();
		else
			UpdateAllTractorBeams();

		if (!m_bRecreateParticles)
			return;
		m_bRecreateParticles = false;
	}
	else
	{
		if (!m_pController)
		{
			m_pController = physenv->CreateMotionController(this);
			UpdateBeam(m_vStart, m_vEnd, m_linearForce);
		}
	}
	CreateParticles();
}

C_BasePlayer* C_Trigger_TractorBeam::GetPredictionOwner(void)
{
	return GetSimulatingPlayer();
}

bool C_Trigger_TractorBeam::ShouldPredict(void)
{
	return GetSimulatingPlayer() == C_BasePlayer::GetLocalPlayer();
}

void C_Trigger_TractorBeam::OnNewParticleEffect(const char* pszParticleName, CNewParticleEffect* pNewParticleEffect)
{
	if (!V_stricmp(pszParticleName, "tractor_beam_src"))
		pNewParticleEffect->SetControlPoint(2, m_vEnd);
}

void C_Trigger_TractorBeam::GetToolRecordingState(KeyValues* msg)
{
	//No IFM for Swarm!
#if 0
	const char* HandlerIDKeyString; // eax

	BaseClass::GetToolRecordingState(msg);
	KeyValues* pKeyValues = CIFM_EntityKeyValuesHandler_AutoRegister::FindOrCreateNonConformantKeyValues(msg);
	HandlerIDKeyString = CIFM_EntityKeyValuesHandler_AutoRegister::GetHandlerIDKeyString();
	pKeyValues->SetString(HandlerIDKeyString, "C_Trigger_TractorBeam");

	pKeyValues->SetInt("entIndex", entindex());
	pKeyValues->SetFloat("starttime", m_flStartTime);
	pKeyValues->SetInt("reversed", m_bReversed);
	pKeyValues->SetFloat("force", m_linearForce);

	pKeyValues->SetFloat("sp_x", m_vStart.x);
	pKeyValues->SetFloat("sp_y", m_vStart.y);
	pKeyValues->SetFloat("sp_z", m_vStart.z);

	pKeyValues->SetFloat("ep_x", m_vEnd.x);
	pKeyValues->SetFloat("ep_y", m_vEnd.y);
	pKeyValues->SetFloat("ep_z", m_vEnd.z);
#endif
}

void C_Trigger_TractorBeam::RestoreToToolRecordedState(KeyValues* pKV)
{
#if 0
	_DWORD vecMin[2]; // [esp+Ch] [ebp-18h] BYREF
	unsigned __int64 vecMin_8; // [esp+14h] [ebp-10h] BYREF

	m_flStartTime = pKV->GetFloat("starttime", 0.0);
	m_bReversed = pKV->GetInt("reversed", 0) == 1;
	m_linearForce = pKV->GetFloat("force", 0.0);
	m_vStart.x = pKV->GetFloat("sp_x", 0.0);
	m_vStart.y = pKV->GetFloat("sp_y", 0.0);
	m_vStart.z = pKV->GetFloat("sp_z", 0.0);
	m_vEnd.x = pKV->GetFloat("ep_x", 0.0);
	m_vEnd.y = pKV->GetFloat("ep_y", 0.0);
	m_vEnd.z = pKV->GetFloat("ep_z", 0.0);
	float v5 = 16384.0;
	float v6 = 16384.0;
	*(float*)vecMin = 16384.0;
	*(float*)&vecMin[1] = 16384.0;
	vecMin_8 = __PAIR64__(LODWORD(16384.0), LODWORD(16384.0));
	SetSize((const Vector*)vecMin, (const Vector*)((char*)&vecMin_8 + 4));
#endif
	m_pMaterial1 = materials->FindMaterial("effects/tractor_beam", 0, 0, 0);
	m_pMaterial2 = materials->FindMaterial("effects/tractor_beam2", 0, 0, 0);
	m_pMaterial3 = materials->FindMaterial("effects/tractor_beam3", 0, 0, 0);
}

bool C_Trigger_TractorBeam::ShouldDraw(void)
{
	return true;
}

float C_Trigger_TractorBeam::GetLinearForce(void)
{
	return m_linearForce;
}

bool C_Trigger_TractorBeam::HasLinearLimit(void)
{
	return m_linearLimit != 0.0;
}

bool C_Trigger_TractorBeam::HasLinearScale(void)
{
	return m_linearScale != 1.0;
}

bool C_Trigger_TractorBeam::HasAngularScale(void)
{
	return m_angularScale != 1.0;
}

bool C_Trigger_TractorBeam::HasAngularLimit(void)
{
	return m_angularLimit != 0.0;
}

bool C_Trigger_TractorBeam::HasAirDensity(void)
{
	return m_addAirDensity != 0.0;
}