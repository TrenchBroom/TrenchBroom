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

#ifndef __TrenchBroom__Texture__
#define __TrenchBroom__Texture__

#include <GL/glew.h>
#include "Utility/Color.h"
#include "Utility/String.h"
#include "Utility/VecMath.h"

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace IO {
        class Mip;
    }
    
    namespace Model {
        class AliasSkin;
        class BspTexture;
        class Palette;
        
        class Texture {
        public:
            static const String Empty;
        protected:
            typedef int IdType;

            GLuint m_textureId;
            unsigned char* m_textureBuffer;

            String m_name;
            IdType m_uniqueId;
            unsigned int m_width;
            unsigned int m_height;
            Color m_averageColor;
            unsigned int m_usageCount;
            bool m_overridden;
            bool m_dummy;

            void init(const String& name, unsigned int width, unsigned int height);
            void init(const String& name, const unsigned char* indexedImage, unsigned int width, unsigned int height, const Palette& palette);
        public:
            Texture(const String& name, const unsigned char* rgbImage, unsigned int width, unsigned int height);
            Texture(const String& name, const unsigned char* indexedImage, unsigned int width, unsigned int height, const Palette& palette);
            Texture(const IO::Mip& mip, const Palette& palette);
            Texture(const String& name, const AliasSkin& skin, unsigned int skinIndex, const Palette& palette);
            Texture(const String& name, const BspTexture& texture, const Palette& palette);
            Texture(const String& name);
            ~Texture();

            inline const String& name() const {
                return m_name;
            }
            
            inline IdType uniqueId() const {
                return m_uniqueId;
            }
            
            inline GLuint textureId() const {
                return m_textureId;
            }
            
            inline unsigned int width() const {
                return m_width;
            }
            
            inline unsigned int height() const {
                return m_height;
            }
            
            inline const Color& averageColor() const {
                return m_averageColor;
            }
            
            inline unsigned int usageCount() const {
                return m_usageCount;
            }
            
            inline void incUsageCount() {
                m_usageCount++;
            }
            
            inline void decUsageCount() {
                m_usageCount--;
            }
            
            inline bool overridden() const {
                return m_overridden;
            }
            
            inline void setOverridden(bool overridden) {
                m_overridden = overridden;
            }
            
            inline bool dummy() const {
                return m_dummy;
            }
            
            void activate();
            void deactivate();
        };
    }
}

#endif /* defined(__TrenchBroom__Texture__) */
