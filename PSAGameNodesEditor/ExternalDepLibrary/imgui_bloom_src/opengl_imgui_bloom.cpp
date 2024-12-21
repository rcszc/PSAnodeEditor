// opengl_imgui_bloom.
#include "opengl_imgui_bloom.h"

using namespace std;

#define DEF_VERTEX_POSITION 3
#define DEF_VERTEX_NORMAL   3
#define DEF_VERTEX_COLOR    4
#define DEF_VERTEX_UVCOORD  2

#define DEF_FP32_BYTES 4U // data_format: float-32bit.
#define DEF_VERTEX_ELEMENT (DEF_VERTEX_POSITION + DEF_VERTEX_NORMAL + DEF_VERTEX_COLOR + DEF_VERTEX_UVCOORD)
#define DEF_VERTEX_BYTES (DEF_VERTEX_ELEMENT * DEF_FP32_BYTES)

// OPENGL VAO SYSTEM PRESET.
inline void VERTEX_ATTRIBUTE_PRESET(const GLsizei& VER_LEN_BYTES, const uint32_t& BEGIN_COUNT) {
	// vertex_data.poscoord 3 * float, bias = 0, Location = 0.
	// vertex_block.begin = 0, vertex_block.end = 2.
	glVertexAttribPointer(BEGIN_COUNT + 0, DEF_VERTEX_POSITION, GL_FLOAT, GL_FALSE, VER_LEN_BYTES, 
		(void*)(0 * DEF_FP32_BYTES));
	glEnableVertexAttribArray(BEGIN_COUNT + 0);

	// vertex_data.color 4 * float, bias = 0 + 3, Locat * ion = 1.
	// vertex_block.begin = 3, vertex_block.end = 6.
	glVertexAttribPointer(BEGIN_COUNT + 1, DEF_VERTEX_COLOR, GL_FLOAT, GL_FALSE, VER_LEN_BYTES, 
		(void*)(3 * DEF_FP32_BYTES));
	glEnableVertexAttribArray(BEGIN_COUNT + 1);

	// vertex_data.texture 2 * float, bias = 0 + 3 + 4, Location = 2.
	// vertex_block.begin = 7, vertex_block.end = 9.
	glVertexAttribPointer(BEGIN_COUNT + 2, DEF_VERTEX_UVCOORD, GL_FLOAT, GL_FALSE, VER_LEN_BYTES, 
		(void*)(7 * DEF_FP32_BYTES));
	glEnableVertexAttribArray(BEGIN_COUNT + 2);

	// vertex_data.normal 3 * float, bias = 0 + 3 + 4 + 2, Location = 3.
	// vertex_block.begin = 9, vertex_block.end = 11.
	glVertexAttribPointer(BEGIN_COUNT + 3, DEF_VERTEX_NORMAL, GL_FLOAT, GL_FALSE, VER_LEN_BYTES, 
		(void*)(9 * DEF_FP32_BYTES));
	glEnableVertexAttribArray(BEGIN_COUNT + 3);
}

#define DEF_DRAW_DATA 72
// PRETSET VERTEX_DATA. format: vao preset.
inline vector<float> RenderTemplateRECT(float zlayer) {
	float RenderVertexData[DEF_DRAW_DATA] = {
		-1.0f, -1.0f, zlayer, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
		 1.0f, -1.0f, zlayer, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
		 1.0f,  1.0f, zlayer, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,

		-1.0f, -1.0f, zlayer, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
		 1.0f,  1.0f, zlayer, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
		-1.0f,  1.0f, zlayer, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f
	};
	return vector<float>(RenderVertexData, RenderVertexData + DEF_DRAW_DATA);
}

namespace BloomImGui {
	void PRESET_ERR_PRINT(const std::string& err_info) {
		std::cout << err_info << std::endl;
	}

