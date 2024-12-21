// opengl_imgui_bloom.
// opengl_api glsl shader. By_RCSZ, 20240917.
// imgui_for_opengl. processing:
// filter => blur_h => blur_v => blend(out).

#ifndef __OPENGL_IMGUI_BLOOM_H
#define __OPENGL_IMGUI_BLOOM_H
#include <GL/glew.h>
#include <GL/GL.h>

#include "imgui.h"
#include "imgui_impl_opengl3.h"

#include <iostream>
#include <vector>
#include <string>
#include <array>
#include <functional>

// opengl preset shaders_script. 
namespace __SYSTEM_SCRIPT {
	extern const char* VertexShaderPublic;
	extern const char* FragmentShaderFilter;
	extern const char* FragmentShaderBlurXaxis;
	extern const char* FragmentShaderBlurYaxis;
	extern const char* FragmentShaderOut;
}

// imgui global_ui bloom fx.
namespace BloomImGui {
#define DEF_OPENGL_NULL_HANDLE 0

	void PRESET_ERR_PRINT(const std::string& err_info);

	struct RenderVertex {
		GLuint RenderVAO = DEF_OPENGL_NULL_HANDLE;
		GLuint RenderVBO = DEF_OPENGL_NULL_HANDLE;
	};
	using OGL_INDEX_SHADER  = GLuint;
	using OGL_INDEX_TXETURE = GLuint;
	using OGL_INDEX_FRAME   = GLuint;

	class __ShaderCompileSystem {
	protected:
		std::string ContextErrorWhat = {};
		bool ShaderCompilationStatus(const GLuint* shader, bool is_program_flag);
	};

	struct FxFilterParams {
		float ColorRGBchannelsAvgFilter = 0.32f;

		std::array<float, 4> ColorMaxFilter = { 1.0f, 1.0f, 1.0f, 1.0f };
		std::array<float, 4> ColorMinFilter = { 0.0f, 0.0f, 0.0f, 0.0f };
	};

	struct FxOutParams {
		float SourceFragmentBlend = 0.72f;
		float BlurFragmentBlend   = 1.28f;

		std::array<float, 4> FragmentColorBlend = { 1.0f, 1.0f, 1.0f, 1.0f };
	};

	using ShaderCode = const std::string&;
	// texture_sampler res: "GL_TEXTURE31 0x84DF" { 31,30,29,28 }
	// version: 0.1A, vao&vbo: 1, shaders: 4, textures: 4, framebuffer: 4
	class FxBloomSystem :public __ShaderCompileSystem {
	private:
		std::function<void(const std::string&)> ERR_INFO_FUNC = {};
        std::function<void(void)> IMGUI_API_NEWFRAME_FUNC = {};
	protected:
		OGL_INDEX_SHADER  FxProcessShaders[4]      = {};
		OGL_INDEX_FRAME   FxProcessFrameBuffers[4] = {};
		// texture mapping => texture_unit.
		// tex0: unit_31, tex1: unit_30, tex2: unit_29, tex3: uint_28.
		OGL_INDEX_TXETURE FxProcessTextures[4] = {};

		RenderVertex FxRenderVertexRECT = {};

		GLuint         FXP_BloomBlurRadius  = 8;
		FxFilterParams FXP_BloomSceneFilter = {};
		FxOutParams    FXP_BloomSceneOut    = {};

		bool CreateRenderVertex(RenderVertex* render_model);
		bool CreateRenderShaderProgram(OGL_INDEX_SHADER* render_shader, ShaderCode vert, ShaderCode frag);
		bool CreateRenderTexture(OGL_INDEX_TXETURE* render_texture, GLuint size_x, GLuint size_y);
		bool CreateRenderFrameBuffer(OGL_INDEX_FRAME* render_framebuffer, OGL_INDEX_FRAME bind_texture);

		void RenderDrawVertex(const RenderVertex& params);
		void RenderDrawTexture(
			const OGL_INDEX_SHADER& shader, const char* name, const OGL_INDEX_TXETURE& texture, GLuint sub_offset
		);
		void RenderFrameBufferContextBegin(const OGL_INDEX_FRAME& frame_buffer);
		void RenderFrameBufferContextEnd();

		void RenderUniformBlurRadius(const OGL_INDEX_SHADER& shader, GLuint radius);
		void RenderUniformFilterParams(const OGL_INDEX_SHADER& shader, const FxFilterParams& params);
		void RenderUniformOutParams(const OGL_INDEX_SHADER& shader, const FxOutParams& params);
	public:
		FxBloomSystem(GLuint win_x, GLuint win_y, std::function<void(const std::string&)> errinfo = PRESET_ERR_PRINT);
		~FxBloomSystem();

		// window api mapping function.
        void GetImGuiNewFrameFUNC(const std::function<void(void)>& function) {
            IMGUI_API_NEWFRAME_FUNC = function;
        }
		FxFilterParams* SettingFilterParams() { return &FXP_BloomSceneFilter; }
		FxOutParams*    SettingBlendParams()  { return &FXP_BloomSceneOut; }

		void SettingBloomRadius(uint32_t radius);

		void RenderContextCaptureBegin();
		void RenderContextCaptureEnd();
	};
}

#endif
