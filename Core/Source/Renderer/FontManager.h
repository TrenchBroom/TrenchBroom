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

using namespace std;

namespace TrenchBroom {
    namespace Renderer {
        class Vbo;
        class VboBlock;

        typedef vector<int> IntBuffer;
        typedef vector<float> FloatBuffer;

        class FontDescriptor {
        public:
            const string name;
            const float size;
            FontDescriptor(const string name, float size) : name(name), size(size) {}
        };

        bool operator<(FontDescriptor const& left, FontDescriptor const& right);

        class StringData {
        private:
            GLenum m_type;
        public:
            class Point {
            public:
                float x,y;
                Point(float x, float y) : x(x), y(y) {};
            };

            FloatBuffer triangleSet;
            vector<FloatBuffer*> triangleStrips;
            vector<FloatBuffer*> triangleFans;
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
            StringData* m_data;
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
            const string str;
            float width;
            float height;

            StringRenderer(const FontDescriptor& descriptor, const string& str, StringData* stringData);
            ~StringRenderer();
            void prepare(Vbo& vbo);
            void renderBackground(float hInset, float vInset);
            void render();
        };

        class StringFactory {
        public:
            virtual ~StringFactory() {};
            virtual StringData* createStringData(const FontDescriptor& descriptor, const string& str) = 0;
        };

        class FontManager {
        private:
            typedef pair<StringRenderer*, int> StringCacheEntry;
            typedef map<const string, StringCacheEntry*> StringCache;
            typedef map<const FontDescriptor, StringCache*> FontCache;

            Vbo* m_vbo;
            vector<StringRenderer*> m_unpreparedStrings;
            FontCache m_fontCache;
            StringFactory& m_stringFactory;
        public:
            FontManager(StringFactory& stringFactory);
            ~FontManager();
            StringRenderer& createStringRenderer(const FontDescriptor& descriptor, const string& str);
            void destroyStringRenderer(StringRenderer& stringRenderer);
            void clear();

            void activate();
            void deactivate();
        };
    }
}

#endif
