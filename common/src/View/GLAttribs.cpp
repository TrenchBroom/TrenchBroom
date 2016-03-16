/*
 Copyright (C) 2010-2014 Kristian Duske
 
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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include "GLAttribs.h"

#include "Renderer/GL.h"

namespace TrenchBroom {
    namespace View {
        const GLAttribs& buildAttribs() {
            static bool initialized = false;
            static GLAttribs attribs = endList(GLAttribs().PlatformDefaults());
            if (initialized)
                return attribs;
            
            typedef std::vector<GLAttribs> List;
            List testAttribs;
            testAttribs.push_back(endList(GLAttribs().PlatformDefaults().RGBA().DoubleBuffer().Depth(32).SampleBuffers(1).Samplers(4)));
            testAttribs.push_back(endList(GLAttribs().PlatformDefaults().RGBA().DoubleBuffer().Depth(24).SampleBuffers(1).Samplers(4)));
            testAttribs.push_back(endList(GLAttribs().PlatformDefaults().RGBA().DoubleBuffer().Depth(32).SampleBuffers(1).Samplers(2)));
            testAttribs.push_back(endList(GLAttribs().PlatformDefaults().RGBA().DoubleBuffer().Depth(24).SampleBuffers(1).Samplers(2)));
            testAttribs.push_back(endList(GLAttribs().PlatformDefaults().RGBA().DoubleBuffer().Depth(32)));
            testAttribs.push_back(endList(GLAttribs().PlatformDefaults().RGBA().DoubleBuffer().Depth(24)));

            List::iterator it, end;
            for (it = testAttribs.begin(), end = testAttribs.end(); it != end && !initialized; ++it) {
                if (wxGLCanvas::IsDisplaySupported(*it)) {
                    attribs = *it;
                    initialized = true;
                }
            }
            
            assert(initialized);
            return attribs;
        }

        GLAttribs endList(GLAttribs attribs) {
            attribs.EndList();
            return attribs;
        }
    }
}
