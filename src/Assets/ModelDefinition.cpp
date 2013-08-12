/*
 Copyright (C) 2010-2013 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
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

        bool ModelSpecification::operator< (const ModelSpecification& rhs) const {
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

        ModelDefinition::~ModelDefinition() {}
        
        bool ModelDefinition::matches(const Model::EntityProperties& properties) const {
            return doMatches(properties);
        }
        
        ModelSpecification ModelDefinition::modelSpecification(const Model::EntityProperties& properties) const {
            assert(matches(properties));
            return doModelSpecification(properties);
        }

        ModelSpecification ModelDefinition::defaultModelSpecification() const {
            return doDefaultModelSpecification();
        }

        ModelDefinition::ModelDefinition() {}

        StaticModelDefinitionMatcher::~StaticModelDefinitionMatcher() {}
        
        bool StaticModelDefinitionMatcher::matches(const Model::EntityProperties& properties) const {
            return doMatches(properties);
        }

        StaticModelDefinitionMatcher::StaticModelDefinitionMatcher() {}

        StaticModelDefinitionPropertyMatcher::StaticModelDefinitionPropertyMatcher(const Model::PropertyKey& key, const Model::PropertyValue& value) :
        m_key(key),
        m_value(value) {}
        
        bool StaticModelDefinitionPropertyMatcher::doMatches(const Model::EntityProperties& properties) const {
            const Model::PropertyValue* propertyValue = properties.property(m_key);
            if (propertyValue == NULL)
                return false;
            return *propertyValue == m_value;
        }

        StaticModelDefinitionFlagMatcher::StaticModelDefinitionFlagMatcher(const Model::PropertyKey& key, const int value) :
        m_key(key),
        m_value(value) {}
        
        bool StaticModelDefinitionFlagMatcher::doMatches(const Model::EntityProperties& properties) const {
            const Model::PropertyValue* propertyValue = properties.property(m_key);
            if (propertyValue == NULL)
                return false;
            const int flagValue = std::atoi(propertyValue->c_str());
            return (flagValue & m_value) != 0;
        }

        StaticModelDefinition::StaticModelDefinition(const IO::Path& path, const size_t skinIndex, const size_t frameIndex) :
        m_path(path),
        m_skinIndex(skinIndex),
        m_frameIndex(frameIndex),
        m_matcher(NULL) {}
        
        StaticModelDefinition::StaticModelDefinition(const IO::Path& path, const size_t skinIndex, const size_t frameIndex, const Model::PropertyKey& key, const Model::PropertyValue& value) :
        m_path(path),
        m_skinIndex(skinIndex),
        m_frameIndex(frameIndex),
        m_matcher(new StaticModelDefinitionPropertyMatcher(key, value)) {}
        
        StaticModelDefinition::StaticModelDefinition(const IO::Path& path, const size_t skinIndex, const size_t frameIndex, const Model::PropertyKey& key, const int value) :
        m_path(path),
        m_skinIndex(skinIndex),
        m_frameIndex(frameIndex),
        m_matcher(new StaticModelDefinitionFlagMatcher(key, value)) {}

        StaticModelDefinition::~StaticModelDefinition() {
            delete m_matcher;
            m_matcher = NULL;
        }

        bool StaticModelDefinition::doMatches(const Model::EntityProperties& properties) const {
            if (m_matcher == NULL)
                return true;
            return m_matcher->matches(properties);
        }
        
        ModelSpecification StaticModelDefinition::doModelSpecification(const Model::EntityProperties& properties) const {
            return ModelSpecification(m_path, m_skinIndex, m_frameIndex);
        }

        ModelSpecification StaticModelDefinition::doDefaultModelSpecification() const {
            return ModelSpecification(m_path, m_skinIndex, m_frameIndex);
        }
        
        DynamicModelDefinition::DynamicModelDefinition(const Model::PropertyKey& pathKey, const Model::PropertyKey& skinKey, const Model::PropertyKey& frameKey) :
        m_pathKey(pathKey),
        m_skinKey(skinKey),
        m_frameKey(frameKey) {
            assert(!m_pathKey.empty());
        }

        bool DynamicModelDefinition::doMatches(const Model::EntityProperties& properties) const {
            const Model::PropertyValue* pathValue = properties.property(m_pathKey);
            if (pathValue == NULL || pathValue->empty())
                return false;
            if (!m_skinKey.empty()) {
                const Model::PropertyValue* skinValue = properties.property(m_skinKey);
                if (skinValue == NULL || skinValue->empty())
                    return false;
            }
            if (!m_frameKey.empty()) {
                const Model::PropertyValue* frameValue = properties.property(m_frameKey);
                if (frameValue == NULL || frameValue->empty())
                    return false;
            }
            return true;
        }
        
        ModelSpecification DynamicModelDefinition::doModelSpecification(const Model::EntityProperties& properties) const {
            const Model::PropertyValue* pathValue = properties.property(m_pathKey);
            assert(pathValue != NULL);
            const IO::Path path(*pathValue);
            
            size_t skinIndex = 0;
            if (!m_skinKey.empty()) {
                const Model::PropertyValue* skinValue = properties.property(m_skinKey);
                assert(skinValue != NULL);
                skinIndex = std::atoi(skinValue->c_str());
            }
            
            size_t frameIndex = 0;
            if (!m_frameKey.empty()) {
                const Model::PropertyValue* frameValue = properties.property(m_frameKey);
                assert(frameValue != NULL);
                frameIndex = std::atoi(frameValue->c_str());
            }
            
            return ModelSpecification(path, skinIndex, frameIndex);
        }

        ModelSpecification DynamicModelDefinition::doDefaultModelSpecification() const {
            return ModelSpecification(IO::Path(""), 0, 0);
        }
    }
}
