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

#import "Controller/Autosaver.h"
#import "Controller/Camera.h"
#import "Controller/Editor.h"
#import "Controller/Grid.h"
#import "Controller/InputController.h"
#import "Controller/Options.h"
#import "Model/Assets/Alias.h"
#import "Model/Assets/Bsp.h"
#import "Model/Map/Brush.h"
#import "Model/Map/Entity.h"
#import "Model/Map/EntityDefinition.h"
#import "Model/Map/Face.h"
#import "Model/Map/Map.h"
#import "Model/Map/Picker.h"
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
- (BOOL)mapViewFocused;
- (BOOL)gridOffModifierPressed;
- (void)autosave;
@end

@implementation MapDocument (Private)

- (BOOL)mapViewFocused {
    for (NSWindowController* controller in [self windowControllers]) {
        if ([controller isKindOfClass:[MapWindowController class]]) {
            MapWindowController* mapController = (MapWindowController*)controller;
            if ([mapController mapViewFocused])
                return true;
        }
    }
    
    return false;
}

- (BOOL)gridOffModifierPressed {
    return ([NSEvent modifierFlags] & NSAlternateKeyMask) == NSAlternateKeyMask;
}

- (void)autosave {
    [autosaveTimer release];
    
    Editor* editor = (Editor *)[editorHolder editor];
    editor->autosaver().triggerAutosave();
    autosaveTimer = [[NSTimer scheduledTimerWithTimeInterval:3 target:self selector:@selector(autosave) userInfo:nil repeats:NO] retain];
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
        
        autosaveTimer = [[NSTimer scheduledTimerWithTimeInterval:3 target:self selector:@selector(autosave) userInfo:nil repeats:NO] retain];
    }
    
    return self;
}

- (void)dealloc {
    [autosaveTimer release];
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
    
    [self updateChangeCount:NSChangeCleared];
    for (NSWindowController* controller in [self windowControllers])
        [controller setDocumentEdited:[self isDocumentEdited]];

    return YES;
}

