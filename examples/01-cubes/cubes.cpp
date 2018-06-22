/*
 * Copyright 2011-2018 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */
#include <random>

#include "common.h"
#include "bgfx_utils.h"
#include "imgui/imgui.h"

#include "sphere.hpp"
#include "camera.h"

BX_PRAGMA_DIAGNOSTIC_PUSH()

BX_PRAGMA_DIAGNOSTIC_IGNORED_CLANG_GCC("-Wunused-variable")
BX_PRAGMA_DIAGNOSTIC_IGNORED_CLANG_GCC("-Wmissing-field-initializers")
BX_PRAGMA_DIAGNOSTIC_IGNORED_CLANG_GCC("-Wshift-negative-value")
BX_PRAGMA_DIAGNOSTIC_IGNORED_CLANG_GCC("-Wunused-parameter")
BX_PRAGMA_DIAGNOSTIC_IGNORED_CLANG_GCC("-Wextra")
BX_PRAGMA_DIAGNOSTIC_IGNORED_CLANG_GCC("-Wshadow")
BX_PRAGMA_DIAGNOSTIC_IGNORED_CLANG_GCC("-Wextra")
BX_PRAGMA_DIAGNOSTIC_IGNORED_CLANG_GCC("-Waggressive-loop-optimizations")
BX_PRAGMA_DIAGNOSTIC_IGNORED_CLANG_GCC("-Wmaybe-uninitialized")

BX_PRAGMA_DIAGNOSTIC_IGNORED_MSVC(4244)
BX_PRAGMA_DIAGNOSTIC_IGNORED_MSVC(4127)
BX_PRAGMA_DIAGNOSTIC_IGNORED_MSVC(4189)
BX_PRAGMA_DIAGNOSTIC_IGNORED_MSVC(4310)
BX_PRAGMA_DIAGNOSTIC_IGNORED_MSVC(4100)
BX_PRAGMA_DIAGNOSTIC_IGNORED_MSVC(4456)
BX_PRAGMA_DIAGNOSTIC_IGNORED_MSVC(4267)

#include "tinyformat.h"


#define STB_VOXEL_RENDER_IMPLEMENTATION 1
#define STBVOX_CONFIG_MODE 20
//         Mode value       Meaning
//             0               Textured blocks, 32-byte quads
//             1               Textured blocks, 20-byte quads
//            20               Untextured blocks, 32-byte quads
//            21               Untextured blocks, 20-byte quads

#include "stb_voxel_render.h"


BX_PRAGMA_DIAGNOSTIC_POP()

#include <tuple>

#define uint unsigned

namespace
{
template <typename T>
const bgfx::Memory* copyFromVector(T& vec) {
	return bgfx::copy(&vec[0], sizeof(vec[0])*((uint32_t)vec.size()));
}

constexpr uint32_t kSizeMesh{3};

constexpr uint32_t kSizeMeshPlus2 = kSizeMesh + 2;


inline uint32_t toUnorm(float _value)
{
	return uint32_t(bx::round(bx::clamp(_value, 0.0f, 255.0f) ) );
}


// RGBA8
inline void packRgba8(void* _dst, const uint8_t* _src)
{
	uint8_t* dst = (uint8_t*)_dst;
	dst[0] = (_src[0]);
	dst[1] = (_src[1]) ;
	dst[2] = (_src[2]) ;
	dst[3] = (_src[3]);
}


///
inline uint32_t encodeRgba8(uint8_t _x, uint8_t _y = 0, uint8_t _z = 0, uint8_t _w = 0)
{
	const uint8_t src[] =
	{
		_x ,
		_y,
		_z,
		_w ,
	};
	uint32_t dst;
	packRgba8(&dst, src);
	return dst;
}

constexpr std::int32_t ceil(float num) {
    std::int32_t inum = static_cast<std::int32_t>(num);
    if (num == static_cast<float>(inum)) {
        return inum;
    }
    return inum + (num > 0 ? 1 : 0);
}

constexpr std::uint8_t ceil255(float num) {
	return uint8_t(ceil(bx::clamp(num, 0.f, 1.f)*255.f));
}

// pack 3 floats in 0.0 .. 1.0 range into 32 bits
constexpr uint32_t packRgb8f(float r, float g, float b) {
	uint8_t r8 = ceil255(r);
	uint8_t g8 = ceil255(g);
	uint8_t b8 = ceil255(b);
	return (uint32_t) r8 + (g8 << 8) + (b8 << 16);
}

constexpr uint32_t packRgb8(uint8_t x, uint8_t y, uint8_t z) {
	return (uint32_t)(x + ((uint32_t)y << 8) + ((uint32_t)z << 16));
}


constexpr uint32_t packRgb8_255(uint8_t x, uint8_t y, uint8_t z) {
	return (uint32_t)((uint32_t)x*255  + (((uint32_t)y*255) << 8) + (((uint32_t)z*255) << 16));
}

// Plane 2 with packed data

struct PosColorVertexPacked
{
	/*
	float    m_x;
	float    m_y;
	float    m_z;
	*/
	uint32_t pos_;
	uint32_t color;

