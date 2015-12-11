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

#include "ModelDefinition.h"

#include <cassert>

namespace TrenchBroom {
    namespace Assets {
        ModelSpecification::ModelSpecification() :
        path(""),
        skinIndex(0),
        frameIndex(0) {}

        ModelSpecification::ModelSpecification(const IO::Path& i_path, const size_t i_skinIndex, const size_t i_frameIndex) :
        path(i_path),
        skinIndex(i_skinIndex),
        frameIndex(i_frameIndex) {}

        bool ModelSpecification::operator<(const ModelSpecification& rhs) const {
            const int pathCmp = path.compare(rhs.path);
            if (pathCmp < 0)
                return true;
            if (pathCmp > 0)
                return false;
            if (skinIndex < rhs.skinIndex)
                return true;
            if (skinIndex > rhs.skinIndex)
                return false;
            return frameIndex < rhs.frameIndex;
        }

        const String ModelSpecification::asString() const {
            StringStream str;
            str << path.asString() << ":" << skinIndex << ":" << frameIndex;
            return str.str();
        }

        ModelDefinition::~ModelDefinition() {}
        
        bool ModelDefinition::matches(const Model::EntityAttributes& attributes) const {
            return doMatches(attributes);
        }
        
        ModelSpecification ModelDefinition::modelSpecification(const Model::EntityAttributes& attributes) const {
            assert(matches(attributes));
            return doModelSpecification(attributes);
        }

        ModelSpecification ModelDefinition::defaultModelSpecification() const {
            return doDefaultModelSpecification();
        }

        ModelDefinition::ModelDefinition() {}

        StaticModelDefinitionMatcher::~StaticModelDefinitionMatcher() {}
        
        bool StaticModelDefinitionMatcher::matches(const Model::EntityAttributes& attributes) const {
            return doMatches(attributes);
        }

        StaticModelDefinitionMatcher::StaticModelDefinitionMatcher() {}

        StaticModelDefinitionAttributeMatcher::StaticModelDefinitionAttributeMatcher(const Model::AttributeName& attributeName, const Model::AttributeValue& attributeValue) :
        m_attributeName(attributeName),
        m_attributeValue(attributeValue) {}
        
        bool StaticModelDefinitionAttributeMatcher::doMatches(const Model::EntityAttributes& attributes) const {
            const Model::AttributeValue* attributeValue = attributes.attribute(m_attributeName);
            if (attributeValue == NULL)
                return false;
            return *attributeValue == m_attributeValue;
        }

        StaticModelDefinitionFlagMatcher::StaticModelDefinitionFlagMatcher(const Model::AttributeName& attributeName, const int attributeValue) :
        m_attributeName(attributeName),
        m_attributeValue(attributeValue) {}
        
        bool StaticModelDefinitionFlagMatcher::doMatches(const Model::EntityAttributes& attributes) const {
            const Model::AttributeValue* attributeValue = attributes.attribute(m_attributeName);
            if (attributeValue == NULL)
                return false;
            const int flagValue = std::atoi(attributeValue->c_str());
            return (flagValue & m_attributeValue) != 0;
        }

        StaticModelDefinition::StaticModelDefinition(const IO::Path& path, const size_t skinIndex, const size_t frameIndex) :
        m_path(path),
        m_skinIndex(skinIndex),
        m_frameIndex(frameIndex),
        m_matcher(NULL) {}
        
        StaticModelDefinition::StaticModelDefinition(const IO::Path& path, const size_t skinIndex, const size_t frameIndex, const Model::AttributeName& attributeName, const Model::AttributeValue& attributeValue) :
        m_path(path),
        m_skinIndex(skinIndex),
        m_frameIndex(frameIndex),
        m_matcher(new StaticModelDefinitionAttributeMatcher(attributeName, attributeValue)) {}
        
        StaticModelDefinition::StaticModelDefinition(const IO::Path& path, const size_t skinIndex, const size_t frameIndex, const Model::AttributeName& attributeName, const int attributeValue) :
        m_path(path),
        m_skinIndex(skinIndex),
        m_frameIndex(frameIndex),
        m_matcher(new StaticModelDefinitionFlagMatcher(attributeName, attributeValue)) {}

        StaticModelDefinition::~StaticModelDefinition() {
            delete m_matcher;
            m_matcher = NULL;
        }

        bool StaticModelDefinition::doMatches(const Model::EntityAttributes& attributes) const {
            if (m_matcher == NULL)
                return true;
            return m_matcher->matches(attributes);
        }
        
        ModelSpecification StaticModelDefinition::doModelSpecification(const Model::EntityAttributes& attributes) const {
            return ModelSpecification(m_path, m_skinIndex, m_frameIndex);
        }

        ModelSpecification StaticModelDefinition::doDefaultModelSpecification() const {
            return ModelSpecification(m_path, m_skinIndex, m_frameIndex);
        }
        
        DynamicModelDefinition::DynamicModelDefinition(const Model::AttributeName& pathAttributeName, const Model::AttributeName& skinAttributeName, const Model::AttributeName& frameAttributeName) :
        m_pathAttributeName(pathAttributeName),
        m_skinAttributeName(skinAttributeName),
        m_frameAttributeName(frameAttributeName) {
            assert(!m_pathAttributeName.empty());
        }

        bool DynamicModelDefinition::doMatches(const Model::EntityAttributes& attributes) const {
            const Model::AttributeValue* pathValue = attributes.attribute(m_pathAttributeName);
            if (pathValue == NULL || pathValue->empty())
                return false;
            if (!m_skinAttributeName.empty()) {
                const Model::AttributeValue* skinValue = attributes.attribute(m_skinAttributeName);
                if (skinValue == NULL || skinValue->empty())
                    return false;
            }
            if (!m_frameAttributeName.empty()) {
                const Model::AttributeValue* frameValue = attributes.attribute(m_frameAttributeName);
                if (frameValue == NULL || frameValue->empty())
                    return false;
            }
            return true;
        }
        
        ModelSpecification DynamicModelDefinition::doModelSpecification(const Model::EntityAttributes& attributes) const {
            const Model::AttributeValue* pathValue = attributes.attribute(m_pathAttributeName);
            assert(pathValue != NULL);
            const IO::Path path(*pathValue);
            
            size_t skinIndex = 0;
            if (!m_skinAttributeName.empty()) {
                const Model::AttributeValue* skinValue = attributes.attribute(m_skinAttributeName);
                assert(skinValue != NULL);
                skinIndex = StringUtils::stringToSize(*skinValue);
            }
            
            size_t frameIndex = 0;
            if (!m_frameAttributeName.empty()) {
                const Model::AttributeValue* frameValue = attributes.attribute(m_frameAttributeName);
                assert(frameValue != NULL);
                frameIndex = StringUtils::stringToSize(*frameValue);
            }
            
            return ModelSpecification(path, skinIndex, frameIndex);
        }

        ModelSpecification DynamicModelDefinition::doDefaultModelSpecification() const {
            return ModelSpecification(IO::Path(""), 0, 0);
        }
    }
}
