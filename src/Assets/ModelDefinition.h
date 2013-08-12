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

#ifndef __TrenchBroom__ModelDefinition__
#define __TrenchBroom__ModelDefinition__

#include "IO/Path.h"
#include "Model/EntityProperties.h"

namespace TrenchBroom {
    namespace Assets {
        struct ModelSpecification {
            IO::Path path;
            size_t skinIndex;
            size_t frameIndex;
            
            ModelSpecification();
            ModelSpecification(const IO::Path& i_path, const size_t i_skinIndex = 0, const size_t i_frameIndex = 0);
            bool operator< (const ModelSpecification& rhs) const;
        };
        
        class ModelDefinition {
        public:
            virtual ~ModelDefinition();
            
            bool matches(const Model::EntityProperties& properties) const;
            ModelSpecification modelSpecification(const Model::EntityProperties& properties) const;
            ModelSpecification defaultModelSpecification() const;
        protected:
            ModelDefinition();
        private:
            virtual bool doMatches(const Model::EntityProperties& properties) const = 0;
            virtual ModelSpecification doModelSpecification(const Model::EntityProperties& properties) const = 0;
            virtual ModelSpecification doDefaultModelSpecification() const = 0;
        };
        
        class StaticModelDefinitionMatcher {
        public:
            virtual ~StaticModelDefinitionMatcher();
            bool matches(const Model::EntityProperties& properties) const;
        protected:
            StaticModelDefinitionMatcher();
        private:
            virtual bool doMatches(const Model::EntityProperties& properties) const = 0;
        };
        
        class StaticModelDefinitionPropertyMatcher : public StaticModelDefinitionMatcher {
        private:
            Model::PropertyKey m_key;
            Model::PropertyValue m_value;
        public:
            StaticModelDefinitionPropertyMatcher(const Model::PropertyKey& key, const Model::PropertyValue& value);
        private:
            bool doMatches(const Model::EntityProperties& properties) const;
        };
        
        class StaticModelDefinitionFlagMatcher : public StaticModelDefinitionMatcher {
        private:
            Model::PropertyKey m_key;
            int m_value;
        public:
            StaticModelDefinitionFlagMatcher(const Model::PropertyKey& key, const int value);
        private:
            bool doMatches(const Model::EntityProperties& properties) const;
        };
        
        
        class StaticModelDefinition : public ModelDefinition {
        private:
            IO::Path m_path;
            size_t m_skinIndex;
            size_t m_frameIndex;
            StaticModelDefinitionMatcher* m_matcher;
        public:
            StaticModelDefinition(const IO::Path& path, const size_t skinIndex, const size_t frameIndex);
            StaticModelDefinition(const IO::Path& path, const size_t skinIndex, const size_t frameIndex, const Model::PropertyKey& key, const Model::PropertyValue& value);
            StaticModelDefinition(const IO::Path& path, const size_t skinIndex, const size_t frameIndex, const Model::PropertyKey& key, const int value);
            ~StaticModelDefinition();
        private:
            bool doMatches(const Model::EntityProperties& properties) const;
            ModelSpecification doModelSpecification(const Model::EntityProperties& properties) const;
            ModelSpecification doDefaultModelSpecification() const;
        };
        
        class DynamicModelDefinition : public ModelDefinition {
        private:
            Model::PropertyKey m_pathKey;
            Model::PropertyKey m_skinKey;
            Model::PropertyKey m_frameKey;
        public:
            DynamicModelDefinition(const Model::PropertyKey& pathKey, const Model::PropertyKey& skinKey = "", const Model::PropertyKey& frameKey = "");
        private:
            bool doMatches(const Model::EntityProperties& properties) const;
            ModelSpecification doModelSpecification(const Model::EntityProperties& properties) const;
            ModelSpecification doDefaultModelSpecification() const;
        };
    }
}

#endif /* defined(__TrenchBroom__ModelDefinition__) */