	static void init()
	{
		ms_decl.begin()
			.add(bgfx::Attrib::Position, 4, bgfx::AttribType::Uint8, false, true)
			// .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
//			.add(bgfx::Attrib::Color2, 4, bgfx::AttribType::Uint8, false, true)
			.add(bgfx::Attrib::Color1, 4, bgfx::AttribType::Uint8, false, true)
			.end();
	}
	static bgfx::VertexDecl ms_decl;
};

bgfx::VertexDecl PosColorVertexPacked::ms_decl;

/*
constexpr PosColorVertexPacked encode_vertex99(int8_t x,int8_t y,int8_t z,int8_t r,int8_t g,int8_t b) {
	return {stbvox_vertex_encode((x), (y), (z), 0, 0), packRgb8(r,g,b)};
}
*/

/*
constexpr PosColorVertexPacked encode_vertex1(int8_t x,int8_t y,int8_t z,int8_t r,int8_t g,int8_t b) {
	return { packRgb8(x,y,z), packRgb8f(r,g,b)};
}


constexpr PosColorVertexPacked encode_vertex(uint8_t x, uint8_t y, uint8_t z,uint8_t r,uint8_t g,uint8_t b) {
	return { float(x)*0.5f, float(y)*0.5f, float(z)*0.5f, packRgb8(r,g,b)};
}
*/

constexpr uint32_t stb_encode32(uint8_t x, uint8_t y, uint8_t z) {
	return x + (y << 7) + (z << 14);
}

constexpr PosColorVertexPacked encode_vertex(uint8_t x, uint8_t y, uint8_t z,uint8_t r,uint8_t g,uint8_t b) {
	return { stb_encode32(x,y,z), packRgb8(r,g,b)};
}

static PosColorVertexPacked s_voxelVerts[] = 
{
	encode_vertex(0, 0, 1, 100, 55, 250),
	encode_vertex(1, 0, 1, 110, 55, 250),
	encode_vertex(1, 1, 1, 110, 55, 250),
	encode_vertex(0, 1, 1, 110, 55, 250)
};


static const uint16_t s_voxelIndices[] =
{
	0, 1, 2,
	0, 3, 2,
};

constexpr uint32_t tmp1x = packRgb8f(0.f,1.f,7.f);
constexpr uint32_t tmpx = packRgb8f(1, 0,0);
constexpr PosColorVertexPacked tmp2x = encode_vertex(1, 1, 0, 1, 0,0);


// Plane
struct PosNormalVertex
{
	float    m_x;
	float    m_y;
	float    m_z;
	uint32_t m_normal;

	static void init()
	{
		ms_decl
			.begin()
			.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
			.add(bgfx::Attrib::Normal, 4, bgfx::AttribType::Uint8, true, true)
			.end();
	};

	static bgfx::VertexDecl ms_decl;
};

bgfx::VertexDecl PosNormalVertex::ms_decl;

static PosNormalVertex s_hplaneVertices[] =
{
	{ -1.0f, 0.0f,  1.0f, encodeNormalRgba8(0.0f, 1.0f, 0.0f) },
	{ 1.0f, 0.0f,  1.0f, encodeNormalRgba8(0.0f, 1.0f, 0.0f) },
	{ -1.0f, 0.0f, -1.0f, encodeNormalRgba8(0.0f, 1.0f, 0.0f) },
	{ 1.0f, 0.0f, -1.0f, encodeNormalRgba8(0.0f, 1.0f, 0.0f) },
};

static const uint16_t s_planeIndices[] =
{
	0, 1, 2,
	1, 3, 2,
};


struct PositionOnlyVertex {
	float x;
	float y;
	float z;

	

