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

#ifndef TrenchBroom_ModelTypes_h
#define TrenchBroom_ModelTypes_h

#include "TrenchBroom.h"
#include "VecMath.h"
#include "StringUtils.h"
#include "SharedPointer.h"
#include "Model/BrushGeometryTypes.h"

#include <map>
#include <set>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class Node;
        typedef std::set<Node*> NodeSet;
        typedef std::vector<Node*> NodeList;
        static const NodeList EmptyNodeList(0);
        
        typedef std::map<Node*, NodeList> ParentChildrenMap;
        
        class NodeVisitor;
        class ConstNodeVisitor;
        
        class World;
        
        class Attributable;
        typedef std::set<Attributable*> AttributableSet;
        static const AttributableSet EmptyAttributableSet;
        typedef std::vector<Attributable*> AttributableList;
        static const AttributableList EmptyAttributableList(0);
        
        class Layer;
        typedef std::vector<Layer*> LayerList;
        static const LayerList EmptyLayerList(0);
        
        class Group;
        typedef std::vector<Group*> GroupList;
        static const GroupList EmptyGroupList(0);
        
        class Entity;
        typedef std::vector<Entity*> EntityList;
        static const EntityList EmptyEntityList(0);
        typedef std::set<Entity*> EntitySet;
        
        class Brush;
        typedef std::vector<Brush*> BrushList;
        static const BrushList EmptyBrushList(0);
        typedef std::set<Brush*> BrushSet;
        
        class Object;
        
        class BrushFace;
        typedef std::set<BrushFace*> BrushFaceSet;
        static const BrushFaceSet EmptyBrushFaceSet;
        typedef std::vector<BrushFace*> BrushFaceList;
        static const BrushFaceList EmptyBrushFaceList(0);
        
        typedef String AttributeName;
        typedef std::vector<AttributeName> AttributeNameList;
        typedef std::set<AttributeName> AttributeNameSet;
        typedef String AttributeValue;
        typedef std::vector<AttributeValue> AttributeValueList;
        
        typedef std::map<Vec3, BrushList, Vec3::LexicographicOrder> VertexToBrushesMap;
        typedef std::map<Vec3, BrushEdgeList, Vec3::LexicographicOrder> VertexToEdgesMap;
        typedef std::map<Vec3, BrushFaceList, Vec3::LexicographicOrder> VertexToFacesMap;
        typedef std::map<Model::Brush*, Vec3::List> BrushVerticesMap;

        class BrushFaceSnapshot;
        typedef std::vector<BrushFaceSnapshot*> BrushFaceSnapshotList;
        
        class NodeSnapshot;
        typedef std::vector<NodeSnapshot*> NodeSnapshotList;
        
        typedef size_t IssueType;

        class Issue;
        typedef std::vector<Issue*> IssueList;
        static const IssueList EmptyIssueList(0);

        class IssueGenerator;
        typedef std::vector<IssueGenerator*> IssueGeneratorList;
        
        class Game;
        typedef std::tr1::shared_ptr<Game> GamePtr;
    }
}

#endif
