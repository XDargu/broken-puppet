#ifndef INC_ITEMS_BY_NAME_H_
#define INC_ITEMS_BY_NAME_H_

template < class TItem >
class CItemsByName {

protected:

	typedef std::map< std::string, TItem* > MItems;
	MItems all_items;

public:

	void destroyAll() {
		for (auto it : all_items) {
			it.second->destroy();
			delete it.second;
		}
		all_items.clear();
	}

	const TItem* getByName(const char* name) {

		// Search ...
		auto it = all_items.find(name);

		// If found, return value
		if (it != all_items.end())
			return it->second;

		// create new obj
		TItem *t = new TItem;
		bool is_ok = t->load(name);
		//assert(is_ok);
		if (is_ok) {

			registerNew(name, t);
			return t;
		}
		else
			return nullptr;
	}

	bool registerNew(const char* name, TItem *t) {
		// Confirm it does NOT exists
		auto it = all_items.find(name);
		if (it != all_items.end())
			return false;

		// So, register it
		t->setName(name);
		all_items[name] = t;
		return true;
	}

	void unregister(const char* name) {
		auto it = all_items.find(name);
		if (it != all_items.end()) {
			all_items.erase(it);
		}
	}

	bool reload(const char* name) {

		// Search ...
		auto it = all_items.find(name);

		// If not found, return false
		if (it == all_items.end()) {
			dbg("Can't reload item %s\n", name);
			return false;
		}

		return it->second->reload();
	}

};

#endif
