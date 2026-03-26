#include "cbase.h"
#include "prop_tractorbeam.h"
#include "soundenvelope.h"
#include "trigger_tractorbeam_shared.h"

#define PORTAL_PROPTRACTOR_FORWARD_BEAM_COLOR			"30 70 255"
#define PORTAL_PROPTRACTOR_REVERSED_BEAM_COLOR			"255 60 10"

IMPLEMENT_SERVERCLASS_ST(ProjectedEntityAmbientSoundProxy, DT_ProjectedEntityAmbientSoundProxy)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS(projected_entity_ambient_sound_proxy, ProjectedEntityAmbientSoundProxy)

ProjectedEntityAmbientSoundProxy::ProjectedEntityAmbientSoundProxy()
{

}

ProjectedEntityAmbientSoundProxy::~ProjectedEntityAmbientSoundProxy()
{

}

int ProjectedEntityAmbientSoundProxy::UpdateTransmitState(void)
{
	return SetTransmitState(FL_EDICT_DONTSEND);
}

ProjectedEntityAmbientSoundProxy* ProjectedEntityAmbientSoundProxy::Create(CBaseEntity* pAttachTo)
{
	ProjectedEntityAmbientSoundProxy* pSoundProxy = (ProjectedEntityAmbientSoundProxy*)CreateEntityByName("projected_entity_ambient_sound_proxy");
	Assert(pSoundProxy);
	pSoundProxy->AttachToEntity(pAttachTo);

	return pSoundProxy;
}

void ProjectedEntityAmbientSoundProxy::AttachToEntity(CBaseEntity* pAttachTo)
{
	SetParent(pAttachTo);
	SetLocalOrigin(vec3_origin);
	SetLocalAngles(vec3_angle);
}

BEGIN_DATADESC(CPropTractorBeamProjector)

DEFINE_INPUTFUNC(FIELD_FLOAT, "SetLinearForce", InputSetLinearForce),

DEFINE_KEYFIELD(m_flLinearForce, FIELD_FLOAT, "linearForce"),
DEFINE_KEYFIELD(m_bNoEmitterParticles, FIELD_BOOLEAN, "noemitterparticles"),
DEFINE_KEYFIELD(m_bUse128Model, FIELD_BOOLEAN, "use128model"),

DEFINE_SOUNDPATCH(m_sndMechanical),
DEFINE_SOUNDPATCH(m_sndAmbientSound),

DEFINE_FIELD(m_hAmbientSoundProxy, FIELD_EHANDLE),

END_DATADESC()

LINK_ENTITY_TO_CLASS(prop_tractor_beam, CPropTractorBeamProjector)

IMPLEMENT_SERVERCLASS_ST(CPropTractorBeamProjector, DT_PropTractorBeamProjector)

SendPropExclude("DT_BaseProjector", "m_bEnabled"),
SendPropFloat(SENDINFO(m_flLinearForce)),
//SendPropBool( SENDINFO( bDisableAutoReprojection ) ),

SendPropVector(SENDINFO(m_vEndPos)),
SendPropBool(SENDINFO(m_bEnabled)),
SendPropBool(SENDINFO(m_bNoEmitterParticles)),

END_SEND_TABLE()

CPropTractorBeamProjector::CPropTractorBeamProjector()
{
	m_bUse128Model = false;
	m_sndMechanical = NULL;
	m_hAmbientSoundProxy = NULL;

	int r1 = 0xFF, g1 = 0x00, b1 = 0x00;
	int r2 = 0xFF, g2 = 0x00, b2 = 0x00;
	const char* szColours1 = PORTAL_PROPTRACTOR_FORWARD_BEAM_COLOR;
	const char* szColours2 = PORTAL_PROPTRACTOR_REVERSED_BEAM_COLOR;
	if (szColours1 != NULL && Q_strlen(szColours1) > 0) {
		sscanf(szColours1, "%i%i%i", &r1, &g1, &b1);
	}
	if (szColours2 != NULL && Q_strlen(szColours2) > 0) {
		sscanf(szColours2, "%i%i%i", &r2, &g2, &b2);
	}

	m_BeamForwardColor.Init(r1, g1, b1);
	m_BeamReversedColor.Init(r2, g2, b2);

}

CPropTractorBeamProjector::~CPropTractorBeamProjector()
{
	CSoundEnvelopeController& controller = CSoundEnvelopeController::GetController();

	if (m_sndMechanical)
	{
		controller.Shutdown(m_sndMechanical);
		controller.SoundDestroy(m_sndMechanical);
	}

	UTIL_Remove(m_hAmbientSoundProxy);
}

