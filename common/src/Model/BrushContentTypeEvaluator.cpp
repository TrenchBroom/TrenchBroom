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

#include "BrushContentTypeEvaluator.h"

#include "Macros.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/Entity.h"

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
            typedef enum {
                Mode_Exact,
                Mode_Prefix,
                Mode_Suffix
            } Mode;
            
            Mode m_mode;
            String m_pattern;
        public:
            TextureNameEvaluator(const String& pattern) {
                assert(!pattern.empty());
                if (pattern[0] == '*') {
                    m_mode = Mode_Suffix;
                    m_pattern = pattern.substr(1);
                } else if (pattern.size() > 1 &&
                           pattern[pattern.size() - 1] == '*' &&
                           pattern[pattern.size() - 2] != '\\') {
                    m_mode = Mode_Prefix;
                    m_pattern = pattern.substr(0, pattern.size() - 1);
                } else {
                    m_mode = Mode_Exact;
                    m_pattern = pattern;
                }
                m_pattern = StringUtils::replaceAll(m_pattern, "\\*", "*");
                assert(!m_pattern.empty());
            }
        private:
            bool doEvaluate(const BrushFace* face) const {
                switch (m_mode) {
                    case Mode_Exact:
                        return StringUtils::caseInsensitiveEqual(face->textureName(), m_pattern);
                    case Mode_Prefix:
                        return StringUtils::caseInsensitivePrefix(face->textureName(), m_pattern);
                    case Mode_Suffix:
                        return StringUtils::caseInsensitiveSuffix(face->textureName(), m_pattern);
                    DEFAULT_SWITCH()
                }
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
            String m_classname;
        public:
            EntityClassnameEvaluator(const String& classname) :
            m_classname(classname) {}
        private:
            bool doEvaluate(const Brush* brush) const {
                const Entity* entity = brush->parent();
                if (entity == NULL)
                    return false;
                return entity->classname() == m_classname;
            }
        };
        
        BrushContentTypeEvaluator::~BrushContentTypeEvaluator() {}
   
        BrushContentTypeEvaluator* BrushContentTypeEvaluator::textureNameEvaluator(const String& pattern) {
            return new TextureNameEvaluator(pattern);
        }
        
        BrushContentTypeEvaluator* BrushContentTypeEvaluator::contentFlagsEvaluator(const int value) {
            return new ContentFlagsEvaluator(value);
        }
        
        BrushContentTypeEvaluator* BrushContentTypeEvaluator::entityClassnameEvaluator(const String& classname) {
            return new EntityClassnameEvaluator(classname);
        }

        bool BrushContentTypeEvaluator::evaluate(const Brush* brush) const {
            return doEvaluate(brush);
        }
    }
}
