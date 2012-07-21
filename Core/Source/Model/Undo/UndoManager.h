/*
 Copyright (C) 2010-2012 Kristian Duske
 
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

#ifndef TrenchBroom_UndoManager_h
#define TrenchBroom_UndoManager_h

#include <string>
#include <vector>
#include "Utilities/Event.h"
#include "Model/Undo/UndoItem.h"
#include "Model/Undo/InvocationUndoItem.h"

namespace TrenchBroom {
    namespace Model {
        class UndoItem;
        class Map;
        
        class UndoGroup {
        private:
            std::string m_name;
            std::vector<UndoItem*> m_items;
        public:
            UndoGroup(const std::string name);
            ~UndoGroup();
            void addItem(UndoItem* item);
            void undo();
            const std::string& name();
            bool empty() const;
        };
        
        class UndoManager {
        private:
            typedef enum {
                TB_US_DEFAULT,
                TB_US_UNDO,
                TB_US_REDO
            } EUndoState;
            
            int m_depth;
            UndoGroup* m_currentGroup;
            std::vector<UndoGroup*> m_undoStack;
            std::vector<UndoGroup*> m_redoStack;
            EUndoState m_state;
        public:
            typedef Event<const UndoGroup&> UndoEvent;

            UndoEvent undoGroupCreated;
            UndoEvent undoPerformed;
            UndoEvent redoPerformed;
            
            UndoManager();
            ~UndoManager();
            void clear();
            void undo();
            void redo();
            void begin(const std::string& name);
            void end();
            void discard();
            void addItem(UndoItem* item);
            void addSnapshot(Map& map);
            
            void addFunctor(Map& map, void(Map::*function)()) {
                NoArgFunctor<Map>* functor = new NoArgFunctor<Map>(&map, function);
                FunctorUndoItem* undoItem = new FunctorUndoItem(map, functor);
                addItem(undoItem);
            }
            
            template <typename Arg1>
            void addFunctor(Map& map, void(Map::*function)(Arg1), Arg1 arg1) {
                OneArgFunctor<Map, Arg1>* functor = new OneArgFunctor<Map, Arg1>(&map, function, arg1);
                FunctorUndoItem* undoItem = new FunctorUndoItem(map, functor);
                addItem(undoItem);
            }
            
            template <typename Arg1, typename Arg2>
            void addFunctor(Map& map, void(Map::*function)(Arg1, Arg2), Arg1 arg1, Arg2 arg2) {
                TwoArgFunctor<Map, Arg1, Arg2>* functor = new TwoArgFunctor<Map, Arg1, Arg2>(&map, function, arg1, arg2);
                FunctorUndoItem* undoItem = new FunctorUndoItem(map, functor);
                addItem(undoItem);
            }
            
            template <typename Arg1, typename Arg2, typename Arg3>
            void addFunctor(Map& map, void(Map::*function)(Arg1, Arg2, Arg3), Arg1 arg1, Arg2 arg2, Arg3 arg3) {
                ThreeArgFunctor<Map, Arg1, Arg2, Arg3>* functor = new ThreeArgFunctor<Map, Arg1, Arg2, Arg3>(&map, function, arg1, arg2, arg3);
                FunctorUndoItem* undoItem = new FunctorUndoItem(map, functor);
                addItem(undoItem);
            }
            
            template <typename Arg1, typename Arg2, typename Arg3, typename Arg4>
            void addFunctor(Map& map, void(Map::*function)(Arg1, Arg2, Arg3, Arg4), Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4) {
                FourArgFunctor<Map, Arg1, Arg2, Arg3, Arg4>* functor = new FourArgFunctor<Map, Arg1, Arg2, Arg3, Arg4>(&map, function, arg1, arg2, arg3, arg4);
                FunctorUndoItem* undoItem = new FunctorUndoItem(map, functor);
                addItem(undoItem);
            }
            
            bool undoStackEmpty();
            bool redoStackEmpty();
            const std::string& topUndoName();
            const std::string& topRedoName();
        };
    }
}

#endif