	static void init() {
		ms_decl.begin()
			.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
			.end();
	};

	static bgfx::VertexDecl ms_decl;
};

bgfx::VertexDecl PositionOnlyVertex::ms_decl;

// #define NOSPHERE
#ifdef NOSPHERE
struct PosColorVertex
{
	float m_x;
	float m_y;
	float m_z;
	uint32_t m_abgr;

	static void init()
	{
		ms_decl
			.begin()
			.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
			.add(bgfx::Attrib::Color0,   4, bgfx::AttribType::Uint8, true)
			.end();
	};

	static bgfx::VertexDecl ms_decl;
};

bgfx::VertexDecl PosColorVertex::ms_decl;

static PosColorVertex s_cubeVertices[] =
{
	{-1.0f,  1.0f,  1.0f, 0xff000000 },
	{ 1.0f,  1.0f,  1.0f, 0xff0000ff },
	{-1.0f, -1.0f,  1.0f, 0xff00ff00 },
	{ 1.0f, -1.0f,  1.0f, 0xff00ffff },
	{-1.0f,  1.0f, -1.0f, 0xffff0000 },
	{ 1.0f,  1.0f, -1.0f, 0xffff00ff },
	{-1.0f, -1.0f, -1.0f, 0xffffff00 },
	{ 1.0f, -1.0f, -1.0f, 0xffffffff },
};

static const uint16_t s_cubeTriList[] =
{
	0, 1, 2, // 0
	1, 3, 2,
	4, 6, 5, // 2
	5, 6, 7,
	0, 2, 4, // 4
	4, 2, 6,
	1, 5, 3, // 6
	5, 7, 3,
	0, 4, 1, // 8
	4, 5, 1,
	2, 3, 6, // 10
	6, 3, 7,
};

static const uint16_t s_cubeTriStrip[] =
{
	0, 1, 2,
	3,
	7,
	1,
	5,
	0,
	4,
	2,
	6,
	7,
	4,
	5,
};

#endif


class ExampleCubes : public entry::AppI
{
public:
	ExampleCubes(const char* _name, const char* _description)
		: entry::AppI(_name, _description)
		, m_r(true)
		, m_g(true)
		, m_b(true)
		, m_a(true)
	{
	}

