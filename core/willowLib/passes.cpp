#include "passes.h"

namespace willowLib
{
	void createDeferredPass(DeferredPass* deferred, int screenWidth, int screenHeight)
	{
		//create framebuffer
		glCreateFramebuffers(1, &deferred->fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, deferred->fbo);

		//generate world_position
		glGenTextures(1, &deferred->world_position);
		glBindTexture(GL_TEXTURE_2D, deferred->world_position);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, screenWidth, screenHeight, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//generate world_normal
		glGenTextures(1, &deferred->world_normal);
		glBindTexture(GL_TEXTURE_2D, deferred->world_normal);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, screenWidth, screenHeight, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//generate albedo
		glGenTextures(1, &deferred->albedo);
		glBindTexture(GL_TEXTURE_2D, deferred->albedo);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, screenWidth, screenHeight, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//generate depth
		glGenTextures(1, &deferred->depth);
		glBindTexture(GL_TEXTURE_2D, deferred->depth);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, screenWidth, screenHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//Attach buffers to the FBO
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, deferred->world_position, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, deferred->world_normal, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, deferred->albedo, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, deferred->depth, 0);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			printf("ERROR::FRAMEBUFFER:: Framebuffer is not complete!\n");
		}

		unsigned int buffers[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
		glDrawBuffers(3, buffers);

		//Unbind the framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void createDisplayPass(DisplayPass* disp)
	{
		float quad[] = {
			//-X- -Y-  -U- -V- 
			  //Tri 1
			  -1,  1,   0,  1,
			  -1, -1,   0,  0,
			   1,  1,   1,  1,

			   //Tri 2
				1, -1,   1,  0,
				1,  1,   1,  1,
			   -1, -1,   0,  0,
		};
		//Set up VAO and VBO for screen quad
		glGenVertexArrays(1, &disp->vao);
		glGenBuffers(1, &disp->vbo);

		glBindVertexArray(disp->vao);
		glBindBuffer(GL_ARRAY_BUFFER, disp->vbo);

		glBufferData(GL_ARRAY_BUFFER, sizeof(quad), &quad, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

		glBindVertexArray(0);
	}

	void createDisplayToTexturePass(DisplayPassToTexture* dispToTex, int screenWidth, int screenHeight)
	{
		float quad[] = {
			//-X- -Y-  -U- -V- 
			  //Tri 1
			  -1,  1,   0,  1,
			  -1, -1,   0,  0,
			   1,  1,   1,  1,

			   //Tri 2
				1, -1,   1,  0,
				1,  1,   1,  1,
			   -1, -1,   0,  0,
		};
		//Set up VAO and VBO for screen quad
		glGenVertexArrays(1, &dispToTex->vao);
		glGenBuffers(1, &dispToTex->vbo);

		glBindVertexArray(dispToTex->vao);
		glBindBuffer(GL_ARRAY_BUFFER, dispToTex->vbo);

		glBufferData(GL_ARRAY_BUFFER, sizeof(quad), &quad, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

		glBindVertexArray(0);
		
		//create framebuffer
		glCreateFramebuffers(1, &dispToTex->fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, dispToTex->fbo);

		//generate scene buffer
		glGenTextures(1, &dispToTex->scene);
		glBindTexture(GL_TEXTURE_2D, dispToTex->scene);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, screenWidth, screenHeight, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//Attach buffers to the FBO
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, dispToTex->scene, 0);
		
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			printf("ERROR::FRAMEBUFFER:: Framebuffer is not complete!\n");
		}

		glDrawBuffer(GL_COLOR_ATTACHMENT0);

		//Unbind the framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void createShadowPass(ShadowPass* shadow)
	{
		//Shadow Framebuffer
		glGenFramebuffers(1, &shadow->fbo);
		//Shadow map
		glGenTextures(1, &shadow->map);

		//Bind to FBO
		glBindFramebuffer(GL_FRAMEBUFFER, shadow->fbo);
		//Bind to shadow map
		glBindTexture(GL_TEXTURE_2D, shadow->map);
		//Initialize shadow map texture
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, SHADOW_RESOLUTION, SHADOW_RESOLUTION, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		//Change buffer texture's filtering mode
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		//Change buffer texture's wrapping mode
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		float borderColor[] = { 1.0f,1.0f,1.0f,1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
		//Change buffer texture's compare mode
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);

		//Attach depth buffer to FBO
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadow->map, 0);
		//Tell glCheckFramebufferStatus that we don't need a color buffer here
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			printf("ERROR::FRAMEBUFFER:: Framebuffer is not complete!\n");
		}

		//Unbind the framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}