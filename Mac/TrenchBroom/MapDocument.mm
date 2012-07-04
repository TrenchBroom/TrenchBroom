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
- (void)translateTextures:(const Vec3f&)dir;
- (void)translateObjects:(const Vec3f&)dir;
@end

@implementation MapDocument (Private)

- (BOOL)gridOffModifierPressed {
    return ([NSEvent modifierFlags] & NSAlternateKeyMask) == NSAlternateKeyMask;
}

- (void)translateTextures:(const Vec3f&)dir {
    Editor* editor = (Editor *)[editorHolder editor];
    Map& map = editor->map();
    Grid& grid = editor->grid();
    
    float delta = [self gridOffModifierPressed] ? 1.0f : static_cast<float>(grid.actualSize());
    map.translateFaces(delta, dir);
}

- (void)translateObjects:(const Vec3f&)dir {
    Editor* editor = (Editor *)[editorHolder editor];
    Map& map = editor->map();
    Grid& grid = editor->grid();
    Selection& selection = map.selection();
    
    float dist = [self gridOffModifierPressed] ? 1.0f : static_cast<float>(grid.actualSize());
    Vec3f delta = grid.moveDelta(selection.bounds(), map.worldBounds(), dir.firstAxis() * dist);
    
    map.translateObjects(delta, true);
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
    Map& map = editor->map();
    UndoManager& undoManager = map.undoManager();
    undoManager.undo();
}

- (IBAction)customRedo:(id)sender {
    Editor* editor = (Editor *)[editorHolder editor];
    Map& map = editor->map();
    UndoManager& undoManager = map.undoManager();
    undoManager.redo();
}

- (IBAction)delete:(id)sender {
    Editor* editor = (Editor *)[editorHolder editor];
    Map& map = editor->map();
    map.deleteObjects();
}

- (IBAction)selectAll:(id)sender {
    Editor* editor = (Editor *)[editorHolder editor];
    Map& map = editor->map();
    
    Selection& selection = map.selection();
    selection.removeAll();
    
    const EntityList& entities = map.entities();
    BrushList brushes;
    for (unsigned int i = 0; i < entities.size(); i++)
        brushes.insert(brushes.begin(), entities[i]->brushes().begin(), entities[i]->brushes().end());
    if (!brushes.empty())
        selection.addBrushes(brushes);
    if (!entities.empty())
        selection.addEntities(entities);
}

- (IBAction)selectEntity:(id)sender {
    Editor* editor = (Editor *)[editorHolder editor];
    Map& map = editor->map();
    map.selectEntities();
}

- (IBAction)selectTouching:(id)sender {
    Editor* editor = (Editor *)[editorHolder editor];
    Map& map = editor->map();
    map.selectTouching(true);
}

- (IBAction)selectNone:(id)sender {
    Editor* editor = (Editor *)[editorHolder editor];
    Map& map = editor->map();
    Selection& selection = map.selection();
    selection.removeAll();
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
    [self translateTextures:editor->camera().right() * -1.0f];
}

- (IBAction)moveTexturesUp:(id)sender {
    Editor* editor = (Editor *)[editorHolder editor];
    [self translateTextures:editor->camera().up()];
}

- (IBAction)moveTexturesRight:(id)sender {
    Editor* editor = (Editor *)[editorHolder editor];
    [self translateTextures:editor->camera().right()];
}

- (IBAction)moveTexturesDown:(id)sender {
    Editor* editor = (Editor *)[editorHolder editor];
    [self translateTextures:editor->camera().up() * -1.0f];
}

- (IBAction)rotateTexturesCW:(id)sender {
    Editor* editor = (Editor *)[editorHolder editor];
    Map& map = editor->map();
    Grid& grid = editor->grid();

    float angle = [self gridOffModifierPressed] ? 1.0f : grid.angle();
    map.rotateFaces(-angle);
}

- (IBAction)rotateTexturesCCW:(id)sender {
    Editor* editor = (Editor *)[editorHolder editor];
    Map& map = editor->map();
    Grid& grid = editor->grid();
    
    float angle = [self gridOffModifierPressed] ? 1.0f : grid.angle();
    map.rotateFaces(angle);
}

- (IBAction)moveObjectsLeft:(id)sender {
    Editor* editor = (Editor *)[editorHolder editor];
    [self translateObjects:editor->camera().right() * -1.0f];
}

