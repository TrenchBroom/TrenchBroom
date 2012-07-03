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
    IBOutlet NSMenu* textureActionMenu;
    IBOutlet NSMenu* objectActionMenu;
    IBOutlet NSMenuItem* actionMenuItem;
    
    EditorHolder* editorHolder;
    void* undoListener;
}

- (EditorHolder*)editorHolder;

- (IBAction)customUndo:(id)sender;
- (IBAction)customRedo:(id)sender;

- (IBAction)selectEntity:(id)sender;
- (IBAction)selectTouching:(id)sender;

- (IBAction)toggleVertexTool:(id)sender;
- (IBAction)toggleEdgeTool:(id)sender;
- (IBAction)toggleFaceTool:(id)sender;

- (IBAction)moveTexturesLeft:(id)sender;
- (IBAction)moveTexturesUp:(id)sender;
- (IBAction)moveTexturesRight:(id)sender;
- (IBAction)moveTexturesDown:(id)sender;
- (IBAction)rotateTexturesCW:(id)sender;
- (IBAction)rotateTexturesCCW:(id)sender;

- (IBAction)moveObjectsLeft:(id)sender;
- (IBAction)moveObjectsUp:(id)sender;
- (IBAction)moveObjectsRight:(id)sender;
- (IBAction)moveObjectsDown:(id)sender;
- (IBAction)moveObjectsToward:(id)sender;
- (IBAction)moveObjectsAway:(id)sender;
- (IBAction)rollObjectsCW:(id)sender;
- (IBAction)rollObjectsCCW:(id)sender;
- (IBAction)pitchObjectsCW:(id)sender;
- (IBAction)pitchObjectsCCW:(id)sender;
- (IBAction)yawObjectsCW:(id)sender;
- (IBAction)yawObjectsCCW:(id)sender;
- (IBAction)flipObjectsHorizontally:(id)sender;
- (IBAction)flipObjectsVertically:(id)sender;
- (IBAction)duplicateObjects:(id)sender;
- (IBAction)enlargeBrushes:(id)sender;

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

