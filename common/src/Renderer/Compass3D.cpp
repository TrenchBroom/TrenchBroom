/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#include "Compass3D.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "View/MovementRestriction.h"

namespace TrenchBroom {
    namespace Renderer {
        Compass3D::Compass3D(const View::MovementRestriction& restriction) :
        m_restriction(restriction) {}

        void Compass3D::doRenderCompass(RenderContext& renderContext, const Mat4x4f& transform) {
            PreferenceManager& prefs = PreferenceManager::instance();

            if (m_restriction.isRestricted(Math::Axis::AX)) {
                renderSolidAxis(  renderContext, transform * Mat4x4f::Rot90XCW,  prefs.get(Preferences::YAxisColor));
                renderSolidAxis(  renderContext, transform,                      prefs.get(Preferences::ZAxisColor));
                renderAxisOutline(renderContext, transform * Mat4x4f::Rot90YCCW, prefs.get(Preferences::CompassAxisOutlineColor));
                renderSolidAxis(  renderContext, transform * Mat4x4f::Rot90YCCW, prefs.get(Preferences::XAxisColor));
            } else if (m_restriction.isRestricted(Math::Axis::AY)) {
                renderSolidAxis(  renderContext, transform * Mat4x4f::Rot90YCCW, prefs.get(Preferences::XAxisColor));
                renderSolidAxis(  renderContext, transform,                      prefs.get(Preferences::ZAxisColor));
                renderAxisOutline(renderContext, transform * Mat4x4f::Rot90XCW,  prefs.get(Preferences::CompassAxisOutlineColor));
                renderSolidAxis(  renderContext, transform * Mat4x4f::Rot90XCW,  prefs.get(Preferences::YAxisColor));
            } else if (m_restriction.isRestricted(Math::Axis::AZ)) {
                renderSolidAxis(  renderContext, transform * Mat4x4f::Rot90YCCW, prefs.get(Preferences::XAxisColor));
                renderSolidAxis(  renderContext, transform * Mat4x4f::Rot90XCW,  prefs.get(Preferences::YAxisColor));
                renderAxisOutline(renderContext, transform,                      prefs.get(Preferences::CompassAxisOutlineColor));
                renderSolidAxis(  renderContext, transform,                      prefs.get(Preferences::ZAxisColor));
            } else {
                renderSolidAxis(renderContext, transform,                        prefs.get(Preferences::ZAxisColor));
                renderSolidAxis(renderContext, transform * Mat4x4f::Rot90YCCW,   prefs.get(Preferences::XAxisColor));
                renderSolidAxis(renderContext, transform * Mat4x4f::Rot90XCW,    prefs.get(Preferences::YAxisColor));
            }
        }
    }
}
