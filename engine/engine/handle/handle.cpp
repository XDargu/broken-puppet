#include "mcv_platform.h"
#include "handle.h"
#include "XMLParser.h"

// -----------------------------------------
bool CHandle::destroy() {
	CHandleManager* hm = CHandleManager::the_register.getByType(type);
	if (hm)
		return hm->destroyObj(*this);
	return false;
}
// -----------------------------------------
CHandle CHandle::clone() const {
	CHandleManager* hm = CHandleManager::the_register.getByType(type);
	if (hm)
		return hm->clone(*this);
	return CHandle();
}
// -----------------------------------------
bool CHandle::isValid() const {
	CHandleManager* hm = CHandleManager::the_register.getByType(type);
	if (hm)
		return hm->isValid(*this);
	return false;
}
// -----------------------------------------
CHandle CHandle::getOwner() const {
	CHandleManager* hm = CHandleManager::the_register.getByType(type);
	if (hm)
		return hm->getOwner(*this);
	return CHandle();
}

bool CHandle::setOwner(CHandle new_owner) {
	CHandleManager* hm = CHandleManager::the_register.getByType(type);
	if (hm)
		return hm->setOwner(*this, new_owner);
	return false;
}

void CHandle::loadFromAtts(const std::string& elem, MKeyValue &atts) {
	CHandleManager* hm = CHandleManager::the_register.getByType(type);
	if (hm)
		hm->loadFromAtts(*this, elem, atts);
}

const char* CHandle::getTypeName() const {
	CHandleManager* hm = CHandleManager::the_register.getByType(type);
	if (hm && isValid())
		return hm->getObjTypeName();
	return "<invalid>";
}

// -----------------------------------------
void CHandleManager::dumpInternals() const {
	dbg(" IDX       Int      Next      Age      External.  NextFree:%d LastFree:%d Used %d/%d\n"
		, next_free_handle_ext_index, last_free_handle_ext_index, size(), capacity());
	for (uint32_t i = 0; i < max_objects_in_use; ++i) {
		const TExternData* ed = external_to_internal + i;
		uint32_t internal_idx = internal_to_external[i];
		dbg("%4d  %08x  %08x %08x      %08x\n"
			, i
			, ed->internal_index
			, ed->next_external_index
			, ed->current_age
			, internal_idx);
	}
}