void CPropTractorBeamProjector::Spawn()
{
	BaseClass::Spawn();
	Precache();

	if (m_bUse128Model)
		SetModel("models/props_ingame/tractor_beam_128.mdl");
	else
		SetModel("models/props/tractor_beam_emitter.mdl");

	SetSolid(SOLID_VPHYSICS);
	ResetSequence(2);
	UseClientSideAnimation();

	m_hAmbientSoundProxy = ProjectedEntityAmbientSoundProxy::Create(this);

	SetFadeDistance(-1.0, 0.0);
	AddEffects(EF_NOSHADOW);

	if (m_bEnabled)
		CreateSpriteTrails();
}

void CPropTractorBeamProjector::CreateSpriteTrails()
{
	const char* attachments[] = { "emitter1", "emitter2", "emitter3" };

	for (int i = 0; i < ARRAYSIZE(attachments); ++i)
	{
		int nAttachment = LookupAttachment(attachments[i]);

		m_pMainGlow[i] = CSprite::SpriteCreate("particle/particle_flares/tbeam_arm.vmt", GetLocalOrigin(), false);

		if (m_pMainGlow[i] != NULL)
		{
			m_pMainGlow[i]->FollowEntity(this);
			m_pMainGlow[i]->SetAttachment(this, nAttachment);

			if (IsReversed())
				m_pMainGlow[i]->SetTransparency(kRenderTransAdd, m_BeamReversedColor->r, m_BeamReversedColor->g, m_BeamReversedColor->b, 255, kRenderFxNone);
			else
				m_pMainGlow[i]->SetTransparency(kRenderTransAdd, m_BeamForwardColor->r, m_BeamForwardColor->g, m_BeamForwardColor->b, 255, kRenderFxNone);

			m_pMainGlow[i]->SetScale(1.25f);
			m_pMainGlow[i]->SetGlowProxySize(4.0f);
		}

		m_pTrails[i] = CSpriteTrail::SpriteTrailCreate("particle/beam_hotblue_add_oriented.vmt", GetLocalOrigin(), false);

		if (m_pTrails[i] != NULL)
		{
			m_pTrails[i]->FollowEntity(this);
			m_pTrails[i]->SetAttachment(this, nAttachment);

			if (IsReversed())
				m_pTrails[i]->SetTransparency(kRenderTransAdd, m_BeamReversedColor->r, m_BeamReversedColor->g, m_BeamReversedColor->b, 255, kRenderFxNone);
			else
				m_pTrails[i]->SetTransparency(kRenderTransAdd, m_BeamForwardColor->r, m_BeamForwardColor->g, m_BeamForwardColor->b, 255, kRenderFxNone);

			m_pTrails[i]->SetStartWidth(64.0f);
			m_pTrails[i]->SetEndWidth(1.0f);
			m_pTrails[i]->SetLifeTime(0.5f);
		}
	}
}

void CPropTractorBeamProjector::RemoveSpriteTrails()
{
	for (int i = 0; i < 3; ++i)
	{
		if (m_pMainGlow[i] != NULL)
		{
			UTIL_Remove(m_pMainGlow[i]);
			m_pMainGlow[i] = NULL;
		}

		if (m_pTrails[i] != NULL)
		{
			UTIL_Remove(m_pTrails[i]);
			m_pTrails[i] = NULL;
		}
	}
}

void CPropTractorBeamProjector::UpdateSpriteTrails()
{
	RemoveSpriteTrails();
	CreateSpriteTrails();
}

void CPropTractorBeamProjector::Precache()
{
	if (m_bUse128Model)
		PrecacheModel("models/props_ingame/tractor_beam_128.mdl");
	else
		PrecacheModel("models/props/tractor_beam_emitter.mdl");

	PrecacheMaterial("particle/particle_flares/tbeam_arm.vmt");
	PrecacheMaterial("particle/beam_hotblue_add_oriented.vmt");

	PrecacheParticleSystem("tractor_beam_arm");
	PrecacheParticleSystem("tractor_beam_core");
	PrecacheParticleSystem("tractor_beam_reversed_core");
	PrecacheScriptSound("VFX.TbeamEmitterSpinLp");
	PrecacheScriptSound("VFX.TBeamPosPolarity");
	PrecacheScriptSound("VFX.TBeamNegPolarity");

	UTIL_PrecacheOther("trigger_tractorbeam");
}

