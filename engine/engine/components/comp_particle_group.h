#ifndef INC_COMP_PARTICLE_GROUP_H_
#define INC_COMP_PARTICLE_GROUP_H_

#include "base_component.h"
#include "render\render_instances.h"
#include "particles\particle_system.h"

struct TCompParticleGroup : TBaseComponent {
	
	std::vector<TParticleSystem> particle_systems;

public:

	std::string def_name;

	TCompParticleGroup() { def_name = ""; };

	~TCompParticleGroup();

	void loadFromAtts(const std::string& elem, MKeyValue &atts);

	void init();

	void update(float elapsed);

	void render();
	void renderDebug3D() const;

	void clearParticleSystems();

	std::string getXMLDefinition();
	std::string getXMLDefinitionWithName(std::string name);
};

#endif
