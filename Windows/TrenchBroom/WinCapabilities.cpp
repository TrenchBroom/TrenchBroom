/*
 Copyright (C) 2010-2012 Kristian Duske
 
 This file is part of TrenchBroom.
 
 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "GL/Capabilities.h"

#include "GL/glew.h"
#include "GL/wglew.h"
#include <Windows.h>

#include <cassert>
#include <iostream>

namespace TrenchBroom {
    namespace GL {
		Capabilities doGlCapabilities() {
            Capabilities capabilities;

			// capabilities.depthBits = 32;

			// is this actually initialized already?
			if (GLEW_ARB_multisample) {
				capabilities.multisample = true;
				capabilities.samples = 4;
			}

            return capabilities;
        }

		/*

		Capabilities glCapabilities() {
            Capabilities capabilities;
            
			HDC hdc = wglGetCurrentDC();
			bool success;

			PIXELFORMATDESCRIPTOR descriptor =
			{
				sizeof(PIXELFORMATDESCRIPTOR),
				1,
				PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    //Flags
				PFD_TYPE_RGBA,            //The kind of framebuffer. RGBA or palette.
				32,                        //Colordepth of the framebuffer.
				0, 0, 0, 0, 0, 0,
				0,
				0,
				0,
				0, 0, 0, 0,
				16,                        //Number of bits for the depthbuffer
				0,                        //Number of bits for the stencilbuffer
				0,                        //Number of Aux buffers in the framebuffer.
				PFD_MAIN_PLANE,
				0,
				0, 0, 0
			};

			int initialPixelFormat = ChoosePixelFormat(hdc, &descriptor);
			assert(initialPixelFormat > 0);

			success = (SetPixelFormat(hdc, initialPixelFormat, &descriptor) == TRUE);
			assert(success);

			HGLRC context = wglCreateContext(hdc);
			assert(context != NULL);

			success = (wglMakeCurrent(hdc, context) == TRUE);
			assert(success);

			int attribs[] = 
			{
                // 32 bit depth buffer, 4 multisamples
				WGL_ACCELERATION_ARB,		GL_TRUE,
				WGL_DRAW_TO_WINDOW_ARB,	GL_TRUE,
				WGL_SUPPORT_OPENGL_ARB,	GL_TRUE,
				WGL_DOUBLE_BUFFER_ARB,		GL_TRUE,
				WGL_PIXEL_TYPE_ARB,		WGL_TYPE_RGBA_ARB,
				WGL_COLOR_BITS_ARB,		32,
				WGL_DEPTH_BITS_ARB,		32,
				WGL_SAMPLE_BUFFERS_ARB,	1,
				WGL_SAMPLES_ARB,			4,
				0,
                // 24 bit depth buffer, 4 multisamples
				WGL_ACCELERATION_ARB,		GL_TRUE,
				WGL_DRAW_TO_WINDOW_ARB,	GL_TRUE,
				WGL_SUPPORT_OPENGL_ARB,	GL_TRUE,
				WGL_DOUBLE_BUFFER_ARB,		GL_TRUE,
				WGL_PIXEL_TYPE_ARB,		WGL_TYPE_RGBA_ARB,
				WGL_COLOR_BITS_ARB,		32,
				WGL_DEPTH_BITS_ARB,		24,
				WGL_SAMPLE_BUFFERS_ARB,	1,
				WGL_SAMPLES_ARB,			4,
				0,
                // 32 bit depth buffer, 2 multisamples
				WGL_ACCELERATION_ARB,		GL_TRUE,
				WGL_DRAW_TO_WINDOW_ARB,	GL_TRUE,
				WGL_SUPPORT_OPENGL_ARB,	GL_TRUE,
				WGL_DOUBLE_BUFFER_ARB,		GL_TRUE,
				WGL_PIXEL_TYPE_ARB,		WGL_TYPE_RGBA_ARB,
				WGL_COLOR_BITS_ARB,		32,
				WGL_DEPTH_BITS_ARB,		32,
				WGL_SAMPLE_BUFFERS_ARB,	1,
				WGL_SAMPLES_ARB,			2,
				0,
                // 24 bit depth buffer, 2 multisamples
				WGL_ACCELERATION_ARB,		GL_TRUE,
				WGL_DRAW_TO_WINDOW_ARB,	GL_TRUE,
				WGL_SUPPORT_OPENGL_ARB,	GL_TRUE,
				WGL_DOUBLE_BUFFER_ARB,		GL_TRUE,
				WGL_PIXEL_TYPE_ARB,		WGL_TYPE_RGBA_ARB,
				WGL_COLOR_BITS_ARB,		32,
				WGL_DEPTH_BITS_ARB,		24,
				WGL_SAMPLE_BUFFERS_ARB,	1,
				WGL_SAMPLES_ARB,			2,
				0,
                // 16 bit depth buffer, 4 multisamples
				WGL_ACCELERATION_ARB,		GL_TRUE,
				WGL_DRAW_TO_WINDOW_ARB,	GL_TRUE,
				WGL_SUPPORT_OPENGL_ARB,	GL_TRUE,
				WGL_DOUBLE_BUFFER_ARB,		GL_TRUE,
				WGL_PIXEL_TYPE_ARB,		WGL_TYPE_RGBA_ARB,
				WGL_COLOR_BITS_ARB,		32,
				WGL_DEPTH_BITS_ARB,		16,
				WGL_SAMPLE_BUFFERS_ARB,	1,
				WGL_SAMPLES_ARB,			4,
				0,
                // 16 bit depth buffer, 2 multisamples
				WGL_ACCELERATION_ARB,		GL_TRUE,
				WGL_DRAW_TO_WINDOW_ARB,	GL_TRUE,
				WGL_SUPPORT_OPENGL_ARB,	GL_TRUE,
				WGL_DOUBLE_BUFFER_ARB,		GL_TRUE,
				WGL_PIXEL_TYPE_ARB,		WGL_TYPE_RGBA_ARB,
				WGL_COLOR_BITS_ARB,		32,
				WGL_DEPTH_BITS_ARB,		16,
				WGL_SAMPLE_BUFFERS_ARB,	1,
				WGL_SAMPLES_ARB,			4,
				0,
                // 32 bit depth buffer, no multisampling
				WGL_ACCELERATION_ARB,		GL_TRUE,
				WGL_DRAW_TO_WINDOW_ARB,	GL_TRUE,
				WGL_SUPPORT_OPENGL_ARB,	GL_TRUE,
				WGL_DOUBLE_BUFFER_ARB,		GL_TRUE,
				WGL_PIXEL_TYPE_ARB,		WGL_TYPE_RGBA_ARB,
				WGL_COLOR_BITS_ARB,		32,
				WGL_DEPTH_BITS_ARB,		32,
				WGL_SAMPLE_BUFFERS_ARB,	0,
				WGL_SAMPLES_ARB,			0,
				0,
                // 24 bit depth buffer, no multisampling
				WGL_ACCELERATION_ARB,		GL_TRUE,
				WGL_DRAW_TO_WINDOW_ARB,	GL_TRUE,
				WGL_SUPPORT_OPENGL_ARB,	GL_TRUE,
				WGL_DOUBLE_BUFFER_ARB,		GL_TRUE,
				WGL_PIXEL_TYPE_ARB,		WGL_TYPE_RGBA_ARB,
				WGL_COLOR_BITS_ARB,		32,
				WGL_DEPTH_BITS_ARB,		24,
				WGL_SAMPLE_BUFFERS_ARB,	0,
				WGL_SAMPLES_ARB,			0,
				0,
                // 16 bit depth buffer, no multisampling
				WGL_ACCELERATION_ARB,		GL_TRUE,
				WGL_DRAW_TO_WINDOW_ARB,	GL_TRUE,
				WGL_SUPPORT_OPENGL_ARB,	GL_TRUE,
				WGL_DOUBLE_BUFFER_ARB,		GL_TRUE,
				WGL_PIXEL_TYPE_ARB,		WGL_TYPE_RGBA_ARB,
				WGL_COLOR_BITS_ARB,		32,
				WGL_DEPTH_BITS_ARB,		24,
				WGL_SAMPLE_BUFFERS_ARB,	0,
				WGL_SAMPLES_ARB,			0,
				0,
			};

			int pixelFormats[16];
			unsigned int pixelFormatCount = 0;
			unsigned int index = 0;
			int attrValues[9];
			bool accelerated = false;

			while (!accelerated && attribs[index] != 0) {
				success = (wglChoosePixelFormatARB(hdc, &attribs[index], NULL, 16, pixelFormats, &pixelFormatCount) == TRUE);
				assert(success);
                
                if (pixelFormatCount > 0) {
					for (unsigned int i = 0; i < pixelFormatCount && !accelerated; i++) {
						success = (wglGetPixelFormatAttribivARB(hdc, i, 0, 9, &attribs[index], attrValues) == TRUE);
						assert(success);
						accelerated = (attrValues[0] == GL_TRUE);
					}
                }

				if (!accelerated) {
			        while (attribs[index] != 0)
		                index++;
	                index++;
				}
			}

			if (accelerated) {
				capabilities.depthBits = attrValues[6];
				capabilities.multisample = (attrValues[7] > 0);
				capabilities.samples = attrValues[8];
			}

			wglDeleteContext(context);

			return capabilities;
        }
		*/
    }
}