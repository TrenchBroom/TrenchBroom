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
    namespace Model {
        struct ModelSpecification {
            IO::Path path;
            size_t skinIndex;
            size_t frameIndex;
            
            ModelSpecification(const IO::Path& i_path, const size_t i_skinIndex = 0, const size_t i_frameIndex = 0);
        };
        
        class ModelDefinition {
        public:
            virtual ~ModelDefinition();
            
            bool matches(const EntityProperties& properties) const;
            ModelSpecification modelSpecification(const EntityProperties& properties) const;
        protected:
            ModelDefinition();
        private:
            virtual bool doMatches(const EntityProperties& properties) const = 0;
            virtual ModelSpecification doModelSpecification(const EntityProperties& properties) const = 0;
        };
        
        class StaticModelDefinitionMatcher {
        public:
            virtual ~StaticModelDefinitionMatcher();
            bool matches(const EntityProperties& properties) const;
        protected:
            StaticModelDefinitionMatcher();
        private:
            virtual bool doMatches(const EntityProperties& properties) const = 0;
        };
        
        class StaticModelDefinitionPropertyMatcher : public StaticModelDefinitionMatcher {
        private:
            PropertyKey m_key;
            PropertyValue m_value;
        public:
            StaticModelDefinitionPropertyMatcher(const PropertyKey& key, const PropertyValue& value);
        private:
            bool doMatches(const EntityProperties& properties) const;
        };
        
        class StaticModelDefinitionFlagMatcher : public StaticModelDefinitionMatcher {
        private:
            PropertyKey m_key;
            int m_value;
        public:
            StaticModelDefinitionFlagMatcher(const PropertyKey& key, const int value);
        private:
            bool doMatches(const EntityProperties& properties) const;
        };
        
        
        class StaticModelDefinition : public ModelDefinition {
        private:
            IO::Path m_path;
            size_t m_skinIndex;
            size_t m_frameIndex;
            StaticModelDefinitionMatcher* m_matcher;
        public:
            StaticModelDefinition(const IO::Path& path, const size_t skinIndex, const size_t frameIndex);
            StaticModelDefinition(const IO::Path& path, const size_t skinIndex, const size_t frameIndex, const PropertyKey& key, const PropertyValue& value);
            StaticModelDefinition(const IO::Path& path, const size_t skinIndex, const size_t frameIndex, const PropertyKey& key, const int value);
            ~StaticModelDefinition();
        private:
            bool doMatches(const EntityProperties& properties) const;
            ModelSpecification doModelSpecification(const EntityProperties& properties) const;
        };
        
        class DynamicModelDefinition : public ModelDefinition {
        private:
            PropertyKey m_pathKey;
            PropertyKey m_skinKey;
            PropertyKey m_frameKey;
        public:
            DynamicModelDefinition(const PropertyKey& pathKey, const PropertyKey& skinKey = "", const PropertyKey& frameKey = "");
        private:
            bool doMatches(const EntityProperties& properties) const;
            ModelSpecification doModelSpecification(const EntityProperties& properties) const;
        };
    }
}

#endif /* defined(__TrenchBroom__ModelDefinition__) */
