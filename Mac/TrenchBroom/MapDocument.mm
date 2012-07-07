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

#import "MapDocument.h"
#import "MapWindowController.h"
#import "EditorHolder.h"
#import "MacProgressIndicator.h"
#import "MacStringFactory.h"

#import "Controller/Camera.h"
#import "Controller/Editor.h"
#import "Controller/Grid.h"
#import "Controller/InputController.h"
#import "Model/Assets/Alias.h"
#import "Model/Assets/Bsp.h"
#import "Model/Map/Brush.h"
#import "Model/Map/Entity.h"
#import "Model/Map/EntityDefinition.h"
#import "Model/Map/Face.h"
#import "Model/Map/Map.h"
#import "Model/Selection.h"
#import "Model/Preferences.h"
#import "Model/Undo/UndoManager.h"
#import "IO/Pak.h"

#import <string>

using namespace TrenchBroom;
using namespace TrenchBroom::Controller;
using namespace TrenchBroom::Renderer;
using namespace TrenchBroom::Model;

namespace TrenchBroom {
    namespace Controller {
        UndoListener::UndoListener(Model::UndoManager& undoManager, MapDocument* mapDocument) : m_undoManager(undoManager), m_mapDocument(mapDocument) {
            m_undoManager.undoGroupCreated  += new Model::UndoManager::UndoEvent::Listener<UndoListener>(this, &UndoListener::undoGroupCreated);
            m_undoManager.undoPerformed     += new Model::UndoManager::UndoEvent::Listener<UndoListener>(this, &UndoListener::undoPerformed);
            m_undoManager.redoPerformed     += new Model::UndoManager::UndoEvent::Listener<UndoListener>(this, &UndoListener::redoPerformed);
        }
        
        UndoListener::~UndoListener() {
            m_undoManager.undoGroupCreated  -= new Model::UndoManager::UndoEvent::Listener<UndoListener>(this, &UndoListener::undoGroupCreated);
            m_undoManager.undoPerformed     -= new Model::UndoManager::UndoEvent::Listener<UndoListener>(this, &UndoListener::undoPerformed);
            m_undoManager.redoPerformed     -= new Model::UndoManager::UndoEvent::Listener<UndoListener>(this, &UndoListener::redoPerformed);
        }
        
        void UndoListener::undoGroupCreated(const Model::UndoGroup& group) {
            [m_mapDocument updateChangeCount:NSChangeDone];
            for (NSWindowController* controller in [m_mapDocument windowControllers])
                [controller setDocumentEdited:[m_mapDocument isDocumentEdited]];
        }
        
        void UndoListener::undoPerformed(const Model::UndoGroup& group) {
            [m_mapDocument updateChangeCount:NSChangeUndone];
            for (NSWindowController* controller in [m_mapDocument windowControllers])
                [controller setDocumentEdited:[m_mapDocument isDocumentEdited]];
        }

        void UndoListener::redoPerformed(const Model::UndoGroup& group) {
            [m_mapDocument updateChangeCount:NSChangeRedone];
            for (NSWindowController* controller in [m_mapDocument windowControllers])
                [controller setDocumentEdited:[m_mapDocument isDocumentEdited]];
        }
    }
}

@interface MapDocument (Private)
- (BOOL)gridOffModifierPressed;
@end

@implementation MapDocument (Private)

- (BOOL)gridOffModifierPressed {
    return ([NSEvent modifierFlags] & NSAlternateKeyMask) == NSAlternateKeyMask;
}

@end

@implementation MapDocument

- (id)init {
    self = [super init];
    if (self) {
        NSBundle* mainBundle = [NSBundle mainBundle];
        NSString* definitionPath = [mainBundle pathForResource:@"quake" ofType:@"def"];
        NSString* palettePath = [mainBundle pathForResource:@"QuakePalette" ofType:@"lmp"];
        
        editorHolder = [[EditorHolder alloc] initWithDefinitionPath:definitionPath palettePath:palettePath];
        Editor* editor = (Editor *)[editorHolder editor];
        undoListener = new UndoListener(editor->map().undoManager(), self);
    }
    
    return self;
}

- (void)dealloc {
    delete (UndoListener *)undoListener;
    [editorHolder release];
    [super dealloc];
}

- (void)makeWindowControllers {
	MapWindowController* controller = [[MapWindowController alloc] initWithWindowNibName:@"MapWindow"];
	[self addWindowController:controller];
    [controller release];
}