- (BOOL)writeToURL:(NSURL *)absoluteURL ofType:(NSString *)typeName error:(NSError **)outError {
    NSString* path = [absoluteURL path];
    const char* pathC = [path cStringUsingEncoding:NSASCIIStringEncoding];

    Editor* editor = (Editor *)[editorHolder editor];
    editor->saveMap(pathC);

    [self updateChangeCount:NSChangeCleared];
    for (NSWindowController* controller in [self windowControllers])
        [controller setDocumentEdited:[self isDocumentEdited]];

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

- (IBAction)selectSiblings:(id)sender {
    Editor* editor = (Editor *)[editorHolder editor];
    editor->selectSiblings();
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

- (IBAction)toggleTextureLock:(id)sender {
    Editor* editor = (Editor *)[editorHolder editor];
    editor->toggleTextureLock();
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

- (IBAction)createEntityFromPopupMenu:(id)sender {
    NSMenuItem* item = (NSMenuItem*)sender;
    std::string definitionName = [[item title] cStringUsingEncoding:NSASCIIStringEncoding];
    Editor* editor = (Editor *)[editorHolder editor];
    editor->createEntityAtClickPos(definitionName);
}

- (IBAction)createEntityFromMainMenu:(id)sender {
    NSMenuItem* item = (NSMenuItem*)sender;
    std::string definitionName = [[item title] cStringUsingEncoding:NSASCIIStringEncoding];
    Editor* editor = (Editor *)[editorHolder editor];
    editor->createEntityAtDefaultPos(definitionName);
}

- (IBAction)moveBrushesToEntity:(id)sender {
    Editor* editor = (Editor *)[editorHolder editor];
    editor->moveBrushesToEntity();
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
    editor->setGridSize(static_cast<int>([menuItem tag]));
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

- (IBAction)toggleIsolateSelection:(id)sender {
    Editor* editor = (Editor *)[editorHolder editor];
    editor->toggleIsolateSelection();
}

- (BOOL)validateMenuItem:(NSMenuItem *)menuItem {
    SEL action = [menuItem action];
    Editor* editor = (Editor *)[editorHolder editor];
    InputController& inputController = editor->inputController();
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
        return [self mapViewFocused] && (selection.selectionMode() == Model::TB_SM_BRUSHES || selection.selectionMode() == Model::TB_SM_ENTITIES || selection.selectionMode() == Model::TB_SM_BRUSHES_ENTITIES);
    } else if (action == @selector(selectAll:)) {
        return [self mapViewFocused];
    } else if (action == @selector(selectSiblings:)) {
        return [self mapViewFocused] && selection.selectionMode() == Model::TB_SM_BRUSHES;
    } else if (action == @selector(selectTouching:)) {
        return [self mapViewFocused] && selection.selectionMode() == Model::TB_SM_BRUSHES && selection.selectedBrushes().size() == 1;
    } else if (action == @selector(selectNone:)) {
        return [self mapViewFocused] && !selection.empty();
    } else if (action == @selector(toggleVertexTool:)) {
        [menuItem setState:inputController.moveVertexToolActive() ? NSOnState : NSOffState];
        if (![self mapViewFocused])
            return false;
        return inputController.moveVertexToolActive() || selection.selectionMode() == Model::TB_SM_BRUSHES || selection.selectionMode() == Model::TB_SM_BRUSHES_ENTITIES;
    } else if (action == @selector(toggleEdgeTool:)) {
        [menuItem setState:inputController.moveEdgeToolActive() ? NSOnState : NSOffState];
        if (![self mapViewFocused])
            return false;
        return inputController.moveEdgeToolActive() || selection.selectionMode() == Model::TB_SM_BRUSHES || selection.selectionMode() == Model::TB_SM_BRUSHES_ENTITIES;
    } else if (action == @selector(toggleFaceTool:)) {
        [menuItem setState:inputController.moveFaceToolActive() ? NSOnState : NSOffState];
        if (![self mapViewFocused])
            return false;
        return inputController.moveFaceToolActive() || selection.selectionMode() == Model::TB_SM_BRUSHES || selection.selectionMode() == Model::TB_SM_BRUSHES_ENTITIES;
    } else if (action == @selector(toggleTextureLock:)) {
        [menuItem setState:editor->options().lockTextures() ? NSOnState : NSOffState];
        return true;
    } else if (action == @selector(moveTexturesLeft:) ||
               action == @selector(moveTexturesUp:) || 
               action == @selector(moveTexturesRight:) ||
               action == @selector(moveTexturesDown:) ||
               action == @selector(rotateTexturesCW:) ||
               action == @selector(rotateTexturesCCW:)) {
        return [self mapViewFocused] && selection.selectionMode() == TB_SM_FACES;
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
        return [self mapViewFocused] && (selection.selectionMode() == Model::TB_SM_BRUSHES || selection.selectionMode() == Model::TB_SM_ENTITIES || selection.selectionMode() == Model::TB_SM_BRUSHES_ENTITIES);
    } else if (action == @selector(enlargeBrushes:)) {
        return [self mapViewFocused] && selection.selectionMode() == Model::TB_SM_BRUSHES;
    } else if (action == @selector(createEntityFromMainMenu:) ||
               action == @selector(createEntityFromPopupMenu:)) {
        Model::EntityDefinitionManager& entityDefinitionManager = map.entityDefinitionManager();
        std::string definitionName = [[menuItem title] cStringUsingEncoding:NSASCIIStringEncoding];
        Model::EntityDefinitionPtr definition = entityDefinitionManager.definition(definitionName);
        if (definition.get() == NULL)
            return NO;
        
        if (definition->type == Model::TB_EDT_POINT)
            return true;
        
        if (definition->type == Model::TB_EDT_BRUSH) {
            Model::Selection& selection = map.selection();
            return !selection.selectedBrushes().empty();
        }
    } else if (action == @selector(moveBrushesToEntity:)) {
        const Model::BrushList& selectedBrushes = selection.selectedBrushes();
        if (!selectedBrushes.empty()) {
            Model::Hit* hit = inputController.event().hits->first(Model::TB_HT_FACE | Model::TB_HT_ENTITY, false);
            Model::Entity* target = NULL;
            if (hit == NULL)
                target = map.worldspawn(true);
            else if (hit->type == Model::TB_HT_FACE)
                target = hit->face().brush()->entity();
            else
                target = &hit->entity();
            
            for (unsigned int i = 0; i < selectedBrushes.size(); i++) {
                if (selectedBrushes[i]->entity() != target) {
                    std::string classname = target->classname() != NULL ? *target->classname() : "Entity";
                    std::string title = "Move Brushes to " + classname;
                    [menuItem setTitle:[NSString stringWithCString:title.c_str() encoding:NSASCIIStringEncoding]];
                    return true;
                }
            }
        }
        [menuItem setTitle:@"Move Brushes to Entity"];
        return false;
    } else if (action == @selector(toggleGrid:)) {
        [menuItem setState:editor->grid().visible() ? NSOnState : NSOffState];
        return [self mapViewFocused];
    } else if (action == @selector(toggleSnapToGrid:)) {
        [menuItem setState:editor->grid().snap() ? NSOnState : NSOffState];
        return [self mapViewFocused];
    } else if (action == @selector(setGridSize:)) {
        [menuItem setState:editor->grid().size() == [menuItem tag] ? NSOnState : NSOffState];
        return [self mapViewFocused];
    } else if (action == @selector(toggleIsolateSelection:)) {
        return !selection.empty();
    }
    
    return [super validateMenuItem:menuItem];
}

@end
