#ifndef INC_SKELETON_MANAGER_H_
#define INC_SKELETON_MANAGER_H_

#include "items_by_name.h"
#include "cal3d/cal3d.h"
#include "XMLParser.h"

// Mas otras cosas que cal3d no tiene
class CCoreModel : public CalCoreModel, public CXMLParser {
  std::string               root_path;
  std::string               name;
  void onStartElement(const std::string &elem, MKeyValue &atts);

  const CMesh*              mesh;

  bool convertCMF(int mesh_id, const char* outfile);

public:
  CCoreModel();
  bool load(const char* name);
  void setName(const char *new_name) {
    name = new_name;
  }
  const std::string& getName() const { return name; }
  const CMesh* getMesh() const { return mesh; }

  struct TBoneCorrector {
    int         bone_id;
    CalVector   local_dir;
    float       local_amount;
    bool        render;
    void apply( CalModel* model, CalVector target, float amount );
	TBoneCorrector() {}
	TBoneCorrector(int the_bone_id, CalVector the_local_dir) {
		bone_id = the_bone_id;
		local_dir = the_local_dir;
	}
  };

  typedef std::vector <TBoneCorrector> VBoneCorrections;
  VBoneCorrections bone_corrections;

  typedef std::vector <int> VBoneIds;
  VBoneIds         bone_ids_to_debug;

  typedef std::vector <float> VAnimBlendTimes;
  VAnimBlendTimes  animation_blend_times;
};

typedef CItemsByName< CCoreModel > CSkeletonManager;
extern CSkeletonManager skeleton_manager;

XMVECTOR Cal2DX(CalVector q);
CalVector DX2Cal(XMVECTOR v);
CalQuaternion DX2CalQuat(XMVECTOR v);
XMVECTOR Cal2DX(CalQuaternion q);

#endif
