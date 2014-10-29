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

#ifndef __TrenchBroom__QuakeReader__
#define __TrenchBroom__QuakeReader__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "IO/QuakeMapParser.h"
#include "Model/ModelTypes.h"

namespace TrenchBroom {
    namespace Model {
        class ModelFactory;
    }
    
    namespace IO {
        class QuakeReader : public QuakeMapParser {
        protected:
            class ParentInfo {
            public:
                typedef enum {
                    Type_Layer,
                    Type_Group,
                    Type_None
                } Type;
                
                Type m_type;
                String m_name;
            public:
                static ParentInfo layer(const String& name);
                static ParentInfo group(const String& name);
            private:
                ParentInfo(Type type, const String& name);
            public:
                bool layer() const;
                bool group() const;
                const String& name() const;
            };
        private:
            typedef enum {
                EntityType_Layer,
                EntityType_Group,
                EntityType_Worldspawn,
                EntityType_Default
            } EntityType;
            
            typedef std::map<String, Model::Layer*> LayerMap;
            typedef std::map<String, Model::Group*> GroupMap;
            
            typedef std::pair<Model::Node*, ParentInfo> NodeParentPair;
            typedef std::vector<NodeParentPair> NodeParentList;
            
            BBox3 m_worldBounds;
            Model::ModelFactory* m_factory;
            
            Model::Node* m_brushParent;
            Model::Node* m_currentNode;
            Model::BrushFaceList m_faces;
            
            LayerMap m_layers;
            GroupMap m_groups;
            NodeParentList m_unresolvedNodes;
        protected:
            QuakeReader(const char* begin, const char* end, Logger* logger = NULL);
            QuakeReader(const String& str, Logger* logger = NULL);
            
            void read(const BBox3& worldBounds);
        public:
            virtual ~QuakeReader();
        private: // implement MapParser interface
            void onFormatDetected(Model::MapFormat::Type format);
            void onBeginEntity(size_t line, const Model::EntityAttribute::List& attributes, const ExtraAttributes& extraAttributes);
            void onEndEntity(size_t startLine, size_t lineCount);
            void onBeginBrush(size_t line);
            void onEndBrush(size_t startLine, size_t lineCount, const ExtraAttributes& extraAttributes);
            void onBrushFace(size_t line, const Vec3& point1, const Vec3& point2, const Vec3& point3, const Model::BrushFaceAttributes& attribs, const Vec3& texAxisX, const Vec3& texAxisY);
        private: // helper methods
            void createLayer(size_t line, const Model::EntityAttribute::List& attributes, const ExtraAttributes& extraAttributes);
            void createGroup(size_t line, const Model::EntityAttribute::List& attributes, const ExtraAttributes& extraAttributes);
            void createEntity(size_t line, const Model::EntityAttribute::List& attributes, const ExtraAttributes& extraAttributes);
            void createBrush(size_t startLine, size_t lineCount, const ExtraAttributes& extraAttributes);

            ParentInfo::Type storeNode(Model::Node* node, const Model::EntityAttribute::List& attributes);
            void stripParentAttributes(Model::Attributable* attributable, ParentInfo::Type parentType);
            
            void resolveNodes();
            Model::Node* resolveParent(const ParentInfo& parentInfo) const;
            
            EntityType entityType(const Model::EntityAttribute::List& attributes) const;
            bool isLayer(const String& classname, const Model::EntityAttribute::List& attributes) const;
            bool isGroup(const String& classname, const Model::EntityAttribute::List& attributes) const;
            bool isWorldspawn(const String& classname, const Model::EntityAttribute::List& attributes) const;

            const String& findAttribute(const Model::EntityAttribute::List& attributes, const String& name, const String& defaultValue = EmptyString) const;
            
            void setFilePosition(Model::Node* node, size_t startLine, size_t lineCount);
        protected:
            void setExtraAttributes(Model::Node* node, const ExtraAttributes& extraAttributes);
        private: // subclassing interface
            virtual Model::ModelFactory* initialize(Model::MapFormat::Type format) = 0;
            virtual void onWorldspawn(const Model::EntityAttribute::List& attributes, const ExtraAttributes& extraAttributes) = 0;
            virtual void onWorldspawnFilePosition(size_t startLine, size_t lineCount) = 0;
            virtual void onLayer(Model::Layer* layer) = 0;
            virtual void onNode(Model::Node* parent, Model::Node* node) = 0;
            virtual void onUnresolvedNode(const ParentInfo& parentInfo, Model::Node* node) = 0;
            virtual void onBrush(Model::Node* parent, Model::Brush* brush) = 0;
            virtual void onBrushFace(Model::Brush* brush, Model::BrushFace* face) = 0;
        };
    }
}

#endif /* defined(__TrenchBroom__QuakeReader__) */
