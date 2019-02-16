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

#include "BrushContentTypeEvaluator.h"

#include "Macros.h"
#include "Assets/Quake3Shader.h"
#include "Assets/Texture.h"
#include "Model/AttributableNode.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"

#include <algorithm>

namespace TrenchBroom {
    namespace Model {
        class BrushFaceEvaluator : public BrushContentTypeEvaluator {
        private:
            StringSet m_ignoreTexture;
        public:
            explicit BrushFaceEvaluator(const StringList& ignoreTexture) :
            m_ignoreTexture(std::begin(ignoreTexture), std::end(ignoreTexture)) {}
            ~BrushFaceEvaluator() override = default;
        private:
            bool doEvaluate(const Brush* brush) const override {
                size_t ignored = 0;
                size_t matched = 0;
                
                for (const auto* face : brush->faces()) {
                    if (m_ignoreTexture.count(face->textureName()) > 0) {
                        ++ignored;
                    } else if (doEvaluate(face)) {
                        ++matched;
                    }
                }
                
                return matched > 0 && matched + ignored == brush->faceCount();
            }
            
            virtual bool doEvaluate(const BrushFace* brush) const = 0;
        };
        
        class TextureNameEvaluator : public BrushFaceEvaluator {
        private:
            String m_pattern;
        public:
            explicit TextureNameEvaluator(const String& pattern, const StringList& ignoreTexture) :
            BrushFaceEvaluator(ignoreTexture),
            m_pattern(pattern) {}
        private:
            bool doEvaluate(const BrushFace* face) const override {
                const auto& textureName = face->textureName();
                auto begin = std::begin(textureName);

                const auto pos = textureName.find_last_of('/');
                if (pos != String::npos) {
                    std::advance(begin, long(pos)+1);
                }

                return StringUtils::matchesPattern(begin, std::end(textureName), std::begin(m_pattern), std::end(m_pattern), StringUtils::CharEqual<StringUtils::CaseInsensitiveCharCompare>());
            }
        };

        class ShaderSurfaceParmsEvaluator : public BrushFaceEvaluator {
        private:
            String m_pattern;
        public:
            explicit ShaderSurfaceParmsEvaluator(const String& pattern, const StringList& ignoreTexture) :
            BrushFaceEvaluator(ignoreTexture),
            m_pattern(pattern) {}
        private:
            bool doEvaluate(const BrushFace* face) const override {
                const auto* texture = face->texture();
                if (texture != nullptr) {
                    const auto& surfaceParms = texture->surfaceParms();
                    return surfaceParms.count(m_pattern) > 0;
                } else {
                    return false;
                }
            }
        };
        
        class ContentFlagsMatcher : public BrushFaceEvaluator {
        private:
            int m_flags;
        public:
            explicit ContentFlagsMatcher(const int flags, const StringList& ignoreTexture) :
            BrushFaceEvaluator(ignoreTexture),
            m_flags(flags) {}
        private:
            bool doEvaluate(const BrushFace* face) const override {
                return (face->surfaceContents() & m_flags) != 0;
            }
        };

        class SurfaceFlagsMatcher : public BrushFaceEvaluator {
        private:
            int m_flags;
        public:
            explicit SurfaceFlagsMatcher(const int flags, const StringList& ignoreTexture) :
            BrushFaceEvaluator(ignoreTexture),
            m_flags(flags) {}
        private:
            bool doEvaluate(const BrushFace* face) const override {
                return (face->surfaceFlags() & m_flags) != 0;
            }
        };

        class EntityClassnameEvaluator : public BrushContentTypeEvaluator {
        private:
            String m_pattern;
        public:
            explicit EntityClassnameEvaluator(const String& pattern) :
            m_pattern(pattern) {}
        private:
            bool doEvaluate(const Brush* brush) const override {
                const AttributableNode* entity = brush->entity();
                if (entity == nullptr) {
                    return false;
                }

                return StringUtils::caseInsensitiveMatchesPattern(entity->classname(), m_pattern);
            }
        };
        
        BrushContentTypeEvaluator::~BrushContentTypeEvaluator() = default;
   
        std::unique_ptr<BrushContentTypeEvaluator> BrushContentTypeEvaluator::textureNameEvaluator(const String& pattern, const StringList& ignoreTexture) {
            return std::make_unique<TextureNameEvaluator>(pattern, ignoreTexture);
        }

        std::unique_ptr<BrushContentTypeEvaluator> BrushContentTypeEvaluator::shaderSurfaceParmsEvaluator(const String& pattern, const StringList& ignoreTexture) {
            return std::make_unique<ShaderSurfaceParmsEvaluator>(pattern, ignoreTexture);
        }

        std::unique_ptr<BrushContentTypeEvaluator> BrushContentTypeEvaluator::contentFlagsEvaluator(const int value, const StringList& ignoreTexture) {
            return std::make_unique<ContentFlagsMatcher>(value, ignoreTexture);
        }

        std::unique_ptr<BrushContentTypeEvaluator> BrushContentTypeEvaluator::surfaceFlagsEvaluator(const int value, const StringList& ignoreTexture) {
            return std::make_unique<SurfaceFlagsMatcher>(value, ignoreTexture);
        }

        std::unique_ptr<BrushContentTypeEvaluator> BrushContentTypeEvaluator::entityClassnameEvaluator(const String& pattern) {
            return std::make_unique<EntityClassnameEvaluator>(pattern);
        }

        bool BrushContentTypeEvaluator::evaluate(const Brush* brush) const {
            return doEvaluate(brush);
        }
    }
}
