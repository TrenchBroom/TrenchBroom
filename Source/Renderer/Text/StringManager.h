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

#ifndef __TrenchBroom__StringManager__
#define __TrenchBroom__StringManager__

#include "Renderer/Text/FontDescriptor.h"
#include "Utility/String.h"

#include <map>

namespace TrenchBroom {
    namespace Utility {
        class Console;
    }
    
    namespace Renderer {
        class Vbo;
        
        namespace Text {
            class PathRenderer;
            class PathTesselator;
            class StringVectorizer;
            
            typedef TrenchBroom::Renderer::Text::PathRenderer StringRenderer;
            
            class StringManager {
            protected:
                class CacheEntry {
                protected:
                    StringRenderer* m_stringRenderer;
                    unsigned int m_usageCount;
                public:
                    CacheEntry(StringRenderer* stringRenderer) :
                    m_stringRenderer(stringRenderer),
                    m_usageCount(1) {}
                    
                    inline StringRenderer* stringRenderer() const {
                        return m_stringRenderer;
                    }
                    
                    inline void incUsageCount() {
                        m_usageCount++;
                    }
                    
                    inline bool decUsageCount() {
                        m_usageCount--;
                        return m_usageCount == 0;
                    }
                };
                
                class CacheKey {
                protected:
                    FontDescriptor m_fontDescriptor;
                    String m_string;
                    unsigned long m_stringHash;
                public:
                    CacheKey(const FontDescriptor& fontDescriptor, const String& str) :
                    m_fontDescriptor(fontDescriptor),
                    m_string(str),
                    m_stringHash(Utility::makeHash(str)) {}
                    
                    inline int compare(const CacheKey& other) const {
                        int descriptorOrder = m_fontDescriptor.compare(other.fontDescriptor());
                        if (descriptorOrder < 0)
                            return -1;
                        if (descriptorOrder > 0)
                            return +1;
                        
                        if (m_stringHash < other.stringHash())
                            return -1;
                        if (m_stringHash > other.stringHash())
                            return +1;
                        
                        return m_string.compare(other.string());
                    }
                    
                    inline bool operator< (const CacheKey& other) const {
                        return compare(other) < 0;
                    }
                    
                    inline const FontDescriptor& fontDescriptor() const {
                        return m_fontDescriptor;
                    }
                    
                    inline const String& string() const {
                        return m_string;
                    }
                    
                    inline unsigned long stringHash() const {
                        return m_stringHash;
                    }
                };
                
                typedef std::map<CacheKey, CacheEntry> StringCache;
                typedef std::map<StringRenderer*, CacheKey> InverseCacheMap;
                typedef std::vector<StringRenderer*> StringRendererList;
                
                StringCache m_stringCache;
                InverseCacheMap m_inverseCache;
                StringRendererList m_unpreparedStrings;
                
                Vbo* m_vbo;
                StringVectorizer* m_stringVectorizer;
                PathTesselator* m_tesselator;
                
                void prepare();
            public:
                StringManager(Utility::Console& console);
                ~StringManager();
                
                StringRenderer* createStringRenderer(const FontDescriptor& fontDescriptor, const String& string);
                void destroyStringRenderer(StringRenderer* stringRenderer);
                
                void activate();
                void deactivate();
            };
        }
    }
}

#endif /* defined(__TrenchBroom__StringManager__) */
