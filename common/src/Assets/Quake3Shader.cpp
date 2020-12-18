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

#include <string>

namespace TrenchBroom {
    namespace Assets {
        const std::string Quake3ShaderStage::BlendFunc::One = "GL_ONE";
        const std::string Quake3ShaderStage::BlendFunc::Zero = "GL_ZERO";
        const std::string Quake3ShaderStage::BlendFunc::SrcColor = "GL_SRC_COLOR";
        const std::string Quake3ShaderStage::BlendFunc::DestColor = "GL_DST_COLOR";
        const std::string Quake3ShaderStage::BlendFunc::OneMinusSrcColor = "GL_ONE_MINUS_SRC_COLOR";
        const std::string Quake3ShaderStage::BlendFunc::OneMinusDestColor = "GL_ONE_MINUS_DST_COLOR";
        const std::string Quake3ShaderStage::BlendFunc::SrcAlpha = "GL_SRC_ALPHA";
        const std::string Quake3ShaderStage::BlendFunc::DestAlpha = "GL_DST_ALPHA";
        const std::string Quake3ShaderStage::BlendFunc::OneMinusSrcAlpha = "GL_ONE_MINUS_SRC_ALPHA";
        const std::string Quake3ShaderStage::BlendFunc::OneMinusDestAlpha = "GL_ONE_MINUS_DST_ALPHA";
        const std::string Quake3ShaderStage::BlendFunc::SrcAlphaSaturate = "GL_SRC_ALPHA_SATURATE";

        bool Quake3ShaderStage::BlendFunc::enable() const {
            return srcFactor != "" && destFactor != "";
        }

        bool Quake3ShaderStage::BlendFunc::validateSrcFactor() const {
            if (srcFactor == One) {
                return true;
            } else if (srcFactor == Zero) {
                return true;
            } else if (srcFactor == DestColor) {
                return true;
            } else if (srcFactor == OneMinusDestColor) {
                return true;
            } else if (srcFactor == SrcAlpha) {
                return true;
            } else if (srcFactor == DestAlpha) {
                return true;
            } else if (srcFactor == OneMinusSrcAlpha) {
                return true;
            } else if (srcFactor == OneMinusDestAlpha) {
                return true;
            } else if (srcFactor == SrcAlphaSaturate) {
                return true;
            } else {
                return false;
            }
        }

        bool Quake3ShaderStage::BlendFunc::validateDestFactor() const {
            if (destFactor == One) {
                return true;
            } else if (destFactor == Zero) {
                return true;
            } else if (destFactor == SrcColor) {
                return true;
            } else if (destFactor == OneMinusSrcColor) {
                return true;
            } else if (destFactor == SrcAlpha) {
                return true;
            } else if (destFactor == DestAlpha) {
                return true;
            } else if (destFactor == OneMinusSrcAlpha) {
                return true;
            } else if (destFactor == OneMinusDestAlpha) {
                return true;
            } else {
                return false;
            }
        }

        void Quake3ShaderStage::BlendFunc::reset() {
            srcFactor = destFactor = "";
        }

        bool operator==(const Quake3ShaderStage::BlendFunc& lhs, const Quake3ShaderStage::BlendFunc& rhs) {
            return lhs.srcFactor == rhs.srcFactor && lhs.destFactor == rhs.destFactor;
        }

        bool operator!=(const Quake3ShaderStage::BlendFunc& lhs, const Quake3ShaderStage::BlendFunc& rhs) {
            return !(lhs == rhs);
        }
        
        bool operator==(const Quake3ShaderStage& lhs, const Quake3ShaderStage& rhs) {
            return lhs.map == rhs.map && lhs.blendFunc == rhs.blendFunc;
        }

        bool operator!=(const Quake3ShaderStage& lhs, const Quake3ShaderStage& rhs) {
            return !(lhs == rhs);
        }

        Quake3ShaderStage& Quake3Shader::addStage() {
            stages.emplace_back();
            return stages.back();
        }

        bool operator==(const Quake3Shader& lhs, const Quake3Shader& rhs) {
            return lhs.shaderPath == rhs.shaderPath &&
                   lhs.editorImage == rhs.editorImage &&
                   lhs.lightImage == rhs.lightImage &&
                   lhs.culling == rhs.culling &&
                   lhs.surfaceParms == rhs.surfaceParms &&
                   lhs.stages == rhs.stages;
        }

        bool operator!=(const Quake3Shader& lhs, const Quake3Shader& rhs) {
            return !(lhs == rhs);
        }
    }
}

