#pragma once
#include <glm/gtc/type_ptr.hpp>
#include "../ew/external/glad.h"

namespace willowLib
{
	const int SHADOW_RESOLUTION = 2048;

	struct DeferredPass {
		GLuint fbo = 0;
		GLuint world_position = 0;
		GLuint world_normal = 0;
		GLuint albedo = 0;
		GLuint depth = 0;
	};
	void createDeferredPass(DeferredPass* deferred, int screenWidth, int screenHeight);

	struct DisplayPass {
		GLuint vao;
		GLuint vbo;
	};
	void createDisplayPass(DisplayPass* disp);

	struct DisplayPassToTexture
	{
		GLuint vao = 0;
		GLuint vbo = 0;
		GLuint fbo = 0;
		GLuint scene = 0;
	};
	void createDisplayToTexturePass(DisplayPassToTexture dispToTex);

	struct StencilPass {
		GLuint fbo = 0;
		GLuint stencil = 0;
	};

	struct ShadowPass{
		GLuint rbo;
		GLuint fbo;
		GLuint map;
		GLuint vao;
		float minBias = 0.005;
		float maxBias = 0.03;
	};
	void createShadowPass(ShadowPass* shadow);
}