	void init(int32_t _argc, const char* const* _argv, uint32_t _width, uint32_t _height) override
	{
		
		auto szvp = sizeof(PosColorVertexPacked);
		tfm::printf("Size of PosColorVertexPacked: %d", szvp);

		auto sphere_ = make_icosphere(1);
		tfm::printf("No vertices: %d\n", sphere_.first.size());
		auto sphereVert = sphere_.first;
		auto sphereInd = sphere_.second;

		//tfm::printf("%d", s_voxelVerts[0].pos_);
		//bx::packRgb8
		tfm::printf("%d  %d\n", s_voxelVerts[0].color, s_voxelVerts[1].color);

		

		Args args(_argc, _argv);

		m_width  = _width;
		m_height = _height;
		m_debug  = BGFX_DEBUG_NONE;
		m_reset  = BGFX_RESET_VSYNC;

		bgfx::Init init;
		init.type     = args.m_type;
		init.vendorId = args.m_pciId;
		init.resolution.width  = m_width;
		init.resolution.height = m_height;
		init.resolution.reset  = m_reset;
		bgfx::init(init);

		// Enable debug text.
		bgfx::setDebug(m_debug);

		// Set view 0 clear state.
		bgfx::setViewClear(0
				, BGFX_CLEAR_COLOR|BGFX_CLEAR_DEPTH
				, 0x303030ff
				, 1.0f
				, 0
				);


#ifdef NOSPHERE
BX_UNUSED(s_cubeTriList, s_cubeTriStrip);
		// Create vertex stream declaration.
		PosColorVertex::init();

		// Create static vertex buffer.
		m_vbh = bgfx::createVertexBuffer(
				// Static data can be passed with bgfx::makeRef
				bgfx::makeRef(s_cubeVertices, sizeof(s_cubeVertices) )
				, PosColorVertex::ms_decl
				);

		// Create static index buffer.
		m_ibh = bgfx::createIndexBuffer(
				// Static data can be passed with bgfx::makeRef
				bgfx::makeRef(s_cubeTriStrip, sizeof(s_cubeTriStrip) )
				);
#else

		//bgfx::copy 
		PositionOnlyVertex::init();
		m_vbh = bgfx::createVertexBuffer(
			//bgfx::copy(&sphereVert[0], sizeof(sphereVert[0])*((uint32_t)sphereVert.size())),
			copyFromVector(sphereVert),
			PositionOnlyVertex::ms_decl
		);

		m_ibh = bgfx::createIndexBuffer(
			bgfx::copy(&sphereInd[0], sizeof(sphereInd[0])* ((uint32_t)sphereInd.size()))
		);
#endif
		// Create program from shaders.
		m_program = loadProgram("vs_cubes", "fs_cubes");
		m_program2 = loadProgram("vs_plane", "fs_cubes");



		// plane
		PosNormalVertex::init();



		m_vbh2 = bgfx::createVertexBuffer(
			bgfx::makeRef(s_hplaneVertices, sizeof(s_hplaneVertices))
			, PosNormalVertex::ms_decl
		);

		m_ibh2 = bgfx::createIndexBuffer(
			bgfx::makeRef(s_planeIndices, sizeof(s_planeIndices))
		);


		/// 33
		PosColorVertexPacked::init();

		/*
		
		
		vbh_voxel_ = bgfx::createVertexBuffer(
			bgfx::makeRef(s_voxelVerts, sizeof(s_voxelVerts)),
			PosColorVertexPacked::ms_decl);
		ibh_voxel_ = bgfx::createIndexBuffer(
			bgfx::makeRef(s_voxelIndices, sizeof(s_voxelIndices)));
		program_voxel_ = loadProgram("vs_voxel", "fs_cubes");
		*/

		m_timeOffset = bx::getHPCounter();

		imguiCreate();


		// Voxels start
		stbvox_init_mesh_maker(&mesh_maker_);

		stbvox_set_input_stride(&mesh_maker_, kSizeMeshPlus2*kSizeMeshPlus2, kSizeMeshPlus2);

		buffer_size_ = 1024*32*8;
		mesh_buffer_ = malloc(buffer_size_);
		memset(mesh_buffer_, 0, buffer_size_);
		//uint32_t* mb = (uint32_t*)mesh_buffer_;
		//assert(mb);
		assert(sizeof(stbvox_mesh_vertex) == sizeof(uint32_t));
		stbvox_set_buffer(&mesh_maker_, 0, 0, mesh_buffer_, buffer_size_);

		auto buf_count = stbvox_get_buffer_count(&mesh_maker_);
		tfm::printf("%s\n", buf_count);
		printf("----------------------------------\n");

		stbvox_input_description* input_descr = stbvox_get_input_description(&mesh_maker_);
		tfm::printf("%s\n", input_descr);

		#ifdef USE_BLOCK_TYPES
		constexpr unsigned numBlockTypes = 256;

		
		unsigned char colorForBlockType[numBlockTypes];
		unsigned char geomForBlockType[numBlockTypes];

		for (unsigned i = 0; i < numBlockTypes; i++) {
			colorForBlockType[i] = 0;
			geomForBlockType[i] = STBVOX_GEOM_empty;
		}
		// 0..63
		colorForBlockType[1] = 63;
		geomForBlockType[1] = STBVOX_GEOM_solid;
		#endif

		// Generate sphere
		sphere(9);

		// Stb voxel needs to access -1 voxels
		input_descr->blocktype = &blocktype_[1][1][1];
		input_descr->rgb = &rgb_[1][1][1]; 
		input_descr->lighting = &lighting_[1][1][1];

		#ifdef USE_BLOCK_TYPES
		// These are indiced by block type
		input_descr->block_geometry = geomForBlockType;
		input_descr->blockze_color = colorForBlockType;
		#endif

		stbvox_set_input_range(&mesh_maker_, 0, 0, 0, kSizeMesh, kSizeMesh, kSizeMesh);
		
		//tfm::printf("rgb: %d\n", rgb[2][4][kSizeMesh-1].b);

		int size_per_quad = stbvox_get_buffer_size_per_quad(&mesh_maker_, 0);
		tfm::printf("size per quad: %d\n", size_per_quad);

		int res = stbvox_make_mesh(&mesh_maker_);
		tfm::printf("\nresult%d\n", res);
		
		uint32_t vertex_data_sz = (uint32_t) ((std::ptrdiff_t)mesh_maker_.output_cur[0][0] - (std::ptrdiff_t)mesh_maker_.output_buffer[0][0]);
		auto num_faces = vertex_data_sz / 32;
		tfm::printf("Total number faces: %d\n", num_faces);

		
		uint32_t* voxel_vertex_data = (uint32_t*) mesh_maker_.output_buffer[0][0];
		uint32_t num_vertices = num_faces * 4;

		uint32_t num_indices = num_faces*6;
		uint16_t* indices = new uint16_t[num_indices];

		for (uint16_t i = 0; i < num_faces; ++i) {
			indices[i*6 + 0] = 0 + i*4;
			indices[i*6 + 1] = 1 + i*4;
			indices[i*6 + 2] = 2 + i*4;
			indices[i*6 + 3] = 3 + i*4;
			indices[i*6 + 4] = 0 + i*4;
			indices[i*6 + 5] = 2 + i*4;
		}

		vbh_voxel_ = bgfx::createVertexBuffer(
			bgfx::copy(voxel_vertex_data, 8*num_vertices),
			PosColorVertexPacked::ms_decl);
		ibh_voxel_ = bgfx::createIndexBuffer(
			bgfx::copy(indices, sizeof(uint16_t)*num_indices));

		delete[] indices;

		program_voxel_ = loadProgram("vs_voxel", "fs_voxel");

		uNormals_[0] = bgfx::createUniform("u_normal_table_0", bgfx::UniformType::Mat3);
		uNormals_[1] = bgfx::createUniform("u_normal_table_1", bgfx::UniformType::Mat3);
		memcpy(&normals_[0], &stbvox_default_normals[0], sizeof(float)*9);
		memcpy(&normals_[1], &stbvox_default_normals[3], sizeof(float)*9);

		uAmbient_ = bgfx::createUniform("u_ambient", bgfx::UniformType::Mat4);

		// Init camera
		cameraCreate();
		// float camPos[] = { 0.0f, 1.5f, 0.0f };
		float eye[3] = { 0.0f, 3.0f, -6.0f };
//		float eye[3] = { 0.0f, 30.0f, -60.0f };
		// float at[3] = { 0.0f,  5.0f,   0.0f };
		cameraSetPosition(eye);
		cameraSetVerticalAngle(-0.3f);
		
	}



