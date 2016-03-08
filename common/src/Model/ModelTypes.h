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
#include "Model/BrushGeometry.h"

#include <map>
#include <set>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        typedef size_t IdType;

        class Node;
        typedef std::set<Node*> NodeSet;
        typedef std::vector<Node*> NodeList;
        static const NodeList EmptyNodeList(0);
        
        typedef std::map<Node*, Node*> NodeMap;
        typedef std::map<Node*, NodeList> ParentChildrenMap;
        
        typedef enum {
            Visibility_Inherited = 1,
            Visibility_Hidden    = 2,
            Visibility_Shown     = 4
        } VisibilityState;
        
        typedef enum {
            Lock_Inherited = 1,
            Lock_Locked    = 2,
            Lock_Unlocked  = 4
        } LockState;

        typedef std::map<Node*, VisibilityState> VisibilityMap;
        typedef std::map<Node*, LockState> LockStateMap;

        class NodeVisitor;
        class ConstNodeVisitor;
        
        class World;
        
        class AttributableNode;
        typedef std::set<AttributableNode*> AttributableNodeSet;
        static const AttributableNodeSet EmptyAttributableNodeSet;
        typedef std::vector<AttributableNode*> AttributableNodeList;
        static const AttributableNodeList EmptyAttributableNodeList(0);
        
        class Layer;
        typedef std::vector<Layer*> LayerList;
        static const LayerList EmptyLayerList(0);
        
        class Group;
        typedef std::vector<Group*> GroupList;
        static const GroupList EmptyGroupList(0);
        typedef std::set<Group*> GroupSet;
        typedef std::map<Group*, String> GroupNameMap;
        
        class Entity;
        typedef std::vector<Entity*> EntityList;
        static const EntityList EmptyEntityList(0);
        typedef std::set<Entity*> EntitySet;
        
        class Brush;
        typedef std::vector<Brush*> BrushList;
        static const BrushList EmptyBrushList(0);
        typedef std::set<Brush*> BrushSet;
        static const BrushSet EmptyBrushSet;
        
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
        
        typedef std::set<BrushEdge*> BrushEdgeSet;
        static const BrushEdgeSet EmptyBrushEdgeSet;
        
        typedef std::map<Vec3, BrushSet, Vec3::LexicographicOrder> VertexToBrushesMap;
        typedef std::map<Vec3, BrushEdgeSet, Vec3::LexicographicOrder> VertexToEdgesMap;
        typedef std::map<Vec3, BrushFaceSet, Vec3::LexicographicOrder> VertexToFacesMap;
        typedef std::map<Model::Brush*, Vec3::List> BrushVerticesMap;
        typedef std::map<Model::Brush*, Edge3::List> BrushEdgesMap;
        typedef std::map<Model::Brush*, Polygon3::List> BrushFacesMap;

        class BrushFaceSnapshot;
        typedef std::vector<BrushFaceSnapshot*> BrushFaceSnapshotList;
        
        class NodeSnapshot;
        typedef std::vector<NodeSnapshot*> NodeSnapshotList;
        
        typedef int IssueType;

        class Issue;
        typedef std::vector<Issue*> IssueList;
        static const IssueList EmptyIssueList(0);

        class IssueQuickFix;
        typedef std::vector<IssueQuickFix*> IssueQuickFixList;
        
        class IssueGenerator;
        typedef std::vector<IssueGenerator*> IssueGeneratorList;
        
        class Game;
        typedef std::tr1::shared_ptr<Game> GamePtr;
    }
}

#endif