- (IBAction)moveObjectsUp:(id)sender {
    [self translateObjects:Vec3f::PosZ];
}

- (IBAction)moveObjectsRight:(id)sender {
    Editor* editor = (Editor *)[editorHolder editor];
    [self translateObjects:editor->camera().right()];
}

- (IBAction)moveObjectsDown:(id)sender {
    [self translateObjects:Vec3f::NegZ];
}

- (IBAction)moveObjectsToward:(id)sender {
    Editor* editor = (Editor *)[editorHolder editor];
    Vec3f dir = (editor->camera().direction() * -1.0f).firstAxis();
    if (dir.firstComponent() == TB_AX_Z)
        dir = (editor->camera().direction() * -1.0f).secondAxis();
    [self translateObjects:dir];
}

- (IBAction)moveObjectsAway:(id)sender {
    Editor* editor = (Editor *)[editorHolder editor];
    Vec3f dir = editor->camera().direction().firstAxis();
    if (dir.firstComponent() == TB_AX_Z)
        dir = editor->camera().direction().secondAxis();
    [self translateObjects:dir];
}

- (IBAction)rollObjectsCW:(id)sender {
    Editor* editor = (Editor *)[editorHolder editor];
    Map& map = editor->map();
    Selection& selection = map.selection();
    
    EAxis axis = editor->camera().direction().firstComponent();
    map.rotateObjects90(axis, selection.center(), true, true);
}

- (IBAction)rollObjectsCCW:(id)sender {
    Editor* editor = (Editor *)[editorHolder editor];
    Map& map = editor->map();
    Selection& selection = map.selection();
    
    EAxis axis = editor->camera().direction().firstComponent();
    map.rotateObjects90(axis, selection.center(), false, true);
}

- (IBAction)pitchObjectsCW:(id)sender {
    Editor* editor = (Editor *)[editorHolder editor];
    Map& map = editor->map();
    Selection& selection = map.selection();
    
    EAxis axis = editor->camera().right().firstComponent();
    map.rotateObjects90(axis, selection.center(), true, true);
}

- (IBAction)pitchObjectsCCW:(id)sender {
    Editor* editor = (Editor *)[editorHolder editor];
    Map& map = editor->map();
    Selection& selection = map.selection();
    
    EAxis axis = editor->camera().right().firstComponent();
    map.rotateObjects90(axis, selection.center(), false, true);
}

- (IBAction)yawObjectsCW:(id)sender {
    Editor* editor = (Editor *)[editorHolder editor];
    Map& map = editor->map();
    Selection& selection = map.selection();
    map.rotateObjects90(TB_AX_Z, selection.center(), true, true);
}

- (IBAction)yawObjectsCCW:(id)sender {
    Editor* editor = (Editor *)[editorHolder editor];
    Map& map = editor->map();
    Selection& selection = map.selection();
    map.rotateObjects90(TB_AX_Z, selection.center(), false, true);
}

- (IBAction)flipObjectsHorizontally:(id)sender {
    Editor* editor = (Editor *)[editorHolder editor];
    Map& map = editor->map();
    Selection& selection = map.selection();
    
    EAxis axis = editor->camera().right().firstComponent();
    map.flipObjects(axis, selection.center(), true);
}

- (IBAction)flipObjectsVertically:(id)sender {
    Editor* editor = (Editor *)[editorHolder editor];
    Map& map = editor->map();
    Selection& selection = map.selection();
    
    map.flipObjects(TB_AX_Z, selection.center(), true);
}

- (IBAction)duplicateObjects:(id)sender {
}

- (IBAction)enlargeBrushes:(id)sender {
}


- (IBAction)toggleGrid:(id)sender {
    Editor* editor = (Editor *)[editorHolder editor];
    editor->grid().toggleVisible();
}

- (IBAction)toggleSnapToGrid:(id)sender {
    Editor* editor = (Editor *)[editorHolder editor];
    editor->grid().toggleSnap();
}

- (IBAction)setGridSize:(id)sender {
    NSMenuItem* menuItem = (NSMenuItem *)sender;
    Editor* editor = (Editor *)[editorHolder editor];
    editor->grid().setSize([menuItem tag]);
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
