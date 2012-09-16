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

#include "Capabilities.h"

#include <OpenGL/OpenGL.h>

#include <cassert>
#include <iostream>

namespace TrenchBroom {
    namespace GL {
        Capabilities glCapabilities() {
            Capabilities capabilities;
            
            CGLPixelFormatAttribute stop = static_cast<CGLPixelFormatAttribute>(0);
            CGLPixelFormatAttribute attribs[] =
            {
                // 32 bit depth buffer, 4 multisamples
                kCGLPFAAccelerated,
                kCGLPFAColorSize,       static_cast<CGLPixelFormatAttribute>(24),
                kCGLPFAAlphaSize,       static_cast<CGLPixelFormatAttribute>(8),
                kCGLPFADoubleBuffer,
                kCGLPFADepthSize,       static_cast<CGLPixelFormatAttribute>(32),
                kCGLPFAMultisample,
                kCGLPFASampleBuffers,   static_cast<CGLPixelFormatAttribute>(1),
                kCGLPFASamples,         static_cast<CGLPixelFormatAttribute>(4),
                stop,
                // 24 bit depth buffer, 4 multisamples
                kCGLPFAAccelerated,
                kCGLPFAColorSize,       static_cast<CGLPixelFormatAttribute>(24),
                kCGLPFAAlphaSize,       static_cast<CGLPixelFormatAttribute>(8),
                kCGLPFADoubleBuffer,
                kCGLPFADepthSize,       static_cast<CGLPixelFormatAttribute>(24),
                kCGLPFAMultisample,
                kCGLPFASampleBuffers,   static_cast<CGLPixelFormatAttribute>(1),
                kCGLPFASamples,         static_cast<CGLPixelFormatAttribute>(4),
                stop,
                // 32 bit depth buffer, 2 multisamples
                kCGLPFAAccelerated,
                kCGLPFAColorSize,       static_cast<CGLPixelFormatAttribute>(24),
                kCGLPFAAlphaSize,       static_cast<CGLPixelFormatAttribute>(8),
                kCGLPFADoubleBuffer,
                kCGLPFADepthSize,       static_cast<CGLPixelFormatAttribute>(32),
                kCGLPFAMultisample,
                kCGLPFASampleBuffers,   static_cast<CGLPixelFormatAttribute>(1),
                kCGLPFASamples,         static_cast<CGLPixelFormatAttribute>(2),
                stop,
                // 24 bit depth buffer, 2 multisamples
                kCGLPFAAccelerated,
                kCGLPFAColorSize,       static_cast<CGLPixelFormatAttribute>(24),
                kCGLPFAAlphaSize,       static_cast<CGLPixelFormatAttribute>(8),
                kCGLPFADoubleBuffer,
                kCGLPFADepthSize,       static_cast<CGLPixelFormatAttribute>(24),
                kCGLPFAMultisample,
                kCGLPFASampleBuffers,   static_cast<CGLPixelFormatAttribute>(1),
                kCGLPFASamples,         static_cast<CGLPixelFormatAttribute>(2),
                stop,
                // 16 bit depth buffer, 4 multisamples
                kCGLPFAAccelerated,
                kCGLPFAColorSize,       static_cast<CGLPixelFormatAttribute>(24),
                kCGLPFAAlphaSize,       static_cast<CGLPixelFormatAttribute>(8),
                kCGLPFADoubleBuffer,
                kCGLPFADepthSize,       static_cast<CGLPixelFormatAttribute>(16),
                kCGLPFAMultisample,
                kCGLPFASampleBuffers,   static_cast<CGLPixelFormatAttribute>(1),
                kCGLPFASamples,         static_cast<CGLPixelFormatAttribute>(4),
                stop,
                // 16 bit depth buffer, 2 multisamples
                kCGLPFAAccelerated,
                kCGLPFAColorSize,       static_cast<CGLPixelFormatAttribute>(24),
                kCGLPFAAlphaSize,       static_cast<CGLPixelFormatAttribute>(8),
                kCGLPFADoubleBuffer,
                kCGLPFADepthSize,       static_cast<CGLPixelFormatAttribute>(24),
                kCGLPFAMultisample,
                kCGLPFASampleBuffers,   static_cast<CGLPixelFormatAttribute>(1),
                kCGLPFASamples,         static_cast<CGLPixelFormatAttribute>(2),
                stop,
                // 32 bit depth buffer, no multisampling
                kCGLPFAAccelerated,
                kCGLPFAColorSize,       static_cast<CGLPixelFormatAttribute>(24),
                kCGLPFAAlphaSize,       static_cast<CGLPixelFormatAttribute>(8),
                kCGLPFADoubleBuffer,
                kCGLPFADepthSize,       static_cast<CGLPixelFormatAttribute>(32),
                stop,
                // 24 bit depth buffer, no multisampling
                kCGLPFAAccelerated,
                kCGLPFAColorSize,       static_cast<CGLPixelFormatAttribute>(24),
                kCGLPFAAlphaSize,       static_cast<CGLPixelFormatAttribute>(8),
                kCGLPFADoubleBuffer,
                kCGLPFADepthSize,       static_cast<CGLPixelFormatAttribute>(24),
                stop,
                // 16 bit depth buffer, no multisampling
                kCGLPFAAccelerated,
                kCGLPFAColorSize,       static_cast<CGLPixelFormatAttribute>(24),
                kCGLPFAAlphaSize,       static_cast<CGLPixelFormatAttribute>(8),
                kCGLPFADoubleBuffer,
                kCGLPFADepthSize,       static_cast<CGLPixelFormatAttribute>(24),
                stop,
                stop
            };
            
            CGLError error;
            CGLPixelFormatObj pixelFormat = NULL;
            GLint numPixelFormats;
            unsigned int index = 0;
            
            while (pixelFormat == NULL && attribs[index] != stop) {
                error = CGLChoosePixelFormat(&attribs[index], &pixelFormat, &numPixelFormats);
                assert(error == kCGLNoError);
                
                if (pixelFormat != NULL) {
                    GLint accelerated = 0;
                    error = CGLDescribePixelFormat(pixelFormat, 0, kCGLPFAAccelerated, &accelerated);
                    assert(error == kCGLNoError);
                    if (accelerated == 0) {
                        // CGLReleasePixelFormat(pixelFormat);
                        pixelFormat = NULL;
                    }
                }
                
                if (pixelFormat == NULL) {
                    while (attribs[index] != stop)
                        index++;
                    index++;
                }
            }
            
            if (pixelFormat != NULL) {
                GLint value;
                error = CGLDescribePixelFormat(pixelFormat, 0, kCGLPFADepthSize, &value);
                assert(error == kCGLNoError);
                capabilities.depthBits = value;
                
                error = CGLDescribePixelFormat(pixelFormat, 0, kCGLPFAMultisample, &value);
                assert(error == kCGLNoError);
                if (value != 0) {
                    capabilities.multisample = true;
                    
                    error = CGLDescribePixelFormat(pixelFormat, 0, kCGLPFASamples, &value);
                    assert(error == kCGLNoError);
                    capabilities.samples = value;
                }
                
                // CGLReleasePixelFormat(pixelFormat);
                pixelFormat = NULL;
            }

            return capabilities;
        }
    }
}