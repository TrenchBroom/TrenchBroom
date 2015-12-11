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

#ifndef TrenchBroom_ModelDefinition
#define TrenchBroom_ModelDefinition

#include "IO/Path.h"
#include "Model/EntityAttributes.h"

namespace TrenchBroom {
    namespace Assets {
        struct ModelSpecification {
            IO::Path path;
            size_t skinIndex;
            size_t frameIndex;
            
            ModelSpecification();
            ModelSpecification(const IO::Path& i_path, const size_t i_skinIndex = 0, const size_t i_frameIndex = 0);
            bool operator<(const ModelSpecification& rhs) const;
            
            const String asString() const;
        };
        
        class ModelDefinition {
        public:
            virtual ~ModelDefinition();
            
            bool matches(const Model::EntityAttributes& attributes) const;
            ModelSpecification modelSpecification(const Model::EntityAttributes& attributes) const;
            ModelSpecification defaultModelSpecification() const;
        protected:
            ModelDefinition();
        private:
            virtual bool doMatches(const Model::EntityAttributes& attributes) const = 0;
            virtual ModelSpecification doModelSpecification(const Model::EntityAttributes& attributes) const = 0;
            virtual ModelSpecification doDefaultModelSpecification() const = 0;
        };
        
        class StaticModelDefinitionMatcher {
        public:
            virtual ~StaticModelDefinitionMatcher();
            bool matches(const Model::EntityAttributes& attributes) const;
        protected:
            StaticModelDefinitionMatcher();
        private:
            virtual bool doMatches(const Model::EntityAttributes& attributes) const = 0;
        };
        
        class StaticModelDefinitionAttributeMatcher : public StaticModelDefinitionMatcher {
        private:
            Model::AttributeName m_attributeName;
            Model::AttributeValue m_attributeValue;
        public:
            StaticModelDefinitionAttributeMatcher(const Model::AttributeName& attributeName, const Model::AttributeValue& attributeValue);
        private:
            bool doMatches(const Model::EntityAttributes& attributes) const;
        };
        
        class StaticModelDefinitionFlagMatcher : public StaticModelDefinitionMatcher {
        private:
            Model::AttributeName m_attributeName;
            int m_attributeValue;
        public:
            StaticModelDefinitionFlagMatcher(const Model::AttributeName& attributeName, const int attributeValue);
        private:
            bool doMatches(const Model::EntityAttributes& attributes) const;
        };
        
        
        class StaticModelDefinition : public ModelDefinition {
        private:
            IO::Path m_path;
            size_t m_skinIndex;
            size_t m_frameIndex;
            StaticModelDefinitionMatcher* m_matcher;
        public:
            StaticModelDefinition(const IO::Path& path, const size_t skinIndex, const size_t frameIndex);
            StaticModelDefinition(const IO::Path& path, const size_t skinIndex, const size_t frameIndex, const Model::AttributeName& attributeName, const Model::AttributeValue& attributeValue);
            StaticModelDefinition(const IO::Path& path, const size_t skinIndex, const size_t frameIndex, const Model::AttributeName& attributeName, const int attributeValue);
            ~StaticModelDefinition();
        private:
            bool doMatches(const Model::EntityAttributes& attributes) const;
            ModelSpecification doModelSpecification(const Model::EntityAttributes& attributes) const;
            ModelSpecification doDefaultModelSpecification() const;
        };
        
        class DynamicModelDefinition : public ModelDefinition {
        private:
            Model::AttributeName m_pathAttributeName;
            Model::AttributeName m_skinAttributeName;
            Model::AttributeName m_frameAttributeName;
        public:
            DynamicModelDefinition(const Model::AttributeName& pathAttributeName, const Model::AttributeName& skinAttributeName = "", const Model::AttributeName& frameAttributeName = "");
        private:
            bool doMatches(const Model::EntityAttributes& attributes) const;
            ModelSpecification doModelSpecification(const Model::EntityAttributes& attributes) const;
            ModelSpecification doDefaultModelSpecification() const;
        };
    }
}

#endif /* defined(TrenchBroom_ModelDefinition) */
