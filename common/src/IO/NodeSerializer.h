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

#ifndef TrenchBroom_NodeSerializer
#define TrenchBroom_NodeSerializer

#include "Model/EntityAttributes.h"
#include "Model/ModelTypes.h"

#include <map>
#include <memory>

namespace TrenchBroom {
    namespace IO {
        class Path;
        
        class NodeSerializer {
        private:
            class BrushSerializer;
        protected:
            static const int FloatPrecision = 17;
        private:
            template <typename T>
            class IdManager {
            private:
                typedef std::map<T, String> IdMap;
                mutable IdMap m_ids;
            public:
                const String& getId(const T& t) const {
                    typename IdMap::iterator it = m_ids.find(t);
                    if (it == m_ids.end())
                        it = m_ids.insert(std::make_pair(t, idToString(makeId()))).first;
                    return it->second;
                }
            private:
                Model::IdType makeId() const {
                    static Model::IdType currentId = 1;
                    return currentId++;
                }
                
                String idToString(const Model::IdType nodeId) const {
                    StringStream str;
                    str << nodeId;
                    return str.str();
                }
            };
            
            typedef IdManager<const Model::Layer*> LayerIds;
            typedef IdManager<const Model::Group*> GroupIds;
            
            LayerIds m_layerIds;
            GroupIds m_groupIds;
        public:
            typedef std::auto_ptr<NodeSerializer> Ptr;
            
            virtual ~NodeSerializer();

            void defaultLayer(Model::World* world);
            void customLayer(Model::Layer* layer);
            void group(Model::Group* group, const Model::EntityAttribute::List& parentAttributes);
            
            void entity(Model::Node* node, const Model::EntityAttribute::List& attributes, const Model::EntityAttribute::List& parentAttributes, Model::Node* brushParent);
            void entity(Model::Node* node, const Model::EntityAttribute::List& attributes, const Model::EntityAttribute::List& parentAttributes, const Model::BrushList& entityBrushes);
        private:
            void beginEntity(const Model::Node* node, const Model::EntityAttribute::List& attributes, const Model::EntityAttribute::List& extraAttributes);
            void beginEntity(const Model::Node* node);
            void endEntity(Model::Node* node);
            
            void entityAttributes(const Model::EntityAttribute::List& attributes);
            void entityAttribute(const Model::EntityAttribute& attribute);

            void brushes(const Model::BrushList& brushes);
            void brush(Model::Brush* brush);
            
            void beginBrush(const Model::Brush* brush);
            void endBrush(Model::Brush* brush);
        public:
            void brushFaces(const Model::BrushFaceList& faces);
        private:
            void brushFace(Model::BrushFace* face);
        private:
            class GetParentAttributes;
        public:
            Model::EntityAttribute::List parentAttributes(const Model::Node* node);
        private:
            Model::EntityAttribute::List layerAttributes(const Model::Layer* layer);
            Model::EntityAttribute::List groupAttributes(const Model::Group* group);
        private:
            virtual void doBeginEntity(const Model::Node* node) = 0;
            virtual void doEndEntity(Model::Node* node) = 0;
            virtual void doEntityAttribute(const Model::EntityAttribute& attribute) = 0;
            
            virtual void doBeginBrush(const Model::Brush* brush) = 0;
            virtual void doEndBrush(Model::Brush* brush) = 0;
            virtual void doBrushFace(Model::BrushFace* face) = 0;
        };
    }
}

#endif /* defined(TrenchBroom_NodeSerializer) */
