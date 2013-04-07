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

#include <GL/glx.h>
#include <GL/gl.h>

#include <cassert>
#include <iostream>

namespace TrenchBroom {
    namespace GL {
        Capabilities doGlCapabilities() {
            Capabilities capabilities;

            Display *dpy = XOpenDisplay(0);

            int attribs[] =
            {
                // 32 bit depth buffer, 4 samples
                GLX_RENDER_TYPE,        GLX_RGBA_BIT,
                GLX_RED_SIZE,           8,
                GLX_GREEN_SIZE,         8,
                GLX_BLUE_SIZE,          8,
                GLX_ALPHA_SIZE,         8,
                GLX_DOUBLEBUFFER,       True,
                GLX_DEPTH_SIZE,         32,
                GLX_SAMPLE_BUFFERS,     1,
                GLX_SAMPLES,            4,
                0,
                // 24 bit depth buffer, 4 samples
                GLX_RENDER_TYPE,        GLX_RGBA_BIT,
                GLX_RED_SIZE,           8,
                GLX_GREEN_SIZE,         8,
                GLX_BLUE_SIZE,          8,
                GLX_ALPHA_SIZE,         8,
                GLX_DOUBLEBUFFER,       True,
                GLX_DEPTH_SIZE,         24,
                GLX_SAMPLE_BUFFERS,     1,
                GLX_SAMPLES,            4,
                0,
                // 32 bit depth buffer, 2 samples
                GLX_RENDER_TYPE,        GLX_RGBA_BIT,
                GLX_RED_SIZE,           8,
                GLX_GREEN_SIZE,         8,
                GLX_BLUE_SIZE,          8,
                GLX_ALPHA_SIZE,         8,
                GLX_DOUBLEBUFFER,       True,
                GLX_DEPTH_SIZE,         32,
                GLX_SAMPLE_BUFFERS,     1,
                GLX_SAMPLES,            2,
                0,
                // 24 bit depth buffer, 2 samples
                GLX_RENDER_TYPE,        GLX_RGBA_BIT,
                GLX_RED_SIZE,           8,
                GLX_GREEN_SIZE,         8,
                GLX_BLUE_SIZE,          8,
                GLX_ALPHA_SIZE,         8,
                GLX_DOUBLEBUFFER,       True,
                GLX_DEPTH_SIZE,         24,
                GLX_SAMPLE_BUFFERS,     1,
                GLX_SAMPLES,            2,
                0,
                // 16 bit depth buffer, 4 samples
                GLX_RENDER_TYPE,        GLX_RGBA_BIT,
                GLX_RED_SIZE,           8,
                GLX_GREEN_SIZE,         8,
                GLX_BLUE_SIZE,          8,
                GLX_ALPHA_SIZE,         8,
                GLX_DOUBLEBUFFER,       True,
                GLX_DEPTH_SIZE,         16,
                GLX_SAMPLE_BUFFERS,     1,
                GLX_SAMPLES,            4,
                0,
                // 16 bit depth buffer, 2 samples
                GLX_RENDER_TYPE,        GLX_RGBA_BIT,
                GLX_RED_SIZE,           8,
                GLX_GREEN_SIZE,         8,
                GLX_BLUE_SIZE,          8,
                GLX_ALPHA_SIZE,         8,
                GLX_DOUBLEBUFFER,       True,
                GLX_DEPTH_SIZE,         16,
                GLX_SAMPLE_BUFFERS,     1,
                GLX_SAMPLES,            2,
                0,
                // 32 bit depth buffer, no multisampling
                GLX_RENDER_TYPE,        GLX_RGBA_BIT,
                GLX_RED_SIZE,           8,
                GLX_GREEN_SIZE,         8,
                GLX_BLUE_SIZE,          8,
                GLX_ALPHA_SIZE,         8,
                GLX_DOUBLEBUFFER,       True,
                GLX_DEPTH_SIZE,         32,
                GLX_SAMPLE_BUFFERS,     0,
                0,
                // 24 bit depth buffer, no multisampling
                GLX_RENDER_TYPE,        GLX_RGBA_BIT,
                GLX_RED_SIZE,           8,
                GLX_GREEN_SIZE,         8,
                GLX_BLUE_SIZE,          8,
                GLX_ALPHA_SIZE,         8,
                GLX_DOUBLEBUFFER,       True,
                GLX_DEPTH_SIZE,         24,
                GLX_SAMPLE_BUFFERS,     0,
                0,
                // 16 bit depth buffer, no multisampling
                GLX_RENDER_TYPE,        GLX_RGBA_BIT,
                GLX_RED_SIZE,           8,
                GLX_GREEN_SIZE,         8,
                GLX_BLUE_SIZE,          8,
                GLX_ALPHA_SIZE,         8,
                GLX_DOUBLEBUFFER,       True,
                GLX_DEPTH_SIZE,         16,
                GLX_SAMPLE_BUFFERS,     0,
                0,
                0,
            };

            unsigned int index = 0;
            int configCount = 0;
            GLXFBConfig* config = NULL;

            while (config == NULL && attribs[index] != 0) {
                config = glXChooseFBConfig(dpy, DefaultScreen(dpy), &attribs[index], &configCount);
                if (config == NULL || configCount == 0) {
                    while (attribs[index] != 0)
                        index += 2;
                    index++;
                }
            }

            if (config != NULL) {
                GLint value = 0;
                glXGetFBConfigAttrib(dpy, *config, GLX_DEPTH_SIZE, &value);
                capabilities.depthBits = value;

                value = 0;
                glXGetFBConfigAttrib(dpy, *config, GLX_SAMPLE_BUFFERS, &value);
                capabilities.multisample = (value != 0);
                if (capabilities.multisample) {
                    value = 0;
                    glXGetFBConfigAttrib(dpy, *config, GLX_SAMPLES, &value);
                    capabilities.multisample = value;
                }

                // delete config;
                config = NULL;
            }

            XCloseDisplay(dpy);

            return capabilities;
        }
    }
}
