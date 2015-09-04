#ifndef INC_ENTITY_H_
#define INC_ENTITY_H_

// ----------------------------------------
class CEntity {
	CHandle components[CHandle::max_types];
public:
	char tag[32];
	char material_tag[32];
	char collision_tag[32];
	~CEntity() {
		for (uint32_t i = 0; i < CHandle::max_types; i++)
			components[i].destroy();
	}

	CEntity() { }
	CEntity(const CEntity& e);
	//CEntity operator=( const CEntity& e );

	// Add a handle to this entity
	CHandle add(CHandle h) {
		assert(h.isValid() || fatal("Handle must be valid\n"));
		uint32_t h_type = h.getType();
		assert(!components[h_type].isValid() || fatal("Entity has already a handle of type %d\n", h.getType()));
		assert(!h.getOwner().isValid() || fatal("Handle %08x has already an owner: %08x\n", h.asUnsigned(), h.getOwner().asUnsigned()));
		components[h_type] = h;
		h.setOwner(CHandle(this));
		assert(h.getOwner() == CHandle(this));
		return h;
	}

	// Return true if the entity has a valid component of that type
	template< class TObj>
	bool has() const {
		return get<TObj>().isValid();
	}

	// Get the index given the obj type, and return whatever handle we
	// are storing now
	template< class TObj >
	CHandle get() const {
		uint32_t h_type = getObjManager<TObj>()->getType();
		return components[h_type];
	}

	CHandle getByType(uint32_t atype) const {
		return components[atype];
	}

	// Destroy the handle of that type
	template< class TObj >
	bool del() {
		uint32_t h_type = getObjManager<TObj>()->getType();
		return components[h_type].destroy();
	}

	// Helpers
	const char* getName() const;
	void loadFromAtts(const std::string& elem, MKeyValue& atts) {
		std::strcpy(tag, atts.getString("tag", "untagged").c_str());
		std::strcpy(collision_tag, atts.getString("collisionTag", "untagged").c_str());
		std::strcpy(material_tag, atts.getString("materialTag", "untagged").c_str());
	}

  // Mandar un msg a todos los componentes de esta entidad que se 
  // hayan registrado a dicho mensage
	template <class TMsgType>
	void sendMsg(const TMsgType& data) {
		TMsgID msg_id = TMsgType::getID();
		// Buscar todos los subscripciones a este tipo de msg
		auto it = msg_subscriptions.find(msg_id);
		while (it != msg_subscriptions.end() && it->first == msg_id) {

			// Si la entidid tiene este component
			CHandle comp = components[it->second.comp_type];

			// Mandarle el msg
			if (comp.isValid())
				it->second.method->execute(comp, &data);

			++it;
		}
	}

	void destroyComponents() {
		for (uint32_t i = 0; i < CHandle::max_types; i++)
			components[i].destroy();
	}

	bool hasTag(char* the_tag) {
		return std::strcmp(tag, the_tag) == 0;
	}

	bool hasCollisionTag(char* the_tag) {
		return std::strcmp(collision_tag, the_tag) == 0;
	}
	bool hasMaterialTag(char* the_tag) {
		return std::strcmp(material_tag, the_tag) == 0;
	}

	/*
	template <class TMsgType>
	void postMsg(const TMsgType& data) {
		// Save in a global buffer, the handle, sizeof( data ), data
		// and return

		// in a safe place ( after all CObjManager<>::udpate )
		// process the queue, reading handle/nbytes,data_msg and dispatching
		// the msg. At that moment handle could be invalid
	}

	*/
};

#endif