- (NSData *)dataOfType:(NSString *)typeName error:(NSError **)outError {
    return nil;
}

- (BOOL)readFromURL:(NSURL *)absoluteURL ofType:(NSString *)typeName error:(NSError **)outError {
    NSString* path = [absoluteURL path];
    const char* pathC = [path cStringUsingEncoding:NSASCIIStringEncoding];

    MacProgressIndicator* indicator = new MacProgressIndicator("Loading map file...");
    Editor* editor = (Editor *)[editorHolder editor];
    editor->loadMap(pathC, indicator);
    delete indicator;
    
    return YES;
}

- (BOOL)writeToURL:(NSURL *)absoluteURL ofType:(NSString *)typeName error:(NSError **)outError {
    NSString* path = [absoluteURL path];
    const char* pathC = [path cStringUsingEncoding:NSASCIIStringEncoding];

    Editor* editor = (Editor *)[editorHolder editor];
    editor->saveMap(pathC);
    
    return YES;
}

+ (BOOL)autosavesInPlace {
    return NO;
}

- (EditorHolder *)editorHolder {
    return editorHolder;
}

- (IBAction)customUndo:(id)sender {
    Editor* editor = (Editor *)[editorHolder editor];
    editor->undo();
}

- (IBAction)customRedo:(id)sender {
    Editor* editor = (Editor *)[editorHolder editor];
    editor->redo();
}

- (IBAction)delete:(id)sender {
    Editor* editor = (Editor *)[editorHolder editor];
    Map& map = editor->map();
    map.deleteObjects();
}

- (IBAction)selectAll:(id)sender {
    Editor* editor = (Editor *)[editorHolder editor];
    editor->selectAll();
}

- (IBAction)selectEntities:(id)sender {
    Editor* editor = (Editor *)[editorHolder editor];
    editor->selectEntities();
}

- (IBAction)selectTouching:(id)sender {
    Editor* editor = (Editor *)[editorHolder editor];
    editor->selectTouching();
}

- (IBAction)selectNone:(id)sender {
    Editor* editor = (Editor *)[editorHolder editor];
    editor->selectNone();
}

- (IBAction)toggleVertexTool:(id)sender {
    Editor* editor = (Editor *)[editorHolder editor];
    InputController& inputController = editor->inputController();
    inputController.toggleMoveVertexTool();
}

- (IBAction)toggleEdgeTool:(id)sender {
    Editor* editor = (Editor *)[editorHolder editor];
    InputController& inputController = editor->inputController();
    inputController.toggleMoveEdgeTool();
}

- (IBAction)toggleFaceTool:(id)sender {
    Editor* editor = (Editor *)[editorHolder editor];
    InputController& inputController = editor->inputController();
    inputController.toggleMoveFaceTool();
}

- (IBAction)moveTexturesLeft:(id)sender {
    Editor* editor = (Editor *)[editorHolder editor];
    editor->moveTextures(Editor::LEFT, [self gridOffModifierPressed]);
}

- (IBAction)moveTexturesUp:(id)sender {
    Editor* editor = (Editor *)[editorHolder editor];
    editor->moveTextures(Editor::UP, [self gridOffModifierPressed]);
}

- (IBAction)moveTexturesRight:(id)sender {
    Editor* editor = (Editor *)[editorHolder editor];
    editor->moveTextures(Editor::RIGHT, [self gridOffModifierPressed]);
}

- (IBAction)moveTexturesDown:(id)sender {
    Editor* editor = (Editor *)[editorHolder editor];
    editor->moveTextures(Editor::DOWN, [self gridOffModifierPressed]);
}

- (IBAction)rotateTexturesCW:(id)sender {
    Editor* editor = (Editor *)[editorHolder editor];
    editor->rotateTextures(true, [self gridOffModifierPressed]);
}

- (IBAction)rotateTexturesCCW:(id)sender {
    Editor* editor = (Editor *)[editorHolder editor];
    editor->rotateTextures(false, [self gridOffModifierPressed]);
}

- (IBAction)moveObjectsLeft:(id)sender {
    Editor* editor = (Editor *)[editorHolder editor];
    editor->moveObjects(Editor::LEFT, [self gridOffModifierPressed]);
}

- (IBAction)moveObjectsUp:(id)sender {
    Editor* editor = (Editor *)[editorHolder editor];
    editor->moveObjects(Editor::UP, [self gridOffModifierPressed]);
}

