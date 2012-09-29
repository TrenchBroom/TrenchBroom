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
#include "Utility/String.h"

namespace TrenchBroom {
    namespace Model {
        class TextureCollection;
        
        class Texture {
        public:
            static const String Empty;
            typedef int IdType;
        protected:
            TextureCollection& m_collection;
            String m_name;
            IdType m_uniqueId;
            unsigned int m_width;
            unsigned int m_height;
            unsigned int m_usageCount;
            bool m_overridden;
        public:
            Texture(TextureCollection& collection, const String& name, unsigned int width, unsigned int height) :
            m_collection(collection),
            m_name(name),
            m_width(width),
            m_height(height),
            m_usageCount(0),
            m_overridden(false) {
                static unsigned int uniqueId = 0;
                m_uniqueId = uniqueId++;
            }

            inline TextureCollection& collection() const {
                return m_collection;
            }
            
            inline const String& name() const {
                return m_name;
            }
            
            inline IdType uniqueId() const {
                return m_uniqueId;
            }
            
            inline unsigned int width() const {
                return m_width;
            }
            
            inline unsigned int height() const {
                return m_height;
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
        };
    }
}

#endif /* defined(__TrenchBroom__Texture__) */
