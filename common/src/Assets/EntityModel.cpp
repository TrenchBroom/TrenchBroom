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

#include "EntityModel.h"

namespace TrenchBroom {
    namespace Assets {
        EntityModel::EntityModel() :
        m_prepared(false) {}

        EntityModel::~EntityModel() {}

        Renderer::TexturedIndexRangeRenderer * EntityModel::buildRenderer(const size_t skinIndex, const size_t frameIndex) const {
            ensure(skinIndex < skinCount(), "skin index out of range");
            ensure(frameIndex < frameCount(), "frame index out of range");

            return doBuildRenderer(skinIndex, frameIndex);
        }

        BBox3f EntityModel::bounds(const size_t skinIndex, const size_t frameIndex) const {
            ensure(skinIndex < skinCount(), "skin index out of range");
            ensure(frameIndex < frameCount(), "frame index out of range");

            return doGetBounds(skinIndex, frameIndex);
        }

        bool EntityModel::prepared() const {
            return m_prepared;
        }

        void EntityModel::prepare(const int minFilter, const int magFilter) {
            if (!m_prepared) {
                doPrepare(minFilter, magFilter);
                m_prepared = true;
            }
        }

        void EntityModel::setTextureMode(const int minFilter, const int magFilter) {
            doSetTextureMode(minFilter, magFilter);
        }
    }
}
