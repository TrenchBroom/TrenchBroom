/*
 Copyright (C) 2010-2017 Kristian Duske
 
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

#include <GL/glew.h>
#include <wx/glcanvas.h>

namespace TrenchBroom {
    namespace View {
        GLAttribs::Config::Config() :
        depth(0),
        multisample(false),
        samples(0) {}
        
        GLAttribs::Config::Config(const int i_depth, const bool i_multisample, const int i_samples) :
        depth(i_depth),
        multisample(i_multisample),
        samples(i_samples) {}
        
        wxGLAttributes GLAttribs::Config::attribs() const {
            wxGLAttributes result;
            result.PlatformDefaults();
            result.RGBA();
            result.DoubleBuffer();
            result.Depth(depth);
            if (multisample) {
                result.SampleBuffers(1);
                result.Samplers(samples);
            }
            result.EndList();
            return result;
        }
        
        GLAttribs::GLAttribs() :
        m_initialized(false) {
            initialize();
        }

        void GLAttribs::initialize() {
            typedef std::vector<Config> List;
            List configs;
            configs.push_back(Config(32, true, 4));
            configs.push_back(Config(24, true, 4));
            configs.push_back(Config(32, true, 2));
            configs.push_back(Config(24, true, 2));
            configs.push_back(Config(32, false, 0));
            configs.push_back(Config(24, false, 0));

            for (const Config& config : configs) {
                const wxGLAttributes attribs = config.attribs();
                if (wxGLCanvas::IsDisplaySupported(attribs)) {
                    m_config = config;
                    m_initialized = true;
                    break;
                }
            }
        }

        const GLAttribs& GLAttribs::instance() {
            static const GLAttribs instance;
            return instance;
        }
        
        wxGLAttributes GLAttribs::getAttribs() const {
            return m_config.attribs();
        }
        
        int GLAttribs::getDepth() const {
            return m_config.depth;
        }

        bool GLAttribs::getMultisample() const {
            return m_config.multisample;
        }
        
        int GLAttribs::getSamples() const {
            return m_config.samples;
        }
        
        wxGLAttributes GLAttribs::attribs() {
            return instance().getAttribs();
        }
        
        int GLAttribs::depth() {
            return instance().getDepth();
        }

        bool GLAttribs::multisample() {
            return instance().getMultisample();
        }
        
        int GLAttribs::samples() {
            return instance().getSamples();
        }
    }
}
