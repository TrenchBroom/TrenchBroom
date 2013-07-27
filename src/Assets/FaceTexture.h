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

#ifndef __TrenchBroom__FaceTexture__
#define __TrenchBroom__FaceTexture__

#include "Color.h"
#include "StringUtils.h"
#include "Assets/Texture.h"
#include "GL/GL.h"
#include "Model/ModelTypes.h"

#include <cassert>
#include <vector>

namespace TrenchBroom {
    namespace Assets {
        class FaceTexture : public Texture {
        private:
            GLuint m_textureId;
            Color m_averageColor;
            String m_name;
            size_t m_width;
            size_t m_height;
            size_t m_usageCount;
            bool m_overridden;
        public:
            FaceTexture(const String& name, const size_t width, const size_t height);
            ~FaceTexture();

            GLuint textureId() const;
            void setTextureId(const GLuint textureId);
            const Color& averageColor() const;
            void setAverageColor(const Color& averageColor);
            const String& name() const;
            size_t width() const;
            size_t height() const;
            size_t usageCount() const;
            void incUsageCount();
            void decUsageCount();
            bool isOverridden() const;
            void setOverridden(const bool overridden);
            
            void activate() const;
            void deactivate() const;
        };
    }
}

#endif /* defined(__TrenchBroom__FaceTexture__) */
