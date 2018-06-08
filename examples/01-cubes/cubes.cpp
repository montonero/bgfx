/*
 * Copyright 2011-2018 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */

#include "common.h"
#include "bgfx_utils.h"
#include "imgui/imgui.h"


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

constexpr uint32_t kSizeMesh{16};

constexpr uint32_t kSizeMeshPlus2 = kSizeMesh + 2;

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
		BX_UNUSED(s_cubeTriList, s_cubeTriStrip);

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

		m_timeOffset = bx::getHPCounter();

		imguiCreate();


		// Voxels start
		stbvox_init_mesh_maker(&mesh_maker_);

		stbvox_set_input_stride(&mesh_maker_, kSizeMeshPlus2*kSizeMeshPlus2, kSizeMeshPlus2);

		buffer_size_ = 1024*32*8;
		mesh_buffer_ = malloc(buffer_size_);
		memset(mesh_buffer_, 0, buffer_size_);
		uint32_t* mb = (uint32_t*)mesh_buffer_;
		assert(mb);
		assert(sizeof(stbvox_mesh_vertex) == sizeof(uint32_t));
		stbvox_set_buffer(&mesh_maker_, 0, 0, mesh_buffer_, buffer_size_);

		auto buf_count = stbvox_get_buffer_count(&mesh_maker_);
		tfm::printf("%s\n", buf_count);
		printf("----------------------------------\n");

		stbvox_input_description* input_descr = stbvox_get_input_description(&mesh_maker_);
		tfm::printf("%s\n", input_descr);

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

		// Generate sphere
		sphere(9);

		// Stb voxel needs to access -1 voxels
		input_descr->blocktype = &blocktype_[1][1][1];
		input_descr->rgb = &rgb_[1][1][1]; 
		input_descr->lighting = &lighting_[1][1][1];

		// These are indiced by block type
		input_descr->block_geometry = geomForBlockType;
		input_descr->block_color = colorForBlockType;
		

		stbvox_set_input_range(&mesh_maker_, 0, 0, 0, kSizeMesh, kSizeMesh, kSizeMesh);
		
		//tfm::printf("rgb: %d\n", rgb[2][4][kSizeMesh-1].b);

		int size_per_quad = stbvox_get_buffer_size_per_quad(&mesh_maker_, 0);
		tfm::printf("%s\n", size_per_quad);

		int res = stbvox_make_mesh(&mesh_maker_);
		tfm::printf("%d\n", res);
	}


	void sphere(uint radius) {
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
				}
			}
		}
		tfm::printf("Num of voxels: %d", counter);
		
	}
	

	void set_voxel_color(std::tuple<uint, uint, uint> pos, stbvox_rgb color) {
		using namespace std;
		uint x, y, z;
		tie(x, y, z) = pos;
		rgb_[x][y][z] = color;
		blocktype_[x][y][z] = 1;
	// blocktype_[x][y][z] = STBVOX_GEOM_solid;
		lighting_[x][y][z] = 255;
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
			float at[3] = { 0.0f,  5.0f,   0.0f };

			// Set view and projection matrix for view 0.
			const bgfx::HMD* hmd = bgfx::getHMD();
			if (NULL != hmd && 0 != (hmd->flags & BGFX_HMD_RENDERING) )
			{
				float view[16];
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
				float view[16];
				bx::mtxLookAt(view, eye, at);

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
					bx::mtxScale(mtxScale, 3.0);

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
						| BGFX_STATE_MSAA
						| BGFX_STATE_PT_TRISTRIP
						);

					// Submit primitive for rendering to view 0.
					bgfx::submit(0, m_program);
				}
			}

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
	bgfx::VertexBufferHandle m_vbh, m_vbh2;
	bgfx::IndexBufferHandle m_ibh, m_ibh2;
	bgfx::ProgramHandle m_program, m_program2;
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
