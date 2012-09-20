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

#include "StringManager.h"

#include "Renderer/Vbo.h"
#include "Renderer/Text/Path.h"
#include "Renderer/Text/PathRenderer.h"
#include "Renderer/Text/PathTesselator.h"
#include "Renderer/Text/StringVectorizer.h"

namespace TrenchBroom {
    namespace Renderer {
        namespace Text {
            void StringManager::prepare() {
                m_vbo->map();
                
                for (unsigned int i = 0; i < m_unpreparedStrings.size(); i++) {
                    StringRenderer* stringRenderer = m_unpreparedStrings[i];
                    stringRenderer->prepare(*m_tesselator, *m_vbo);
                }
                
                m_unpreparedStrings.clear();
                m_vbo->unmap();
            }
            
            StringManager::StringManager(Utility::Console& console) :
            m_stringVectorizer(NULL),
            m_tesselator(NULL),
            m_vbo(NULL) {
                m_stringVectorizer = new StringVectorizer(console);
                m_tesselator = new PathTesselator();
            }
            
            StringManager::~StringManager() {
                if (m_stringVectorizer != NULL) {
                    delete m_stringVectorizer;
                    m_stringVectorizer = NULL;
                }
                
                if (m_tesselator != NULL) {
                    delete m_tesselator;
                    m_tesselator = NULL;
                }
                
                StringCache::iterator it, end;
                for (it = m_stringCache.begin(), end = m_stringCache.end(); it != end; ++it) {
                    CacheEntry& cacheEntry = it->second;
                    delete cacheEntry.stringRenderer();
                }
                m_stringCache.clear();
                m_inverseCache.clear();
                m_unpreparedStrings.clear();
                
                if (m_vbo != NULL) {
                    delete m_vbo;
                    m_vbo = NULL;
                }
            }

            StringRenderer* StringManager::createStringRenderer(const FontDescriptor& fontDescriptor, const String& string) {
                CacheKey cacheKey(fontDescriptor, string);
                StringCache::iterator it = m_stringCache.find(cacheKey);
                if (it != m_stringCache.end()) {
                    CacheEntry& cacheEntry = it->second;
                    cacheEntry.incUsageCount();
                    return cacheEntry.stringRenderer();
                }
                
                PathPtr path = m_stringVectorizer->makePath(fontDescriptor, string);
                StringRenderer* stringRenderer = new StringRenderer(path);
                m_stringCache.insert(std::pair<CacheKey, CacheEntry>(cacheKey, CacheEntry(stringRenderer)));
                m_inverseCache.insert(std::pair<StringRenderer*, CacheKey>(stringRenderer, cacheKey));
                m_unpreparedStrings.push_back(stringRenderer);

                return stringRenderer;
            }
            
            void StringManager::destroyStringRenderer(StringRenderer* stringRenderer) {
                InverseCacheMap::iterator inverseIt = m_inverseCache.find(stringRenderer);
                assert(inverseIt != m_inverseCache.end());
                
                CacheKey& cacheKey = inverseIt->second;
                StringCache::iterator cacheIt = m_stringCache.find(cacheKey);
                assert(cacheIt != m_stringCache.end());
                
                CacheEntry& cacheEntry = cacheIt->second;
                if (cacheEntry.decUsageCount()) {
                    StringRendererList::iterator it = find(m_unpreparedStrings.begin(), m_unpreparedStrings.end(), stringRenderer);
                    if (it != m_unpreparedStrings.end())
                        m_unpreparedStrings.erase(it);
                    
                    delete cacheEntry.stringRenderer();
                    m_inverseCache.erase(inverseIt);
                    m_stringCache.erase(cacheIt);
                }
            }
            
            void StringManager::activate() {
                if (m_vbo == NULL)
                    m_vbo = new Vbo(GL_ARRAY_BUFFER, 0xFFFF);
                m_vbo->activate();
                
                if (!m_unpreparedStrings.empty())
                    prepare();
                
                glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
                glEnableClientState(GL_VERTEX_ARRAY);
                glVertexPointer(2, GL_FLOAT, 0, 0);
                glEnable(GL_POLYGON_SMOOTH);
                glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
            }
            
            void StringManager::deactivate() {
                glDisable(GL_POLYGON_SMOOTH);
                glPopClientAttrib();
                m_vbo->deactivate();
            }
            
            Vec2f StringManager::measureString(const FontDescriptor& fontDescriptor, const String& string) {
                CacheKey cacheKey(fontDescriptor, string);
                StringCache::iterator it = m_stringCache.find(cacheKey);
                if (it != m_stringCache.end()) {
                    CacheEntry& cacheEntry = it->second;
                    StringRenderer& stringRenderer = *cacheEntry.stringRenderer();
                    return Vec2f(stringRenderer.width(), stringRenderer.height());
                }
                
                return m_stringVectorizer->measureString(fontDescriptor, string);
            }
            
            Vec2f StringManager::selectFontSize(const FontDescriptor& fontDescriptor, const String& string, const Vec2f& bounds, unsigned int minSize, FontDescriptor& result) {
                result = fontDescriptor;
                Vec2f actualBounds = measureString(result, string);
                while (actualBounds.x > bounds.x && result.size() > minSize) {
                    result = FontDescriptor(result.name(), result.size() - 1);
                    actualBounds = measureString(result, string);
                }
                return actualBounds;
            }
        }
    }
}