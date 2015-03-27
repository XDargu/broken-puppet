#ifndef INC_MESH_H_
#define INC_MESH_H_

#include "items_by_name.h"

class CDataProvider;

class CMesh
{

	ID3D11Buffer*          vb;       // Vertex buffer
	ID3D11Buffer*          ib;       // Index buffer

	unsigned               nvertexs;
	unsigned               nindices;
	D3D_PRIMITIVE_TOPOLOGY topology;    // TriangleList, POINTS, LINES...
	const CVertexDecl*     vtxs_decl;   // The type of vertexs

	// Just to we don't forget to activate the mesh before rendering
	static const CMesh*    current_active_mesh;

	// 
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
		static const unsigned current_version = 2;

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
	static const unsigned groups_magic = 0x44772222;
	static const unsigned end_magic = 0x44444444;
	struct TChunkHeader {
		unsigned magic;
		unsigned nbytes;
	};

	// ------------------------------------
	enum eVertexTypes {
		POSITIONS = 1001
		, POSITION_UV
		, POSITION_COLOR
		, POSITION_UV_NORMAL
		, POSITION_UV_NORMAL_SKIN
	};

	// ------------------------------------
	struct TGroup {
		int   first_index;
		int   num_indices;
	};
	typedef std::vector< TGroup > VGroups;
	VGroups groups;

	friend class CCoreModel;

	// ------------------------------------
public:
	CMesh();
	~CMesh();

	typedef unsigned short TIndex;

	enum ePrimitiveType {
		POINTS = 4000
		, LINE_LIST
		, TRIANGLE_LIST
		, TRIANGLE_STRIP
		, LINE_LIST_ADJ
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

	void destroy();

	void activate() const;
	void render() const;
	void renderGroup(int group_id) const;
	size_t getNGroups() const { return groups.size(); }

	void activateAndRender() const {
		activate();
		render();
	}

	bool load(CDataProvider& dp);
	bool load(const char* name);
	void setName(const char*) {}
};

typedef CItemsByName< CMesh > CMeshManager;
extern CMeshManager mesh_manager;

#endif
