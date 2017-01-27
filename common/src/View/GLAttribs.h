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

#ifndef TrenchBroom_GLAttribs_h
#define TrenchBroom_GLAttribs_h

#include <vector>

class wxGLAttributes;

namespace TrenchBroom {
    namespace View {
        class GLAttribs {
        private:
            struct Config {
                int depth;
                bool multisample;
                int samples;

                Config();
                Config(const int i_depth, const bool i_multisample, const int i_samples);
                wxGLAttributes attribs() const;
            };
            
            bool m_initialized;
            Config m_config;
        private:
            GLAttribs();
            void initialize();
        private:
            static const GLAttribs& instance();
            
            wxGLAttributes getAttribs() const;
            int getDepth() const;
            bool getMultisample() const;
            int getSamples() const;
        public:
            static wxGLAttributes attribs();
            static int depth();
            static bool multisample();
            static int samples();
        };
    }
}

#endif