void CPropTractorBeamProjector::Project(void)
{
	BaseClass::Project();

	m_vEndPos = m_hFirstChild->GetEndPoint();

	CSoundEnvelopeController& controller = CSoundEnvelopeController::GetController();
	if (!m_sndMechanical)
	{
		EmitSound_t ep;
		ep.m_nSpeakerEntity = -1;
		ep.m_hSoundScriptHandle = SOUNDEMITTER_INVALID_HANDLE;
		ep.m_nFlags = 0;
		ep.m_nPitch = 100;
		//memset(&ep.m_pOrigin, 0, 12);
		ep.m_bEmitCloseCaption = 1;
		ep.m_bWarnOnDirectWaveReference = 0;
		//memset(&ep.m_UtlVecSoundOrigin, 0, sizeof(ep.m_UtlVecSoundOrigin));
		//ep.m_nSoundEntryVersion = 1;
		ep.m_nChannel = 6;
		ep.m_pSoundName = "VFX.TbeamEmitterSpinLp";
		ep.m_flVolume = 1.0;
		ep.m_SoundLevel = SNDLVL_NORM;
		ep.m_pOrigin = &GetAbsOrigin();

		CReliableBroadcastRecipientFilter filter;
		filter.AddRecipientsByPAS(GetAbsOrigin());

		m_sndMechanical = controller.SoundCreate(filter, entindex(), ep);

		//CUtlVector<ITriggerTractorBeamAutoList *, CUtlMemory<ITriggerTractorBeamAutoList *, int>>::~CUtlVector<ITriggerTractorBeamAutoList *, CUtlMemory<ITriggerTractorBeamAutoList *, int>>((CUtlVector<__m128, CUtlMemory<__m128, int> > *)&ep.m_UtlVecSoundOrigin);
	}
	controller.Play(m_sndMechanical, 0.1, 100.0);
	controller.SoundChangeVolume(m_sndMechanical, 1.0, 0.75);

	const char* soundName = "VFX.TBeamPosPolarity";//IsReversed() ? "VFX.TBeamNegPolarity" : "VFX.TBeamPosPolarity";

	bool bIsSameSound = false;
	if (m_sndAmbientSound)
	{
		const char* pszAmbientSoundName = controller.SoundGetName(m_sndAmbientSound).ToCStr();
		if (!pszAmbientSoundName)
			pszAmbientSoundName = "";

		bIsSameSound = V_strcmp(pszAmbientSoundName, soundName) != 0;
	}
	if (!m_sndAmbientSound || bIsSameSound)
	{
		if (!m_sndAmbientSound)
		{
			controller.Shutdown(m_sndAmbientSound);
			controller.SoundDestroy(m_sndAmbientSound);
			m_sndAmbientSound = NULL;
		}

		CReliableBroadcastRecipientFilter filter;
		filter.AddAllPlayers();
		filter.MakeReliable();

		m_sndAmbientSound = controller.SoundCreate(filter, m_hAmbientSoundProxy->entindex(), soundName);
		//controller.SoundChangeVolume( m_sndAmbientSound, 0.6, 0.1f );
		//controller.SoundChangePitch( m_sndAmbientSound, 1.5, 0.1f );

		controller.Play(m_sndAmbientSound, 0.8, 150, 0);
	}
}

void CPropTractorBeamProjector::Shutdown(void)
{
	RemoveSpriteTrails();
	BaseClass::Shutdown();

	CSoundEnvelopeController& controller = CSoundEnvelopeController::GetController();

	if (m_sndMechanical)
		controller.SoundFadeOut(m_sndMechanical, 1.5);

	if (m_sndAmbientSound)
	{
		controller.Shutdown(m_sndAmbientSound);
		controller.SoundDestroy(m_sndAmbientSound);
		m_sndAmbientSound = NULL;
	}
}

bool CPropTractorBeamProjector::IsReversed(void)
{
	return m_flLinearForce < 0.0;
}


CBaseProjectedEntity* CPropTractorBeamProjector::CreateNewProjectedEntity(void)
{
	return CProjectedTractorBeamEntity::CreateNewInstance();
}

void CPropTractorBeamProjector::InputSetLinearForce(inputdata_t& inputdata)
{
	m_flLinearForce = inputdata.value.Float();

	if (m_flLinearForce == 0.0)
	{
		EnableProjection(false);
		m_vEndPos = vec3_origin;
	}
	else if (m_bEnabled)
	{
		Project();
	}
	UpdateSpriteTrails();
}
