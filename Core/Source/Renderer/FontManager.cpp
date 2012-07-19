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

#include "FontManager.h"
#include <cassert>
#include "Renderer/Vbo.h"

namespace TrenchBroom {
    namespace Renderer {
        bool operator<(FontDescriptor const& left, FontDescriptor const& right) {
            int cmp = left.name.compare(right.name);
            if (cmp < 0) return true;
            if (cmp > 0) return false;
            return left.size < right.size;
        }

        StringData::StringData(float width, float height) : width(width), height(height), vertexCount(0) {
        }

        StringData::~StringData() {
            while (!triangleStrips.empty()) delete triangleStrips.back(), triangleStrips.pop_back();
            while (!triangleFans.empty()) delete triangleFans.back(), triangleFans.pop_back();
        }

        void StringData::begin(GLenum type) {
            m_type = type;
            switch (m_type) {
                case GL_TRIANGLE_STRIP: {
                    FloatBuffer* strip = new FloatBuffer();
                    triangleStrips.push_back(strip);
                    break;
                }
                case GL_TRIANGLE_FAN: {
                    FloatBuffer* fan = new FloatBuffer();
                    triangleFans.push_back(fan);
                    break;
                }
                default:
                    break;
            }
        }

        void StringData::append(Point& vertex) {
            switch (m_type) {
                case GL_TRIANGLES:
                    triangleSet.push_back(vertex.x);
                    triangleSet.push_back(vertex.y);
                    break;
                case GL_TRIANGLE_STRIP: {
                    FloatBuffer* strip = triangleStrips.back();
                    strip->push_back(vertex.x);
                    strip->push_back(vertex.y);
                    break;
                }
                case GL_TRIANGLE_FAN: {
                    FloatBuffer* fan = triangleFans.back();
                    fan->push_back(vertex.x);
                    fan->push_back(vertex.y);
                    break;
                }
                default:
                    break;
            }
            vertexCount++;
        }

        void StringData::end() {
            // nothing to do
        }

        StringRenderer::StringRenderer(const FontDescriptor& descriptor, const std::string& str, StringData* stringData) : fontDescriptor(descriptor), str(str), m_data(stringData), m_vboBlock(NULL), width(stringData->width), height(stringData->height) {
            assert(stringData != NULL);
            m_hasTriangleSet = false;
            m_hasTriangleStrips = false;
            m_hasTriangleFans = false;
            m_listId = 0;
        }

        StringRenderer::~StringRenderer() {
            if (m_data != NULL)
                delete m_data;
            if (m_vboBlock != NULL)
                m_vboBlock->freeBlock();
            if (m_hasTriangleStrips) {
                delete m_triangleStripIndices;
                delete m_triangleStripCounts;
            }
            if (m_hasTriangleFans) {
                delete m_triangleFanIndices;
                delete m_triangleFanCounts;
            }
            if (m_listId > 0)
                glDeleteLists(m_listId, 1);
        }

        void StringRenderer::prepare(Vbo& vbo) {
            assert(m_data != NULL);
            m_vboBlock = vbo.allocBlock(2 * m_data->vertexCount * sizeof(float));
            assert(m_vboBlock != NULL);

            m_hasTriangleSet = !m_data->triangleSet.empty();
            m_hasTriangleStrips = !m_data->triangleStrips.empty();
            m_hasTriangleFans = !m_data->triangleFans.empty();

            unsigned int offset = 0;
            if (m_hasTriangleSet) {
                m_triangleSetIndex = (m_vboBlock->address + offset) / (2 * sizeof(float));
                m_triangleSetCount = (int)m_data->triangleSet.size() / 2;
                const unsigned char* buffer = (const unsigned char*)&m_data->triangleSet[0];
                offset = m_vboBlock->writeBuffer(buffer, offset, m_data->triangleSet.size() * sizeof(float));
            }

            if (m_hasTriangleStrips) {
                m_triangleStripIndices = new IntBuffer();
                m_triangleStripCounts = new IntBuffer();
                for (unsigned int i = 0; i < m_data->triangleStrips.size(); i++) {
                    FloatBuffer* strip = m_data->triangleStrips[i];
                    m_triangleStripIndices->push_back((m_vboBlock->address + offset) / (2 * sizeof(float)));
                    m_triangleStripCounts->push_back((int)strip->size() / 2);
                    const unsigned char* buffer = (const unsigned char*)&(*strip)[0];
                    offset = m_vboBlock->writeBuffer(buffer, offset, strip->size() * sizeof(float));
                }
            }

            if (m_hasTriangleFans) {
                m_triangleFanIndices = new IntBuffer();
                m_triangleFanCounts = new IntBuffer();
                for (unsigned int i = 0; i < m_data->triangleFans.size(); i++) {
                    FloatBuffer* fan = m_data->triangleFans[i];
                    m_triangleFanIndices->push_back((m_vboBlock->address + offset) / (2 * sizeof(float)));
                    m_triangleFanCounts->push_back((int)fan->size() / 2);
                    const unsigned char* buffer = (const unsigned char*)&(*fan)[0];
                    offset = m_vboBlock->writeBuffer(buffer, offset, fan->size() * sizeof(float));
                }
            }

            delete m_data;
            m_data = NULL;
        }

        void StringRenderer::renderBackground(float hInset, float vInset) {
            glBegin(GL_QUADS);
            glVertex3f(-hInset, -vInset, 0);
            glVertex3f(-hInset, height + vInset, 0);
            glVertex3f(width + hInset, height + vInset, 0);
            glVertex3f(width + hInset, -vInset, 0);
            glEnd();
        }

