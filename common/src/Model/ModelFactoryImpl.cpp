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

#include "ModelFactoryImpl.h"

#include "Exceptions.h"
#include "Model/BrushNode.h"
#include "Model/BrushError.h"
#include "Model/BrushFace.h"
#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Model/GroupNode.h"
#include "Model/LayerNode.h"
#include "Model/ParallelTexCoordSystem.h"
#include "Model/ParaxialTexCoordSystem.h"
#include "Model/WorldNode.h"

#include <kdl/overload.h>
#include <kdl/result.h>
#include <kdl/string_utils.h>

#include <cassert>

namespace TrenchBroom {
    namespace Model {
        ModelFactoryImpl::ModelFactoryImpl() :
        m_format(MapFormat::Unknown) {}

        ModelFactoryImpl::ModelFactoryImpl(const MapFormat format) :
        m_format(format) {
            assert(m_format != MapFormat::Unknown);
        }

        MapFormat ModelFactoryImpl::doGetFormat() const {
            return m_format;
        }
    }
}
