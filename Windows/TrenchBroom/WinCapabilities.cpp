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
            
            HDC hdc = GetDC(NULL);

			PIXELFORMATDESCRIPTOR descriptor;
			descriptor.nSize = sizeof(PIXELFORMATDESCRIPTOR);
			descriptor.nVersion = 1;
			descriptor.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_GENERIC_ACCELERATED | PFD_DOUBLEBUFFER;
			descriptor.iPixelType = PFD_TYPE_RGBA;
			descriptor.cColorBits = 32;
			descriptor.cRedBits = descriptor.cRedShift = 0;
			descriptor.cGreenBits = descriptor.cGreenShift = 0;
			descriptor.cBlueBits = descriptor.cBlueShift = 0;
			descriptor.cAlphaBits = descriptor.cAlphaShift = 0;
			descriptor.cAccumBits = descriptor.cAccumRedBits = descriptor.cAccumGreenBits = descriptor.cAccumBlueBits = descriptor.cAccumAlphaBits = 0;
			descriptor.cStencilBits = 0;
			descriptor.cAuxBuffers = 0;
			descriptor.bReserved = 0;

			int pixelFormatIndex = ChoosePixelFormat(hdc, &descriptor);
			assert(pixelFormatIndex > 0);
			
			bool set = (SetPixelFormat(hdc, pixelFormatIndex, &descriptor) == TRUE);
			assert(set);

			int maxPixelFormatIndex = DescribePixelFormat(hdc, pixelFormatIndex, sizeof(PIXELFORMATDESCRIPTOR), &descriptor);
			assert(maxPixelFormatIndex > 0);

			capabilities.depthBits = descriptor.cDepthBits;

			// is this actually initialized already?
			if (GLEW_ARB_multisample) {
				capabilities.multisample = true;
				capabilities.samples = 4;
			}

            return capabilities;
        }
    }
}