        void StringRenderer::render() {
            assert(m_vboBlock != NULL);
            if (m_listId == 0) {
                m_listId = glGenLists(1);
                assert(m_listId > 0);

                glNewList(m_listId, GL_COMPILE);
                if (m_hasTriangleSet) {
                    glDrawArrays(GL_TRIANGLES, m_triangleSetIndex, m_triangleSetCount);
                    m_hasTriangleSet = false;
                }
                if (m_hasTriangleStrips) {
                    GLint* indexPtr = &(*m_triangleStripIndices)[0];
                    GLsizei* countPtr = &(*m_triangleStripCounts)[0];
                    GLsizei primCount = (int)m_triangleStripIndices->size();
                    glMultiDrawArrays(GL_TRIANGLE_STRIP, indexPtr, countPtr, primCount);
                    delete m_triangleStripIndices;
                    delete m_triangleStripCounts;
                    m_hasTriangleStrips = false;
                }
                if (m_hasTriangleFans) {
                    GLint* indexPtr = &(*m_triangleFanIndices)[0];
                    GLsizei* countPtr = &(*m_triangleFanCounts)[0];
                    GLsizei primCount = (int)m_triangleFanIndices->size();
                    glMultiDrawArrays(GL_TRIANGLE_FAN, indexPtr, countPtr, primCount);
                    delete m_triangleFanIndices;
                    delete m_triangleFanCounts;
                    m_hasTriangleFans = false;
                }
                glEndList();
            }

            glCallList(m_listId);
        }

        FontManager::FontManager(StringFactory* stringFactory) : m_stringFactory(stringFactory), m_vbo(NULL) {}

        FontManager::~FontManager() {
            clear();
            if (m_vbo != NULL)
                delete m_vbo;
            delete m_stringFactory;
        }

        StringRendererPtr FontManager::createStringRenderer(const FontDescriptor& descriptor, const std::string& str) {
            FontCacheMap& fontCache = m_fontCache.fontCacheMap;
            FontCacheMap::iterator fontIt = fontCache.find(descriptor);
            StringCachePtr stringCachePtr;
            if (fontIt != fontCache.end()) {
                stringCachePtr = fontIt->second;
                StringCacheMap::iterator stringIt = stringCachePtr->stringCacheMap.find(str);
                if (stringIt != stringCachePtr->stringCacheMap.end()) {
                    StringCacheEntryPtr entry = stringIt->second;
                    entry->count++;
                    return entry->stringRenderer;
                }
            }

            StringData* stringData = m_stringFactory->createStringData(descriptor, str);
			if (stringData == NULL)
				return NULL;

            StringRenderer* stringRenderer = new StringRenderer(descriptor, str, stringData);
            StringRendererPtr stringRendererPtr(stringRenderer);
            
            if (stringCachePtr.get() == NULL) {
                stringCachePtr = StringCachePtr(new StringCache());
                fontCache[descriptor] = stringCachePtr;
            }

            m_unpreparedStrings.push_back(stringRendererPtr);
            stringCachePtr->stringCacheMap[str] = StringCacheEntryPtr(new StringCacheEntry(stringRendererPtr, 1));

            return stringRendererPtr;
        }

        void FontManager::destroyStringRenderer(StringRendererPtr stringRenderer) {
            std::vector<StringRendererPtr>::iterator unprepStrIt;
            for (unprepStrIt = m_unpreparedStrings.begin(); unprepStrIt != m_unpreparedStrings.end(); ++unprepStrIt) {
                if (unprepStrIt->get() == stringRenderer.get()) {
                    m_unpreparedStrings.erase(unprepStrIt);
                    break;
                }
            }

            FontCacheMap& fontCache = m_fontCache.fontCacheMap;
            FontCacheMap::iterator fontIt = fontCache.find(stringRenderer->fontDescriptor);
            StringCachePtr stringCachePtr;
            if (fontIt != fontCache.end()) {
                stringCachePtr = fontIt->second;
                StringCacheMap::iterator stringIt = stringCachePtr->stringCacheMap.find(stringRenderer->str);
                if (stringIt != stringCachePtr->stringCacheMap.end()) {
                    StringCacheEntryPtr entry = stringIt->second;
                    entry->count--;
                    if (entry->count <= 0) {
                        stringCachePtr->stringCacheMap.erase(stringIt);
                        if (stringCachePtr->stringCacheMap.empty())
                            fontCache.erase(fontIt);
                    }
                }
            }
        }

        void FontManager::clear() {
            m_unpreparedStrings.clear();
            m_fontCache.fontCacheMap.clear();
        }

        void FontManager::activate() {
            if (m_vbo == NULL)
                m_vbo = new Vbo(GL_ARRAY_BUFFER, 0xFFFF);
            m_vbo->activate();
            if (!m_unpreparedStrings.empty()) {
                m_vbo->map();
                for (unsigned int i = 0; i < m_unpreparedStrings.size(); i++)
                    m_unpreparedStrings[i]->prepare(*m_vbo);
                m_vbo->unmap();
                m_unpreparedStrings.clear();
            }
            glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
            glEnableClientState(GL_VERTEX_ARRAY);
            glVertexPointer(2, GL_FLOAT, 0, 0);
			glEnable(GL_POLYGON_SMOOTH);
			glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
        }

        void FontManager::deactivate() {
			glDisable(GL_POLYGON_SMOOTH);
            glPopClientAttrib();
            m_vbo->deactivate();
        }
    }
}
