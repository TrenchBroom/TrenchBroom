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

#include "Quake3Shader.h"

#include <cassert>

namespace TrenchBroom {
    namespace Assets {
        Quake3Shader::Quake3Shader() :
        m_hasTexturePath(false),
        m_hasQerImagePath(false),
        m_hasQerTransparency(false) {}

        bool Quake3Shader::operator==(const Quake3Shader& other) const {
            return (m_hasTexturePath == other.m_hasTexturePath && m_texturePath == other.m_texturePath &&
                    m_hasQerImagePath == other.m_hasQerImagePath && m_qerImagePath == other.m_qerImagePath &&
                    m_hasQerTransparency == other.m_hasQerTransparency && m_qerTransparency == other.m_qerTransparency);
        }

        bool Quake3Shader::hasTexturePath() const {
            return m_hasTexturePath;
        }

        const IO::Path& Quake3Shader::texturePath() const {
            assert(m_hasTexturePath);
            return m_texturePath;
        }

        void Quake3Shader::setTexturePath(const IO::Path& texturePath) {
            m_texturePath = texturePath;
            m_hasTexturePath = true;
        }

        bool Quake3Shader::hasQerImagePath() const {
            return m_hasQerImagePath;
        }

        const IO::Path& Quake3Shader::qerImagePath() const {
            assert(m_hasQerImagePath);
            return m_qerImagePath;
        }

        void Quake3Shader::setQerImagePath(const IO::Path& qerImagePath) {
            m_qerImagePath = qerImagePath;
            m_hasQerImagePath = true;
        }

        bool Quake3Shader::hasQerTransparency() const {
            return m_hasQerTransparency;
        }

        float Quake3Shader::qerTransparency() const {
            assert(m_hasQerTransparency);
            return m_qerTransparency;
        }

        void Quake3Shader::setQerTransparency(const float qerTransparency) {
            m_qerTransparency = qerTransparency;
            m_hasQerTransparency = true;
        }
    }
}

