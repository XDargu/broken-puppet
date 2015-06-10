#include "mcv_platform.h"
#include "entity_manager.h"
#include "entity_inspector.h"
#include "comp_transform.h"
#include "comp_anim_editor.h"

void TCompAnimEditor::init() {
#ifdef _DEBUG

	// Hide console, listener and actioner bars
	TwDefine(" Lister visible=false ");
	TwDefine(" Actioner visible=false ");
	TwDefine(" Console visible=false ");
	TwDefine(" ConsoleInput visible=false ");

	// Inspect the player
	CHandle player = CEntityManager::get().getByName("Player");
	CEntityInspector::get().inspectEntity(player);

#endif
}

void TCompAnimEditor::update(float elapsed) {
#ifdef _DEBUG
	// Hide console, listener and actioner bars
	TwDefine(" Lister visible=false ");
	TwDefine(" Actioner visible=false ");
	TwDefine(" Console visible=false ");
	TwDefine(" ConsoleInput visible=false ");
	TwDefine(" PostProcessOptioner visible=false ");
	TwDefine(" DebugOptioner visible=false ");

	CHandle player = CEntityManager::get().getByName("Player");
	if (player.isValid()) {
		CHandle ie = CEntityInspector::get().getInspectedEntity();
		
		if (!(ie == player))
			CEntityInspector::get().inspectEntity(player);
	}

#endif
}