	bool __ShaderCompileSystem::ShaderCompilationStatus(const GLuint* shader, bool is_program_flag) {
		ContextErrorWhat.clear();
		// return state_flag, error_log size.
		GLint CompileSuccessFlag = NULL, ShaderLogBytes = NULL;
		char* ShaderErrorInfo = nullptr;

		if (!is_program_flag) {
			// shader compiler status & log_length.
			glGetShaderiv(*shader, GL_COMPILE_STATUS, &CompileSuccessFlag);
			glGetShaderiv(*shader, GL_INFO_LOG_LENGTH, &ShaderLogBytes);

			if (!CompileSuccessFlag) {
				// program link failed.
				ShaderErrorInfo = new char[ShaderLogBytes];
				glGetShaderInfoLog(*shader, ShaderLogBytes, NULL, (GLchar*)ShaderErrorInfo);

				string ErrMagCache(" ", ShaderLogBytes);
				ErrMagCache.insert(ErrMagCache.begin(), ShaderErrorInfo, ShaderErrorInfo + (size_t)ShaderLogBytes);

				ContextErrorWhat = "compiler_err_msg: ";
				ContextErrorWhat += ErrMagCache;
				
				delete[] ShaderErrorInfo;
				return (bool)CompileSuccessFlag;
			}
			return (bool)CompileSuccessFlag;
		}
		// program link status & log_length.
		glGetProgramiv(*shader, GL_LINK_STATUS, &CompileSuccessFlag);
		glGetProgramiv(*shader, GL_INFO_LOG_LENGTH, &ShaderLogBytes);

		if (!CompileSuccessFlag) {
			// shader compilation failed.
			ShaderErrorInfo = new char[ShaderLogBytes];
			glGetProgramInfoLog(*shader, ShaderLogBytes, NULL, (GLchar*)ShaderErrorInfo);

			string ErrMagCache(" ", ShaderLogBytes);
			ErrMagCache.insert(ErrMagCache.begin(), ShaderErrorInfo, ShaderErrorInfo + (size_t)ShaderLogBytes);

			ContextErrorWhat = "link_program_err_msg: ";
			ContextErrorWhat += ErrMagCache;

			delete[] ShaderErrorInfo;
			return (bool)CompileSuccessFlag;
		}
		return (bool)CompileSuccessFlag;
	}

	bool FxBloomSystem::CreateRenderVertex(RenderVertex* render_model) {
		// create vbo => craete vao => config vao => upload data => unbind.
		glGenBuffers(1, &render_model->RenderVBO);
		glBindBuffer(GL_ARRAY_BUFFER, render_model->RenderVBO);

		glGenVertexArrays(1, &render_model->RenderVAO);
		glBindVertexArray(render_model->RenderVAO);

		if (render_model->RenderVBO == DEF_OPENGL_NULL_HANDLE || render_model->RenderVAO == DEF_OPENGL_NULL_HANDLE)
			return false;

		VERTEX_ATTRIBUTE_PRESET(DEF_VERTEX_BYTES, 0);
		// preset 2d-depth(z) 0.0f
		auto VertexDataTemp = RenderTemplateRECT(0.0f);
		glBufferData(GL_ARRAY_BUFFER, VertexDataTemp.size() * DEF_FP32_BYTES, VertexDataTemp.data(), GL_STATIC_DRAW);

		glBindVertexArray(NULL);
		glBindBuffer(GL_ARRAY_BUFFER, NULL);
	return true;
	}

#define DEF_SHADER_CODE_COUNT 1
	bool FxBloomSystem::CreateRenderShaderProgram(OGL_INDEX_SHADER* render_shader, ShaderCode vert, ShaderCode frag) {
		string ShaderCodeTemp[2] = { vert,frag };
		*render_shader = glCreateProgram();
		// create vert & frag shader => load source => compile shaders => attach => link_program.
		GLuint VertShader = glCreateShader(GL_VERTEX_SHADER);
		GLuint FragShader = glCreateShader(GL_FRAGMENT_SHADER);

		const GLchar* VertCode = ShaderCodeTemp[0].c_str();
		const GLchar* FragCode = ShaderCodeTemp[1].c_str();
		
		glShaderSource(VertShader, DEF_SHADER_CODE_COUNT, &VertCode, NULL);
		glShaderSource(FragShader, DEF_SHADER_CODE_COUNT, &FragCode, NULL);

		// compile shader => get state_flag => api_err_info.
		glCompileShader(VertShader);
		if (!ShaderCompilationStatus(&VertShader, false)) {
			ERR_INFO_FUNC("vertex_shader: " + ContextErrorWhat);
			return false;
		}
		glCompileShader(FragShader);
		if (!ShaderCompilationStatus(&FragShader, false)) {
			ERR_INFO_FUNC("fragment_shader: " + ContextErrorWhat);
			return false;
		}
		// vertex & fragment shader => shader_program.
		glAttachShader(*render_shader, VertShader);
		glAttachShader(*render_shader, FragShader);
		
		glLinkProgram(*render_shader);
		if (!ShaderCompilationStatus(render_shader, true)) {
			ERR_INFO_FUNC(ContextErrorWhat);
			return false;
		}
		return true;
	}