	void sphere(uint radius) {
		std::random_device rd;  //Will be used to obtain a seed for the random number engine
		std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
		std::uniform_int_distribution<short> dis(100, 255);

		int counter{ 0 };
		constexpr uint c = kSizeMesh / 2;
		auto rsq = radius*radius;
		for (uint x = 0; x < kSizeMesh; x++) {
			auto x2 = (x-c) * (x-c);
			for (uint y = 0; y < kSizeMesh; y++) {
				auto y2 = (y-c)*(y-c);
				for (uint z = 0; z < kSizeMesh; z++) {
					auto z2 = (z-c)*(z-c);
					if (x2 + y2 + z2 <= rsq) {
						set_voxel_color(std::make_tuple(x + 1, y + 1, z + 1), { 200, 20, 100 });
						counter++;
					}
//					lighting_[x + 1][y + 1][z + 1] = (uint8_t) dis(gen);
					lighting_[x + 1][y + 1][z + 1] = (uint8_t)200;
				}
			}
		}
		tfm::printf("Num of voxels: %d\n", counter);
		
	}
	

	void set_voxel_color(std::tuple<uint, uint, uint> pos, stbvox_rgb color) {
		using namespace std;
		uint x, y, z;
		tie(x, y, z) = pos;
		rgb_[x][y][z] = color;
		blocktype_[x][y][z] = 1;
	// blocktype_[x][y][z] = STBVOX_GEOM_solid;
		lighting_[x][y][z] = 100;
	}

	virtual int shutdown() override
	{
		imguiDestroy();

		// Cleanup.
		bgfx::destroy(m_ibh);
		bgfx::destroy(m_vbh);
		bgfx::destroy(m_program);

		// Shutdown bgfx.
		bgfx::shutdown();

		return 0;
	}

