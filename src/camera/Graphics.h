/*
 * Graphics.h
 *
 * Copyright (C) 2016 Cyrille Potereau
 *
 * based on original work from Chris Cummings:
 * http://robotblogging.blogspot.fr/2013/10/gpu-accelerated-camera-processing-on.html
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#pragma once

#include "GLES2/gl2.h"
#include "EGL/egl.h"
#include "EGL/eglext.h"

void InitGraphics();
void ReleaseGraphics();
void BeginFrame();
void EndFrame();

class GfxShader
{
	GLchar* Src;
	GLuint Id;
	GLuint GlShaderType;

public:

	GfxShader() : Src(NULL), Id(0), GlShaderType(0) {}
	~GfxShader() { if(Src) delete[] Src; }

	bool LoadVertexShader(const char* filename);
	bool LoadFragmentShader(const char* filename);
	GLuint GetId() { return Id; }
};

class GfxProgram
{
	GfxShader* VertexShader;
	GfxShader* FragmentShader;
	GLuint Id;

public:

	GfxProgram() {}
	~GfxProgram() {}

	bool Create(GfxShader* vertex_shader, GfxShader* fragment_shader);
	GLuint GetId() { return Id; }
};

class GfxTexture
{
	int Width;
	int Height;
	GLuint Id;

public:

	GfxTexture() : Width(0), Height(0) {}
	~GfxTexture() {}

	bool Create(int width, int height, const void* data = NULL);
	void SetPixels(const void* data);
	GLuint GetId() { return Id; }
};

void DrawTextureRect(GfxTexture* texture, float x0, float y0, float x1, float y1);
