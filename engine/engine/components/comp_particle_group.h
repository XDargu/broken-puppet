#ifndef INC_COMP_PARTICLE_GROUP_H_
#define INC_COMP_PARTICLE_GROUP_H_

#include "base_component.h"
#include "render\render_instances.h"
#include "particles\particle_system.h"

struct TCompParticleGroup : TBaseComponent {
	
	std::vector<TParticleSystem> particle_systems;
	std::vector<TParticleSystem> particle_systems_to_destroy;

public:

	bool destroy_on_death;
	std::string def_name;

	TCompParticleGroup() { def_name = ""; destroy_on_death = false; };

	~TCompParticleGroup();

	void loadFromAtts(const std::string& elem, MKeyValue &atts);

	void init();

	void update(float elapsed);
	void fixedUpdate(float elapsed);

	void render();
	void renderDebug3D() const;

	void removeParticleSystem(TParticleSystem* ps);
	void clearParticleSystems();

	void restart();

	std::string getXMLDefinition();
	std::string getXMLDefinitionWithName(std::string name);
};

#endif