	bool update() override
	{
		if (!entry::processEvents(m_width, m_height, m_debug, m_reset, &m_mouseState) )
		{
			// Update frame timer
			int64_t now = bx::getHPCounter();
			static int64_t last = now;
			const int64_t frameTime = now - last;
			last = now;
			const double freq = double(bx::getHPFrequency());
			const float deltaTime = float(frameTime / freq);

			// Update camera
			cameraUpdate(deltaTime*0.15f, m_mouseState);

			float view[16];
			cameraGetViewMtx(view);

			imguiBeginFrame(m_mouseState.m_mx
				,  m_mouseState.m_my
				, (m_mouseState.m_buttons[entry::MouseButton::Left  ] ? IMGUI_MBUT_LEFT   : 0)
				| (m_mouseState.m_buttons[entry::MouseButton::Right ] ? IMGUI_MBUT_RIGHT  : 0)
				| (m_mouseState.m_buttons[entry::MouseButton::Middle] ? IMGUI_MBUT_MIDDLE : 0)
				,  m_mouseState.m_mz
				, uint16_t(m_width)
				, uint16_t(m_height)
				);

			showExampleDialog(this);

			ImGui::SetNextWindowPos(
				  ImVec2(m_width - m_width / 5.0f - 10.0f, 10.0f)
				, ImGuiCond_FirstUseEver
				);
			ImGui::SetNextWindowSize(
				  ImVec2(m_width / 5.0f, m_height / 4.0f)
				, ImGuiCond_FirstUseEver
				);
			ImGui::Begin("Settings"
				, NULL
				, 0
				);

			ImGui::Checkbox("Write R", &m_r);
			ImGui::Checkbox("Write G", &m_g);
			ImGui::Checkbox("Write B", &m_b);
			ImGui::Checkbox("Write A", &m_a);

			ImGui::End();

			imguiEndFrame();

			float time = (float)( (bx::getHPCounter()-m_timeOffset)/double(bx::getHPFrequency() ) );

			//float at[3]  = { 0.0f, 0.0f,   0.0f };
			//float eye[3] = { 0.0f, 0.0f, -35.0f };

			float eye[3] = { 0.0f, 30.0f, -60.0f };
			// float at[3] = { 0.0f,  5.0f,   0.0f };

			// Set view and projection matrix for view 0.
			const bgfx::HMD* hmd = bgfx::getHMD();
			if (NULL != hmd && 0 != (hmd->flags & BGFX_HMD_RENDERING) )
			{
				bx::mtxQuatTranslationHMD(view, hmd->eye[0].rotation, eye);
				bgfx::setViewTransform(0, view, hmd->eye[0].projection, BGFX_VIEW_STEREO, hmd->eye[1].projection);

				// Set view 0 default viewport.
				//
				// Use HMD's width/height since HMD's internal frame buffer size
				// might be much larger than window size.
				bgfx::setViewRect(0, 0, 0, hmd->width, hmd->height);
			}
			else
			{
			
				//bx::mtxLookAt(view, eye, at);

				float proj[16];
				bx::mtxProj(proj, 60.0f, float(m_width)/float(m_height), 0.1f, 100.0f, bgfx::getCaps()->homogeneousDepth);
				bgfx::setViewTransform(0, view, proj);

				// Set view 0 default viewport.
				bgfx::setViewRect(0, 0, 0, uint16_t(m_width), uint16_t(m_height) );
			}

			// This dummy draw call is here to make sure that view 0 is cleared
			// if no other draw calls are submitted to view 0.
			bgfx::touch(0);

			// Submit 11x11 cubes.
			for (uint32_t yy = 0; yy < 1; ++yy)
			{
				for (uint32_t xx = 0; xx < 1; ++xx)
				{
					//float mtx[16];
					float mtxPos[16];
					bx::mtxRotateXY(mtxPos, time + xx * 0.21f, time + yy * 0.37f);
					//mtxPos[12] = -15.0f + float(xx)*3.0f;
					//mtxPos[13] = -15.0f + float(yy)*3.0f;

					mtxPos[14] = 0.0f;

					float mtxScale[16];
					bx::mtxScale(mtxScale, 6.0);

					float modelView[16];

					bx::mtxMul(modelView, mtxScale, mtxPos);

					// Set model matrix for rendering.
					bgfx::setTransform(modelView, 1);

					// Set vertex and index buffer.
					bgfx::setVertexBuffer(0, m_vbh);
					bgfx::setIndexBuffer(m_ibh);

					// Set render states.
					bgfx::setState(0
						| (m_r ? BGFX_STATE_WRITE_R : 0)
						| (m_g ? BGFX_STATE_WRITE_G : 0)
						| (m_b ? BGFX_STATE_WRITE_B : 0)
						| (m_a ? BGFX_STATE_WRITE_A : 0)
						| BGFX_STATE_WRITE_Z
						| BGFX_STATE_DEPTH_TEST_LESS
						| BGFX_STATE_CULL_CW
						// | BGFX_STATE_CULL_CCW
						| BGFX_STATE_MSAA
						| BGFX_STATE_PT_LINESTRIP
						);

					// Submit primitive for rendering to view 0.
					bgfx::submit(0, m_program);
				}
			}

			#if 1
			float mtxFloor[16];
			bx::mtxSRT(mtxFloor
				, 30.0f, 30.0f, 30.0f
				, 0.0f, 0.0f, 0.0f
				, 0.0f, -10.0f, 0.0f
			);

			bgfx::setTransform(mtxFloor, 1);

			bgfx::setVertexBuffer(0, m_vbh2);
			bgfx::setIndexBuffer(m_ibh2);

			uint64_t state = 0
				| BGFX_STATE_WRITE_RGB
				| BGFX_STATE_WRITE_A
				| BGFX_STATE_WRITE_Z
				| BGFX_STATE_DEPTH_TEST_LESS
				| BGFX_STATE_CULL_CCW
				| BGFX_STATE_MSAA
				;
			bgfx::setState(state);
			bgfx::submit(0, m_program2);
			#endif


			// VOXELS
			bgfx::setUniform(uNormals_[0], &normals_[0]);
			bgfx::setUniform(uNormals_[1], &normals_[1]);

			bgfx::setUniform(uAmbient_, stbvox_default_ambient);

			float mtxIdentity[16];
			bx::mtxIdentity(mtxIdentity);
			
			bgfx::setTransform(mtxIdentity, 1);

			bgfx::setVertexBuffer(0, vbh_voxel_);
			bgfx::setIndexBuffer(ibh_voxel_);

			//bgfx::setState(state);
#if 0
			bgfx::setState(0
						| BGFX_STATE_WRITE_RGB
						| BGFX_STATE_WRITE_Z
						| BGFX_STATE_DEPTH_TEST_LESS
						//| BGFX_STATE_CULL_CW
						| BGFX_STATE_MSAA
						//| BGFX_STATE_PT_TRISTRIP
						);
#endif

			bgfx::setState(0
						| BGFX_STATE_WRITE_RGB
						| BGFX_STATE_WRITE_Z
						| BGFX_STATE_DEPTH_TEST_LESS
						//| BGFX_STATE_CULL_CW
						| BGFX_STATE_CULL_CCW
						| BGFX_STATE_MSAA
						//| BGFX_STATE_PT_TRISTRIP
				//| BGFX_STATE_PT_LINES
				//| BGFX_STATE_PT_TRISTRIP
						);

			bgfx::submit(0, program_voxel_);

			// Advance to next frame. Rendering thread will be kicked to
			// process submitted rendering primitives.
			bgfx::frame();

			return true;
		}

		return false;
	}

