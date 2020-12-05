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

#pragma once

#include "IO/Path.h"

#include <set>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Assets {
        class Quake3ShaderStage {
        public:
            struct BlendFunc {
                std::string srcFactor;
                std::string destFactor;

                static const std::string One;
                static const std::string Zero;
                static const std::string SrcColor;
                static const std::string DestColor;
                static const std::string OneMinusSrcColor;
                static const std::string OneMinusDestColor;
                static const std::string SrcAlpha;
                static const std::string DestAlpha;
                static const std::string OneMinusSrcAlpha;
                static const std::string OneMinusDestAlpha;
                static const std::string SrcAlphaSaturate;

                bool enable() const;
                bool validateSrcFactor() const;
                bool validateDestFactor() const;
                void reset();

                bool operator==(const BlendFunc& other) const;
            };
        public:
            IO::Path map;
            BlendFunc blendFunc;
        public:
            bool operator==(const Quake3ShaderStage& other) const;
        };

        class Quake3Shader {
        public:
            enum class Culling {
                Front,
                Back,
                None
            };
        public:
            IO::Path shaderPath;
            IO::Path editorImage;
            IO::Path lightImage;
            Culling culling = Culling::Front;
            std::set<std::string> surfaceParms;
            std::vector<Quake3ShaderStage> stages;
        public:
            bool operator==(const Quake3Shader& other) const;
            friend bool isEqual(const Quake3Shader& lhs, const Quake3Shader& rhs);

            Quake3ShaderStage& addStage();
        };
    }
}