	bool FxBloomSystem::CreateRenderTexture(OGL_INDEX_TXETURE* render_texture, GLuint size_x, GLuint size_y) {
		// create texture => init texture => config texture.
		glGenTextures(1, render_texture);
		glBindTexture(GL_TEXTURE_2D, *render_texture);

		// color_format: HDR12(rgba12-bit).
		glTexImage2D(GL_TEXTURE_2D, NULL, GL_RGBA12, (GLsizei)size_x, (GLsizei)size_y, NULL, GL_RGBA, GL_FLOAT, nullptr);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glBindTexture(GL_TEXTURE_2D, NULL);
		return true;
	}

	bool FxBloomSystem::CreateRenderFrameBuffer(OGL_INDEX_FRAME* render_framebuffer, OGL_INDEX_FRAME bind_texture) {
		// create frame_buffer => texture bind fbo => check status.
		glGenFramebuffers(1, render_framebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, *render_framebuffer);

		glBindFramebuffer(GL_FRAMEBUFFER, *render_framebuffer);
		glBindTexture(GL_TEXTURE_2D, bind_texture);

		if (bind_texture == DEF_OPENGL_NULL_HANDLE)
			return false;
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bind_texture, NULL);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			glDeleteRenderbuffers(1, render_framebuffer);
			return false;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, NULL);
		glBindTexture(GL_TEXTURE_2D, NULL);
		return true;
	}

	void FxBloomSystem::RenderDrawVertex(const RenderVertex& params) {
		// bind vao & vbo => draw vertex => unbind.
		glBindVertexArray(params.RenderVAO);
		glBindBuffer(GL_ARRAY_BUFFER, params.RenderVBO);
		glDrawArrays(GL_TRIANGLES, NULL, GLsizei(DEF_DRAW_DATA / DEF_VERTEX_ELEMENT));
		glBindVertexArray(NULL);
		glBindBuffer(GL_ARRAY_BUFFER, NULL);
	}

	void FxBloomSystem::RenderDrawTexture(
		const OGL_INDEX_SHADER& shader, const char* name, const OGL_INDEX_TXETURE& texture, GLuint sub_offset
	) {
		// active tmu => bind texture => upload uniform(value).
		glActiveTexture(GL_TEXTURE31 - sub_offset);
		glBindTexture(GL_TEXTURE_2D, texture);

		GLint UniformLocation = glGetUniformLocation(shader, name);
		glUniform1i(UniformLocation, GLint(31 - sub_offset));
	}

	void FxBloomSystem::RenderFrameBufferContextBegin(const OGL_INDEX_FRAME& frame_buffer) {
		// bind frame_buffer => draw color => clear window buffer.
		glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void FxBloomSystem::RenderFrameBufferContextEnd() {
		glBindFramebuffer(GL_FRAMEBUFFER, NULL);
	}

	void FxBloomSystem::RenderUniformBlurRadius(const OGL_INDEX_SHADER& shader, GLuint radius) {
		GLint UniformLocation = glGetUniformLocation(shader, "BlurRadius");
		glUniform1i(UniformLocation, (GLint)radius);
	}

	inline void UNIFORM_FLOAT1(const OGL_INDEX_SHADER& shader, const char* name, float value) {
		GLint UniformLocation = glGetUniformLocation(shader, name);
		glUniform1f(UniformLocation, value);
	}
	inline void UNIFORM_FLOAT4(const OGL_INDEX_SHADER& shader, const char* name, const float value[4]) {
		GLint UniformLocation = glGetUniformLocation(shader, name);
		glUniform4f(UniformLocation, value[0], value[1], value[2], value[3]);
	}

	void FxBloomSystem::RenderUniformFilterParams(const OGL_INDEX_SHADER& shader, const FxFilterParams& params) {
		UNIFORM_FLOAT1(shader, "SceneFilterAvg",      params.ColorRGBchannelsAvgFilter);
		UNIFORM_FLOAT4(shader, "SceneFilterColorMax", params.ColorMaxFilter.data());
		UNIFORM_FLOAT4(shader, "SceneFilterColorMin", params.ColorMinFilter.data());
	}

	void FxBloomSystem::RenderUniformOutParams(const OGL_INDEX_SHADER& shader, const FxOutParams& params) {
		UNIFORM_FLOAT1(shader, "SourceBlend",   params.SourceFragmentBlend);
		UNIFORM_FLOAT1(shader, "BlurBlend",     params.BlurFragmentBlend);
		UNIFORM_FLOAT4(shader, "OutColorBlend", params.FragmentColorBlend.data());
	}

	FxBloomSystem::FxBloomSystem(GLuint win_x, GLuint win_y, function<void(const string&)> errinfo) {
		ERR_INFO_FUNC = errinfo;

		if (!CreateRenderVertex(&FxRenderVertexRECT))
			ERR_INFO_FUNC("failed create vao,vbo rect.");

		auto PublicVert = __SYSTEM_SCRIPT::VertexShaderPublic;
		if (!CreateRenderShaderProgram(&FxProcessShaders[0], PublicVert, __SYSTEM_SCRIPT::FragmentShaderFilter))
			ERR_INFO_FUNC("failed create_shader: filter.");
		if (!CreateRenderShaderProgram(&FxProcessShaders[1], PublicVert, __SYSTEM_SCRIPT::FragmentShaderBlurXaxis))
			ERR_INFO_FUNC("failed create_shader: blur_x-axis.");
		if (!CreateRenderShaderProgram(&FxProcessShaders[2], PublicVert, __SYSTEM_SCRIPT::FragmentShaderBlurYaxis))
			ERR_INFO_FUNC("failed create_shader: blur_y-axis.");
		if (!CreateRenderShaderProgram(&FxProcessShaders[3], PublicVert, __SYSTEM_SCRIPT::FragmentShaderOut))
			ERR_INFO_FUNC("failed create_shader: out_blend.");

		bool ErrorFlags = false;
		for (size_t i = 0; i < 4; ++i) {
			ErrorFlags |= !CreateRenderTexture(&FxProcessTextures[i], win_x, win_y);
			ErrorFlags |= !CreateRenderFrameBuffer(&FxProcessFrameBuffers[i], FxProcessTextures[i]);
		}
		if (ErrorFlags)
			ERR_INFO_FUNC("failed create textures,framebuffer.");
	}

	FxBloomSystem::~FxBloomSystem() {
		// free resource: texture, fbo, shader, vao, vbo.
		glDeleteTextures(4, FxProcessTextures);
		glDeleteFramebuffers(4, FxProcessFrameBuffers);
		for (size_t i = 0; i < 4; ++i)
			glDeleteShader(FxProcessShaders[i]);
		glDeleteVertexArrays(1, &FxRenderVertexRECT.RenderVAO);
		glDeleteBuffers(1, &FxRenderVertexRECT.RenderVBO);
	}

	void FxBloomSystem::SettingBloomRadius(uint32_t radius) {
		// clamp radius_value[1,30].
		radius = radius > 30 ? 30 : radius;
		radius = radius < 1 ? 1 : radius;
		FXP_BloomBlurRadius = radius;
	}

	void FxBloomSystem::RenderContextCaptureBegin() {
		// start imgui new_frame.
		ImGui_ImplOpenGL3_NewFrame();
        IMGUI_API_NEWFRAME_FUNC();
		ImGui::NewFrame();
	}

	constexpr const char* TexturesUniformName[3] = { "SourceScene", "BlurXprocScene", "BlurYprocScene" };
	void FxBloomSystem::RenderContextCaptureEnd() {
		// capture source_scene(imgui).
		RenderFrameBufferContextBegin(FxProcessFrameBuffers[0]);
		{
			// imgui render_gui frame.
			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		}
		RenderFrameBufferContextEnd();

		// capture render: filter fragments(color).
		RenderFrameBufferContextBegin(FxProcessFrameBuffers[1]);
		{
			glUseProgram(FxProcessShaders[0]);
			RenderUniformFilterParams(FxProcessShaders[0], FXP_BloomSceneFilter);
			RenderDrawTexture(FxProcessShaders[0], TexturesUniformName[0], FxProcessTextures[0], (GLuint)0);
			RenderDrawVertex(FxRenderVertexRECT);
			glUseProgram(NULL);
		}
		RenderFrameBufferContextEnd();

		// 1: capture render: fx_blur(x) fragments(color).
		// 2: capture render: fx_blur(y) fragments(color).
		for (size_t i = 1; i < 3; ++i) {
			RenderFrameBufferContextBegin(FxProcessFrameBuffers[i + 1]);
			{
				glUseProgram(FxProcessShaders[i]);
				RenderUniformBlurRadius(FxProcessShaders[i], FXP_BloomBlurRadius);
				RenderDrawTexture(FxProcessShaders[i], TexturesUniformName[i], FxProcessTextures[i], (GLuint)i);
				RenderDrawVertex(FxRenderVertexRECT);
				glUseProgram(NULL);
			}
			RenderFrameBufferContextEnd();
		}

		// render: blend out_frame presentation.
		glUseProgram(FxProcessShaders[3]);
		RenderUniformOutParams(FxProcessShaders[3], FXP_BloomSceneOut);
		RenderDrawTexture(FxProcessShaders[3], "SourceScene", FxProcessTextures[0], 0);
		RenderDrawTexture(FxProcessShaders[3], "BlurScene", FxProcessTextures[3], 3);
		RenderDrawVertex(FxRenderVertexRECT);
		glUseProgram(NULL);
	}
}
