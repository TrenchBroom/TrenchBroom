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
#include "Utility/CachedPtr.h"
#include "Utility/String.h"
#include "Utility/VecMath.h"

#include <map>
#include <memory>

using namespace TrenchBroom::Math;

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
            typedef Utility::CachedPtr<StringRenderer> StringRendererPtr;
            
            class StringManager : public Utility::CachedPtr<StringRenderer>::Cache {
            public:
                typedef std::auto_ptr<StringManager> Ptr;
            protected:
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
                
                typedef std::map<CacheKey, StringRendererPtr> StringCache;
                typedef std::map<StringRenderer*, CacheKey> InverseCacheMap;
                typedef std::map<CacheKey, StringRenderer*> UnpreparedStringMap;
                typedef std::vector<StringRenderer*> StringRendererList;
                
                StringCache m_stringCache;
                InverseCacheMap m_inverseCache;
                UnpreparedStringMap m_unpreparedStrings;
                StringRendererList m_deletableStrings;
                
                StringVectorizer* m_stringVectorizer;
                PathTesselator* m_tesselator;
                Vbo* m_vbo;
                
                void prepareStrings();
                void deleteStrings();
            public:
                StringManager(Utility::Console& console);
                ~StringManager();
                
                StringRendererPtr stringRenderer(const FontDescriptor& fontDescriptor, const String& string);
                void deleteElement(StringRenderer* stringRenderer);

                Vec2f measureString(const FontDescriptor& fontDescriptor, const String& string);
                Vec2f selectFontSize(const FontDescriptor& fontDescriptor, const String& string, const Vec2f& bounds, unsigned int minSize, FontDescriptor& resultDescriptor);
                Vec2f selectFontSizeWithEllipses(const FontDescriptor& fontDescriptor, const String& string, const Vec2f& bounds, unsigned int minSize, FontDescriptor& resultDescriptor, String& resultString);

                void activate();
                void deactivate();
            };
        }
    }
}

#endif /* defined(__TrenchBroom__StringManager__) */
