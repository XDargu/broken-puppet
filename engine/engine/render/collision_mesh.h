#ifndef INC_COLLISION_MESH_H_
#define INC_COLLISION_MESH_H_

#include "items_by_name.h"
#include "../navmesh/navmesh.h"
#include <PxPhysicsAPI.h>


class CDataProvider;

typedef unsigned short TIndex;
typedef unsigned char u8;

class CCollision_Mesh
{
	D3D_PRIMITIVE_TOPOLOGY topology;    // TriangleList, POINTS, LINES...
	const CVertexDecl*     vtxs_decl;   // The type of vertexs
	u8* vertexs;
	TIndex* indices;

	// Just to we don't forget to activate the mesh before rendering
	//static const CCollision_Mesh*    current_active_mesh;

	struct THeader {
		unsigned magic;
		unsigned version;
		unsigned nvertexs;
		unsigned nidxs;
		unsigned primitive_type;
		unsigned bytes_per_vertex;
		unsigned bytes_per_index;
		unsigned vertex_type;
		unsigned magic_tail;

		static const unsigned valid_magic = 0x44776655;
		static const unsigned current_version = 1;

		bool isValid() const {
			return magic == valid_magic
				&& magic_tail == valid_magic
				&& version == current_version
				&& bytes_per_index == 2
				;
		}
	};

	static const unsigned vtxs_magic = 0x44774444;
	static const unsigned idxs_magic = 0x44773333;
	static const unsigned end_magic = 0x44444444;
	struct TChunkHeader {
		unsigned magic;
		unsigned nbytes;
	};

	enum eVertexTypes {
		POSITIONS = 1001
		, POSITION_UV
		, POSITION_COLOR
		, POSITION_UV_NORMAL
	};


public:
	CCollision_Mesh();
	~CCollision_Mesh();

	//puntero a floats necesario para navMeshes
	float* vertex_floats;
	int* index_int;
	unsigned               nvertexs;
	unsigned               nindices;
	unsigned nav_stride_vertex;
	//std::vector<float>v_vertex;
	std::vector<int> v_tris;
	//Collision Mesh de acceso publico----------------------------------
	physx::PxTriangleMesh* collision_mesh;
	//------------------------------------------------------------------

	enum ePrimitiveType {
		POINTS = 4000
		, LINE_LIST
		, TRIANGLE_LIST
		, TRIANGLE_STRIP
	};

	bool create(
		unsigned anvertexs
		, const void* the_vertexs       // Can't be null
		, unsigned anindices
		, const TIndex* the_indices     // Can be null
		, ePrimitiveType primitive_type
		, const CVertexDecl* avtxs_decl
		);

	// -------------------- Create using a vertex type
	template< class TVertex >
	bool create(
		unsigned anvertexs
		, const TVertex* the_vertexs       // Can't be null
		, unsigned anindices
		, const TIndex* the_indices     // Can be null
		, ePrimitiveType primitive_type
		) {
		return create(
			anvertexs
			, the_vertexs       // Can't be null
			, anindices
			, the_indices     // Can be null
			, primitive_type
			, getVertexDecl<TVertex>()
			);
	}

	// Método para recuperar puntero a primera posición de vectores
	std::vector<unsigned char> getTheVertexs();

	// Método para recuperar puntero a primera posición de indices
	std::vector <unsigned short> getTheIndices();

	// Metodo para recuperar numero de vertices
	unsigned getNvertexs();

	// Metodo para recuperar numero de indices
	unsigned getNindices();

	// Metodo para recuperar topologia 
	D3D_PRIMITIVE_TOPOLOGY getTopology();

	//Nav meshes atributos ------------------
	//float* getVertexPointerNav();
	//int* getIndexPointerNav();
	//---------------------------------------

	bool load(CDataProvider& dp);

	void destroy();

	// Leer archivo donde se encuentra almacenado la información del Mesh collider. Necesita punteros al cooking y al Physics de PhysX
	bool load(const char* name);//, PxCooking* mCooking, PxPhysics* gPhysicsSDK);

	void setName(const char*) {}
};

typedef CItemsByName< CCollision_Mesh > CMeshCollisionManager;
extern CMeshCollisionManager mesh_collision_manager;

#endif

