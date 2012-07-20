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

#ifndef TrenchBroom_FontManager_h
#define TrenchBroom_FontManager_h

#include <string>
#include <vector>
#include <map>
#include "GL/GLee.h"
#include "Utilities/MessageException.h"
#include "Utilities/SharedPointer.h"

namespace TrenchBroom {
    namespace Renderer {
        class Vbo;
        class VboBlock;

        typedef std::vector<int> IntBuffer;
        typedef std::vector<float> FloatBuffer;

        class FontDescriptor {
        public:
            const std::string name;
            const int size;
            FontDescriptor(const std::string& name, int size) : name(name), size(size) {}
        };

        bool operator<(FontDescriptor const& left, FontDescriptor const& right);

        class StringData {
        private:
            GLenum m_type;
        public:
            class Point {
            public:
                float x,y;
				Point() : x(0), y(0) {};
                Point(float x, float y) : x(x), y(y) {};
            };

            FloatBuffer triangleSet;
            std::vector<FloatBuffer*> triangleStrips;
            std::vector<FloatBuffer*> triangleFans;
            int vertexCount;
            float width, height;

            StringData(float width, float height);
            ~StringData();
            void begin(GLenum type);
            void append(Point& vertex);
            void end();
        };

        class StringRenderer {
        private:
            VboBlock* m_vboBlock;
            GLuint m_listId;
            bool m_hasTriangleSet;
            bool m_hasTriangleStrips;
            bool m_hasTriangleFans;
            int m_triangleSetIndex;
            int m_triangleSetCount;
            IntBuffer* m_triangleStripIndices;
            IntBuffer* m_triangleStripCounts;
            IntBuffer* m_triangleFanIndices;
            IntBuffer* m_triangleFanCounts;
        public:
            const FontDescriptor fontDescriptor;
            const std::string str;
            float width;
            float height;

            StringRenderer(const FontDescriptor& descriptor, const std::string& str, float width, float height);
            ~StringRenderer();
            void prepare(const StringData& stringData, Vbo& vbo);
            void renderBackground(float hInset, float vInset);
            void render();
        };

        typedef std::tr1::shared_ptr<StringRenderer> StringRendererPtr;

        class StringFactory {
        public:
            virtual ~StringFactory() {};
            virtual StringData* createStringData(const FontDescriptor& descriptor, const std::string& str) = 0;
            virtual StringData::Point measureString(const FontDescriptor& descriptor, const std::string& str) = 0;
        };

        class FontManager {
        private:
            // this had to be structured like this to avoid a warning in VC++
            class StringCacheEntry {
            public:
                StringRendererPtr stringRenderer;
                int count;
                StringCacheEntry(StringRendererPtr stringRenderer, int count) : stringRenderer(stringRenderer), count(count) {};
            };
            typedef std::tr1::shared_ptr<StringCacheEntry> StringCacheEntryPtr;

            typedef std::map<const std::string, StringCacheEntryPtr> StringCacheMap;
            class StringCache {
            public:
                StringCacheMap stringCacheMap;
            };
            typedef std::tr1::shared_ptr<StringCache> StringCachePtr;
            
            typedef std::map<const FontDescriptor, StringCachePtr> FontCacheMap;
            class FontCache {
            public:
                FontCacheMap fontCacheMap;
            };

            Vbo* m_vbo;
            std::vector<StringRendererPtr> m_unpreparedStrings;
            FontCache m_fontCache;
            StringFactory* m_stringFactory;
        public:
            FontManager(StringFactory* stringFactory);
            ~FontManager();
            StringRendererPtr createStringRenderer(const FontDescriptor& descriptor, const std::string& str);
            void destroyStringRenderer(StringRendererPtr stringRenderer);
            void clear();

            void activate();
            void deactivate();
        };
        
        class FontCreationException : public MessageException {
        public:
            FontCreationException(const std::stringstream& str) : MessageException(str) {}
        };
    }
}

#endif
