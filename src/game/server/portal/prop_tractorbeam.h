#include "baseprojector.h"
#include "Sprite.h"
#include "SpriteTrail.h"

class ProjectedEntityAmbientSoundProxy : public CBaseEntity
{
public:

	DECLARE_CLASS(ProjectedEntityAmbientSoundProxy, CBaseEntity);
	DECLARE_SERVERCLASS();
	ProjectedEntityAmbientSoundProxy();
	~ProjectedEntityAmbientSoundProxy();

	static ProjectedEntityAmbientSoundProxy* Create(CBaseEntity* pAttachTo);

	int UpdateTransmitState();
	void AttachToEntity(CBaseEntity* pAttachTo);
};

class CPropTractorBeamProjector : public CBaseProjector
{
public:

	DECLARE_CLASS(CPropTractorBeamProjector, CBaseProjector);
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

	CPropTractorBeamProjector();
	~CPropTractorBeamProjector();
	void Spawn();
	void Precache();
	void Project();

	void CreateSpriteTrails();
	void UpdateSpriteTrails();

	float GetLinearForce() { return m_flLinearForce; }

	bool IsReversed();

	CNetworkColor32(m_BeamForwardColor);
	CNetworkColor32(m_BeamReversedColor);

protected:

	virtual CBaseProjectedEntity* CreateNewProjectedEntity();

	void InputSetLinearForce(inputdata_t& inputdata);

	void Shutdown();

	CNetworkVar(float, m_flLinearForce);

	CNetworkVector(m_vEndPos);

	CNetworkVar(bool, m_bNoEmitterParticles);
	CNetworkVar(bool, bDisableAutoReprojection);

	bool m_bUse128Model;

private:

	CHandle<CSprite> m_pMainGlow[3];
	CHandle<CSpriteTrail> m_pTrails[3];

	CSoundPatch* m_sndMechanical;
	CSoundPatch* m_sndAmbientSound;

	CHandle<ProjectedEntityAmbientSoundProxy> m_hAmbientSoundProxy;
};