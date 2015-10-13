#ifndef INC_COMP_PARTICLE_GROUP_H_
#define INC_COMP_PARTICLE_GROUP_H_

#include "base_component.h"
#include "render\render_instances.h"
#include "particles\particle_system.h"

struct TCompParticleGroup : TBaseComponent {
	CHandle h_transform;
	CHandle h_player_trans;
	std::vector<TParticleSystem>* particle_systems;
public:
	enum flag{
		UNDEFINED,
		IMPACT
	};
	flag kind;
public:

	bool destroy_on_death;
	char def_name[128];

	TCompParticleGroup() { def_name[0] = 0x00; destroy_on_death = false; particle_systems = new std::vector < TParticleSystem >; kind = UNDEFINED; };

	~TCompParticleGroup();

	void loadFromAtts(const std::string& elem, MKeyValue &atts);

	void init();

	void update(float elapsed);
	void fixedUpdate(float elapsed);

	void render();
	void renderDistorsion();
	void renderDebug3D() const;

	void removeParticleSystem(TParticleSystem* ps);
	void clearParticleSystems();

	void restart();

	std::string getXMLDefinition();
	std::string getXMLDefinitionWithName(std::string name);
};

#endif
