// opengl_imgui_bloom_script.
#include "opengl_imgui_bloom.h"

using namespace std;

namespace __SYSTEM_SCRIPT {
	const char* VertexShaderPublic = R"(
#version 460 core
layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec4 VertexColor;
layout (location = 2) in vec2 VertexTexture;
layout (location = 3) in vec3 VertexNormal;

out vec4 FxColor;
out vec2 FxCoord;

mat4 ConstOrthoMatrix = mat4(
	1.0, 0.0,  0.0, 0.0,
	0.0, 1.0,  0.0, 0.0,
	0.0, 0.0, -1.0, 0.0,
	0.0, 0.0,  0.0, 1.0
);

void main()
{
	gl_Position = ConstOrthoMatrix * vec4(VertexPosition, 1.0);

	FxColor = VertexColor;
	FxCoord = VertexTexture;
}
)";
	const char* FragmentShaderFilter = R"(
#version 460 core

in vec4 FxColor;
in vec2 FxCoord;

out vec4 FragColor;

uniform sampler2D SourceScene;

// [一级]RGBA分量过滤.
uniform vec4 SceneFilterColorMax;
uniform vec4 SceneFilterColorMin;

// [二级]RGB均值过滤.
uniform float SceneFilterAvg;

// component filter color, RGB.
vec4 FilterColor(vec4 InColor, vec4 ColorMax, vec4 ColorMin)
{
	bool PassFlag = all(greaterThanEqual(InColor, ColorMin)) && 
		all(lessThanEqual(InColor, ColorMax));

    if (PassFlag) {
        return InColor;
    }
	return vec4(0.0, 0.0f, 0.0f, InColor.a);
}

// average filter color, RGB.
vec4 FilterColorAvg(vec4 InColor, float Value)
{
    float ColorAverage = (InColor.r + InColor.g + InColor.b) / 3.0;
    if (ColorAverage > Value) {
        return InColor;
    } 
	return vec4(0.0, 0.0f, 0.0f, InColor.a);
}

void main()
{
	vec4 Lv1_FILTER = FilterColor(texture(SourceScene, FxCoord), SceneFilterColorMax, SceneFilterColorMin);
	vec4 Lv2_FILTER = FilterColorAvg(Lv1_FILTER, SceneFilterAvg);
    FragColor = Lv2_FILTER;
}
)";
	const char* FragmentShaderBlurXaxis = R"(
#version 460 core

in vec4 FxColor;
in vec2 FxCoord;

out vec4 FragColor;

uniform sampler2D BlurXprocScene;
uniform int BlurRadius;

#define SHADER_FRAG_MS_PI 3.1415926536
float Gaussian(float Gx, float Sigma)
{
    return exp(-(Gx * Gx) / (2.0 * Sigma * Sigma)) / (sqrt(2.0 * SHADER_FRAG_MS_PI) * Sigma);
}

// horizontal blur process.
vec4 BloomEffectX(sampler2D TexSample, vec2 TexCoords) 
{
	vec2 TexOffset = 1.0 / textureSize(TexSample, 0).xy;
	vec3 Result = texture(TexSample, TexCoords).rgb * Gaussian(0.0, BlurRadius);
	
	for(int i = 1; i < BlurRadius; ++i) {
		float BlurColor = Gaussian(float(i), BlurRadius);
		
		Result += texture(TexSample, TexCoords + vec2(TexOffset.x * i, 0.0)).rgb * BlurColor;
		Result += texture(TexSample, TexCoords - vec2(TexOffset.x * i, 0.0)).rgb * BlurColor;
    }
	return vec4(Result, 1.0);
}

void main()
{
    FragColor = BloomEffectX(BlurXprocScene, FxCoord);
}
)";
	const char* FragmentShaderBlurYaxis = R"(
#version 460 core

in vec4 FxColor;
in vec2 FxCoord;

out vec4 FragColor;

uniform sampler2D BlurYprocScene;
uniform int BlurRadius;

#define SHADER_FRAG_MS_PI 3.1415926536
float Gaussian(float Gx, float Sigma)
{
    return exp(-(Gx * Gx) / (2.0 * Sigma * Sigma)) / (sqrt(2.0 * SHADER_FRAG_MS_PI) * Sigma);
}

// vertical blur process.
vec4 BloomEffectY(sampler2D TexSample, vec2 TexCoords) 
{
	vec2 TexOffset = 1.0 / textureSize(TexSample, 0).xy;
	vec3 Result = texture(TexSample, TexCoords).rgb * Gaussian(0.0, BlurRadius);

	for(int i = 1; i < BlurRadius; ++i) {
		float BlurColor = Gaussian(float(i), BlurRadius);
		
		Result += texture(TexSample, TexCoords + vec2(0.0, TexOffset.y * i)).rgb * BlurColor;
		Result += texture(TexSample, TexCoords - vec2(0.0, TexOffset.y * i)).rgb * BlurColor;
	}
	return vec4(Result, 1.0);
}

void main()
{
    FragColor = BloomEffectY(BlurYprocScene, FxCoord);
}
)";
	const char* FragmentShaderOut = R"(
#version 460 core

in vec4 FxColor;
in vec2 FxCoord;

out vec4 FragColor;

uniform sampler2D SourceScene;
uniform sampler2D BlurScene;

uniform float SourceBlend;
uniform float BlurBlend;

uniform vec4 OutColorBlend;

vec4 SmoothTexture(sampler2D TexSample, vec2 TexCoords, float SmoothFactor) {
    vec2 TexSize = 1.0 / textureSize(TexSample, 0).xy;
	// sample_fragment center color.
    vec4 Color = texture(TexSample, TexCoords) * (1.0 - SmoothFactor);
    Color += texture(TexSample, TexCoords + vec2(TexSize.x, 0.0)) * (SmoothFactor * 0.25);
    Color += texture(TexSample, TexCoords - vec2(TexSize.x, 0.0)) * (SmoothFactor * 0.25);
    Color += texture(TexSample, TexCoords + vec2(0.0, TexSize.y)) * (SmoothFactor * 0.25);
    Color += texture(TexSample, TexCoords - vec2(0.0, TexSize.y)) * (SmoothFactor * 0.25);
    return Color;
}

void main()
{
	vec4 BlurScene = SmoothTexture(BlurScene, FxCoord, 0.92);
    vec4 Bloom = texture(SourceScene, FxCoord) * SourceBlend + BlurScene * BlurBlend;
	FragColor = OutColorBlend * Bloom;
}
)";
}