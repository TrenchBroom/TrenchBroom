/*
 Copyright (C) 2010-2016 Kristian Duske
 
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
#include "Model/AttributableNode.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"

namespace TrenchBroom {
    namespace Model {
        class BrushFaceEvaluator : public BrushContentTypeEvaluator {
        public:
            virtual ~BrushFaceEvaluator() {}
        private:
            bool doEvaluate(const Brush* brush) const {
                const Model::BrushFaceList& faces = brush->faces();
                Model::BrushFaceList::const_iterator it, end;
                for (it = faces.begin(), end = faces.end(); it != end; ++it) {
                    const Model::BrushFace* face = *it;
                    if (!doEvaluate(face))
                        return false;
                }
                return true;
            }
            
            virtual bool doEvaluate(const BrushFace* brush) const = 0;
        };
        
        class TextureNameEvaluator : public BrushFaceEvaluator {
        private:
            StringUtils::CaseSensitiveStringMatcher m_matcher;
        public:
            TextureNameEvaluator(const String& pattern) :
            m_matcher(pattern) {}
        private:
            bool doEvaluate(const BrushFace* face) const {
                const String& textureName = face->textureName();
                const size_t pos = textureName.find_last_of('/');
                if (pos == String::npos)
                    return m_matcher.matches(textureName);
                return m_matcher.matches(textureName.substr(pos + 1));
            }
        };
        
        class ContentFlagsEvaluator : public BrushFaceEvaluator {
        private:
            int m_flags;
        public:
            ContentFlagsEvaluator(const int flags) :
            m_flags(flags) {}
        private:
            bool doEvaluate(const BrushFace* face) const {
                return (face->surfaceContents() & m_flags) != 0;
            }
        };
        
        class EntityClassnameEvaluator : public BrushContentTypeEvaluator {
        private:
            StringUtils::CaseSensitiveStringMatcher m_matcher;
        public:
            EntityClassnameEvaluator(const String& pattern) :
            m_matcher(pattern) {}
        private:
            bool doEvaluate(const Brush* brush) const {
                const AttributableNode* entity = brush->entity();
                if (entity == NULL)
                    return false;
                
                return m_matcher.matches(entity->classname());
            }
        };
        
        BrushContentTypeEvaluator::~BrushContentTypeEvaluator() {}
   
        BrushContentTypeEvaluator* BrushContentTypeEvaluator::textureNameEvaluator(const String& pattern) {
            return new TextureNameEvaluator(pattern);
        }
        
        BrushContentTypeEvaluator* BrushContentTypeEvaluator::contentFlagsEvaluator(const int value) {
            return new ContentFlagsEvaluator(value);
        }

        BrushContentTypeEvaluator* BrushContentTypeEvaluator::entityClassnameEvaluator(const String& pattern) {
            return new EntityClassnameEvaluator(pattern);
        }

        bool BrushContentTypeEvaluator::evaluate(const Brush* brush) const {
            return doEvaluate(brush);
        }
    }
}
