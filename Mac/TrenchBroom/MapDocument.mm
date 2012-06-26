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

#import "Controller/Editor.h"
#import "Controller/Grid.h"
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

- (BOOL)validateMenuItem:(NSMenuItem *)menuItem {
    SEL action = [menuItem action];
    if (action == @selector(customUndo:)) {
        Editor* editor = (Editor *)[editorHolder editor];
        Map& map = editor->map();
        UndoManager& undoManager = map.undoManager();
        
        if (undoManager.undoStackEmpty()) {
            [menuItem setTitle:@"Undo"];
            return NO;
        } else {
            NSString* objcName = [NSString stringWithCString:undoManager.topUndoName().c_str() encoding:NSASCIIStringEncoding];
            [menuItem setTitle:[NSString stringWithFormat:@"Undo %@", objcName]];
        }
    } else if (action == @selector(customRedo:)) {
        Editor* editor = (Editor *)[editorHolder editor];
        Map& map = editor->map();
        UndoManager& undoManager = map.undoManager();
        
        if (undoManager.redoStackEmpty()) {
            [menuItem setTitle:@"Redo"];
            return NO;
        } else {
            NSString* objcName = [NSString stringWithCString:undoManager.topRedoName().c_str() encoding:NSASCIIStringEncoding];
            [menuItem setTitle:[NSString stringWithFormat:@"Redo %@", objcName]];
        }
    } else if (action == @selector(toggleGrid:)) {
        Editor* editor = (Editor *)[editorHolder editor];
        [menuItem setState:editor->grid().visible() ? NSOnState : NSOffState];
    } else if (action == @selector(toggleSnapToGrid:)) {
        Editor* editor = (Editor *)[editorHolder editor];
        [menuItem setState:editor->grid().snap() ? NSOnState : NSOffState];
    } else if (action == @selector(setGridSize:)) {
        Editor* editor = (Editor *)[editorHolder editor];
        [menuItem setState:editor->grid().size() == [menuItem tag] ? NSOnState : NSOffState];
    }
    
    return [super validateMenuItem:menuItem];
}

@end
