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

#import <Cocoa/Cocoa.h>

@class EditorHolder;

@interface MapDocument : NSDocument {
    EditorHolder* editorHolder;
    void* undoListener;
}

- (EditorHolder*)editorHolder;

- (IBAction)customUndo:(id)sender;
- (IBAction)customRedo:(id)sender;


- (IBAction)selectEntity:(id)sender;
- (IBAction)selectTouching:(id)sender;

- (IBAction)toggleGrid:(id)sender;
- (IBAction)toggleSnapToGrid:(id)sender;
- (IBAction)setGridSize:(id)sender;
@end

namespace TrenchBroom {
    namespace Model {
        class UndoManager;
        class UndoGroup;
    }
    
    namespace Controller {
        class UndoListener {
        private:
            Model::UndoManager& m_undoManager;
            MapDocument* m_mapDocument;
        public:
            UndoListener(Model::UndoManager& undoManager, MapDocument* mapDocument);
            ~UndoListener();
            void undoGroupCreated(const Model::UndoGroup& group);
            void undoPerformed(const Model::UndoGroup& group);
            void redoPerformed(const Model::UndoGroup& group);
        };
    }
}

