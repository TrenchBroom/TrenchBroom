/*
 Copyright (C) 2010-2017 Kristian Duske
 
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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TrenchBroom_EntityModel
#define TrenchBroom_EntityModel

#include "VecMath.h"

namespace TrenchBroom {
    namespace Renderer {
        class TexturedIndexRangeRenderer;
    }
    
    namespace Assets {
        class EntityModel {
        private:
            bool m_prepared;
        public:
            EntityModel();
            virtual ~EntityModel();
            
            Renderer::TexturedIndexRangeRenderer* buildRenderer(size_t skinIndex, size_t frameIndex) const;
            BBox3f bounds(size_t skinIndex, size_t frameIndex) const;

            virtual size_t frameCount() const = 0;
            virtual size_t skinCount() const = 0;

            bool prepared() const;
            void prepare(int minFilter, int magFilter);
            void setTextureMode(int minFilter, int magFilter);
        private:
            virtual Renderer::TexturedIndexRangeRenderer* doBuildRenderer(size_t skinIndex, size_t frameIndex) const = 0;
            virtual BBox3f doGetBounds(size_t skinIndex, size_t frameIndex) const = 0;
            virtual void doPrepare(int minFilter, int magFilter) = 0;
            virtual void doSetTextureMode(int minFilter, int magFilter) = 0;
        };
    }
}

#endif /* defined(TrenchBroom_EntityModel) */
