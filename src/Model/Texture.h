/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#ifndef __TrenchBroom__Texture__
#define __TrenchBroom__Texture__

#include "StringUtils.h"
#include "SharedPointer.h"
#include "GL/GL.h"

#include <cassert>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class Texture;
        typedef std::tr1::shared_ptr<Texture> TexturePtr;
        typedef std::vector<TexturePtr> TextureList;

        class Texture {
        private:
            GLuint m_textureId;
            String m_name;
            size_t m_width;
            size_t m_height;
            size_t m_usageCount;
            bool m_overridden;
        public:
            static TexturePtr newTexture(const GLuint textureId, const String& name, const size_t width, const size_t height);
            
            inline const String& name() const {
                return m_name;
            }
            
            inline size_t width() const {
                return m_width;
            }
            
            inline size_t height() const {
                return m_height;
            }
            
            inline size_t usageCount() const {
                return m_usageCount;
            }
            
            inline void incUsageCount() {
                ++m_usageCount;
            }
            
            inline void decUsageCount() {
                assert(m_usageCount > 0);
                --m_usageCount;
            }
            
            inline bool isOverridden() const {
                return m_overridden;
            }
            
            inline void setOverridden(const bool overridden) {
                m_overridden = overridden;
            }
        private:
            Texture(const GLuint textureId, const String& name, const size_t width, const size_t height);
        };
    }
}

#endif /* defined(__TrenchBroom__Texture__) */