- (IBAction)moveObjectsRight:(id)sender {
    Editor* editor = (Editor *)[editorHolder editor];
    editor->moveObjects(Editor::RIGHT, [self gridOffModifierPressed]);
}

- (IBAction)moveObjectsDown:(id)sender {
    Editor* editor = (Editor *)[editorHolder editor];
    editor->moveObjects(Editor::DOWN, [self gridOffModifierPressed]);
}

- (IBAction)moveObjectsForward:(id)sender {
    Editor* editor = (Editor *)[editorHolder editor];
    editor->moveObjects(Editor::FORWARD, [self gridOffModifierPressed]);
}

- (IBAction)moveObjectsBackward:(id)sender {
    Editor* editor = (Editor *)[editorHolder editor];
    editor->moveObjects(Editor::BACKWARD, [self gridOffModifierPressed]);
}

- (IBAction)rollObjectsCW:(id)sender {
    Editor* editor = (Editor *)[editorHolder editor];
    editor->rotateObjects(Editor::ROLL, true);
}

- (IBAction)rollObjectsCCW:(id)sender {
    Editor* editor = (Editor *)[editorHolder editor];
    editor->rotateObjects(Editor::ROLL, false);
}

- (IBAction)pitchObjectsCW:(id)sender {
    Editor* editor = (Editor *)[editorHolder editor];
    editor->rotateObjects(Editor::PITCH, true);
}

- (IBAction)pitchObjectsCCW:(id)sender {
    Editor* editor = (Editor *)[editorHolder editor];
    editor->rotateObjects(Editor::PITCH, false);
}

- (IBAction)yawObjectsCW:(id)sender {
    Editor* editor = (Editor *)[editorHolder editor];
    editor->rotateObjects(Editor::YAW, true);
}

- (IBAction)yawObjectsCCW:(id)sender {
    Editor* editor = (Editor *)[editorHolder editor];
    editor->rotateObjects(Editor::YAW, false);
}

- (IBAction)flipObjectsHorizontally:(id)sender {
    Editor* editor = (Editor *)[editorHolder editor];
    editor->flipObjects(true);
}

- (IBAction)flipObjectsVertically:(id)sender {
    Editor* editor = (Editor *)[editorHolder editor];
    editor->flipObjects(false);
}

- (IBAction)duplicateObjects:(id)sender {
}

- (IBAction)enlargeBrushes:(id)sender {
}


- (IBAction)toggleGrid:(id)sender {
    Editor* editor = (Editor *)[editorHolder editor];
    editor->toggleGrid();
}

- (IBAction)toggleSnapToGrid:(id)sender {
    Editor* editor = (Editor *)[editorHolder editor];
    editor->toggleSnapToGrid();
}

- (IBAction)setGridSize:(id)sender {
    NSMenuItem* menuItem = (NSMenuItem *)sender;
    Editor* editor = (Editor *)[editorHolder editor];
    editor->setGridSize([menuItem tag]);
}

- (IBAction)moveCameraLeft:(id)sender {
    Editor* editor = (Editor *)[editorHolder editor];
    editor->moveCamera(Editor::LEFT, [self gridOffModifierPressed]);
}

- (IBAction)moveCameraUp:(id)sender {
    Editor* editor = (Editor *)[editorHolder editor];
    editor->moveCamera(Editor::UP, [self gridOffModifierPressed]);
}

- (IBAction)moveCameraRight:(id)sender {
    Editor* editor = (Editor *)[editorHolder editor];
    editor->moveCamera(Editor::RIGHT, [self gridOffModifierPressed]);
}

- (IBAction)moveCameraDown:(id)sender {
    Editor* editor = (Editor *)[editorHolder editor];
    editor->moveCamera(Editor::DOWN, [self gridOffModifierPressed]);
}

- (IBAction)moveCameraForward:(id)sender {
    Editor* editor = (Editor *)[editorHolder editor];
    editor->moveCamera(Editor::FORWARD, [self gridOffModifierPressed]);
}

- (IBAction)moveCameraBackward:(id)sender {
    Editor* editor = (Editor *)[editorHolder editor];
    editor->moveCamera(Editor::BACKWARD, [self gridOffModifierPressed]);
}

