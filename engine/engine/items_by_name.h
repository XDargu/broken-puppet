#ifndef INC_ITEMS_BY_NAME_H_
#define INC_ITEMS_BY_NAME_H_

template < class TItem >
class CItemsByName {

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
    t->load(name);
    t->setName(name);

    all_items[name] = t;
    return t;
  }


};

#endif