	entry::MouseState m_mouseState;

	uint32_t m_width;
	uint32_t m_height;
	uint32_t m_debug;
	uint32_t m_reset;
	bgfx::VertexBufferHandle m_vbh, m_vbh2, vbh_voxel_;
	bgfx::IndexBufferHandle m_ibh, m_ibh2, ibh_voxel_;
	bgfx::ProgramHandle m_program, m_program2, program_voxel_;

	bgfx::UniformHandle uNormals_[2];
	float normals_[2][3][3];

	bgfx::UniformHandle uAmbient_;

	int64_t m_timeOffset;

	bool m_r;
	bool m_g;
	bool m_b;
	bool m_a;

	// Voxels
	stbvox_mesh_maker mesh_maker_;
	void* mesh_buffer_ {nullptr};
	unsigned buffer_size_ {0};

	stbvox_rgb rgb_[kSizeMeshPlus2][kSizeMeshPlus2][kSizeMeshPlus2]{};
	stbvox_block_type blocktype_[kSizeMeshPlus2][kSizeMeshPlus2][kSizeMeshPlus2]{};
	unsigned char lighting_[kSizeMeshPlus2][kSizeMeshPlus2][kSizeMeshPlus2]{};
		

};

} // namespace

ENTRY_IMPLEMENT_MAIN(ExampleCubes, "01-cubes", "Rendering simple static mesh.");