- (BOOL)validateMenuItem:(NSMenuItem *)menuItem {
    SEL action = [menuItem action];
    Editor* editor = (Editor *)[editorHolder editor];
    Map& map = editor->map();
    Selection& selection = map.selection();
    UndoManager& undoManager = map.undoManager();

    if (action == @selector(customUndo:)) {
        if (undoManager.undoStackEmpty()) {
            [menuItem setTitle:@"Undo"];
            return NO;
        } else {
            NSString* objcName = [NSString stringWithCString:undoManager.topUndoName().c_str() encoding:NSASCIIStringEncoding];
            [menuItem setTitle:[NSString stringWithFormat:@"Undo %@", objcName]];
        }
    } else if (action == @selector(customRedo:)) {
        if (undoManager.redoStackEmpty()) {
            [menuItem setTitle:@"Redo"];
            return NO;
        } else {
            NSString* objcName = [NSString stringWithCString:undoManager.topRedoName().c_str() encoding:NSASCIIStringEncoding];
            [menuItem setTitle:[NSString stringWithFormat:@"Redo %@", objcName]];
        }
    } else if (action == @selector(delete:)) {
        return selection.mode() == Model::TB_SM_BRUSHES || selection.mode() == Model::TB_SM_ENTITIES || selection.mode() == Model::TB_SM_BRUSHES_ENTITIES;
    } else if (action == @selector(selectEntity:)) {
        return selection.mode() == Model::TB_SM_BRUSHES;
    } else if (action == @selector(selectTouching:)) {
        return selection.mode() == Model::TB_SM_BRUSHES && selection.brushes().size() == 1;
    } else if (action == @selector(selectNone:)) {
        return !selection.empty();
    } else if (action == @selector(toggleVertexTool:)) {
        Editor* editor = (Editor *)[editorHolder editor];
        InputController& inputController = editor->inputController();
        return inputController.moveVertexToolActive() || selection.mode() == Model::TB_SM_BRUSHES;
    } else if (action == @selector(toggleEdgeTool:)) {
        Editor* editor = (Editor *)[editorHolder editor];
        InputController& inputController = editor->inputController();
        return inputController.moveEdgeToolActive() || selection.mode() == Model::TB_SM_BRUSHES;
    } else if (action == @selector(toggleFaceTool:)) {
        Editor* editor = (Editor *)[editorHolder editor];
        InputController& inputController = editor->inputController();
        return inputController.moveFaceToolActive() || selection.mode() == Model::TB_SM_BRUSHES;
    } else if (action == @selector(moveTexturesLeft:) || 
               action == @selector(moveTexturesUp:) || 
               action == @selector(moveTexturesRight:) ||
               action == @selector(moveTexturesDown:) ||
               action == @selector(rotateTexturesCW:) ||
               action == @selector(rotateTexturesCCW:)) {
        return selection.mode() == TB_SM_FACES;
    } else if (action == @selector(moveObjectsLeft:) ||
               action == @selector(moveObjectsUp:) ||
               action == @selector(moveObjectsRight:) ||
               action == @selector(moveObjectsDown:) ||
               action == @selector(moveObjectsAway:) ||
               action == @selector(moveObjectsToward:) ||
               action == @selector(rollObjectsCW:) ||
               action == @selector(rollObjectsCCW:) ||
               action == @selector(pitchObjectsCW:) ||
               action == @selector(pitchObjectsCCW:) ||
               action == @selector(yawObjectsCW:) ||
               action == @selector(yawObjectsCCW:) ||
               action == @selector(flipObjectsHorizontally:) ||
               action == @selector(flipObjectsVertically:) ||
               action == @selector(duplicateObjects:)) {
        return selection.mode() == Model::TB_SM_BRUSHES || selection.mode() == Model::TB_SM_ENTITIES || selection.mode() == Model::TB_SM_BRUSHES_ENTITIES;
    } else if (action == @selector(enlargeBrushes:)) {
        return selection.mode() == Model::TB_SM_BRUSHES;
    } else if (action == @selector(toggleGrid:)) {
        [menuItem setState:editor->grid().visible() ? NSOnState : NSOffState];
    } else if (action == @selector(toggleSnapToGrid:)) {
        [menuItem setState:editor->grid().snap() ? NSOnState : NSOffState];
    } else if (action == @selector(setGridSize:)) {
        [menuItem setState:editor->grid().size() == [menuItem tag] ? NSOnState : NSOffState];
    }
    
    return [super validateMenuItem:menuItem];
}

@end
