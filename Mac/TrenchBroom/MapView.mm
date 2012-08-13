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

#import "MapView.h"
#import "EditorHolder.h"
#import "EntityMenuUtils.h"
#import "MacStringFactory.h"
#import "MapDocument.h"

#import "Controller/Camera.h"
#import "Controller/Editor.h"
#import "GL/GLee.h"
#import "GUI/EditorGui.h"
#import "Gwen/InputHandler.h"
#import "Model/Map/Map.h"
#import "Model/Map/EntityDefinition.h"
#import "Model/Selection.h"
#import "Renderer/FontManager.h"
#import "Utilities/VecMath.h"

#import <math.h>

using namespace TrenchBroom;

namespace TrenchBroom {
    namespace Gui {
        EditorGuiListener::EditorGuiListener(EditorGui* editorGui, MapView* mapView) : m_editorGui(editorGui), m_mapView(mapView) {
            m_editorGui->editorGuiRedraw += new EditorGui::EditorGuiEvent::Listener<EditorGuiListener>(this, &EditorGuiListener::editorGuiRedraw);
        }
        
        EditorGuiListener::~EditorGuiListener() {
            m_editorGui->editorGuiRedraw -= new EditorGui::EditorGuiEvent::Listener<EditorGuiListener>(this, &EditorGuiListener::editorGuiRedraw);
        }

        void EditorGuiListener::editorGuiRedraw(EditorGui& editorGui) {
            [m_mapView setNeedsDisplay:YES];
        }
    }
}

@interface MapView (Private)
- (void)prepareOpenGL;
- (BOOL)key:(NSEvent*)theEvent down:(BOOL)down;
- (BOOL)key:(NSEvent*)theEvent mask:(NSUInteger)theMask gwenKey:(int)theGwenKey;
- (Controller::Editor*)editor;
- (Gui::EditorGui*)editorGui;
@end

@implementation MapView (Private)
- (void)prepareOpenGL {
    GLint swapInt = 1;
    [[self openGLContext] setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];
}

- (BOOL)key:(NSEvent*)theEvent down:(BOOL)down {
    if (editorGui == NULL) return NO;
    switch ([theEvent keyCode]) {
        case 36:
            return [self editorGui]->canvas()->InputKey(Gwen::Key::Return, down);
        case 51:
            return [self editorGui]->canvas()->InputKey(Gwen::Key::Backspace, down);
        case 117:
            return [self editorGui]->canvas()->InputKey(Gwen::Key::Delete, down);
        case 123:
            return [self editorGui]->canvas()->InputKey(Gwen::Key::Left, down);
        case 124:
            return [self editorGui]->canvas()->InputKey(Gwen::Key::Right, down);
        case 48:
            return [self editorGui]->canvas()->InputKey(Gwen::Key::Tab, down);
        case 49:
            return [self editorGui]->canvas()->InputKey(Gwen::Key::Space, down);
        case 115:
            return [self editorGui]->canvas()->InputKey(Gwen::Key::Home, down);
        case 119:
            return [self editorGui]->canvas()->InputKey(Gwen::Key::End, down);
        case 126:
            return [self editorGui]->canvas()->InputKey(Gwen::Key::Up, down);
        case 125:
            return [self editorGui]->canvas()->InputKey(Gwen::Key::Down, down);
        case 53:
            return [self editorGui]->canvas()->InputKey(Gwen::Key::Escape, down);
        default:
            // http://stackoverflow.com/questions/891594/nsstring-to-wchar-t
            if (down) {
                const char* utf16 = [[theEvent characters] cStringUsingEncoding:NSUTF16LittleEndianStringEncoding];
                wchar_t c = *utf16;
                return [self editorGui]->canvas()->InputCharacter(c);
            }
            return true;
    }
}

- (BOOL)key:(NSEvent*)theEvent mask:(NSUInteger)theMask gwenKey:(int)theGwenKey; {
    if ((flags & theMask) != ([theEvent modifierFlags] & theMask)) {
        BOOL down = ([theEvent modifierFlags] & theMask) == theMask;
        return [self editorGui]->canvas()->InputKey(theGwenKey, down);
    }
    return NO;
}

- (Controller::Editor*)editor {
    if (editorHolder == NULL) {
        NSWindow* window = [self window];
        NSWindowController* controller = [window windowController];
        MapDocument* document = [controller document];
        editorHolder = [[document editorHolder] retain];
    }
    return (Controller::Editor*)[editorHolder editor];
}

- (Gui::EditorGui*)editorGui {
    return (Gui::EditorGui*)editorGui;
}

@end

@implementation MapView

- (void)dealloc {
    if (editorGuiListener != NULL)
        delete ((Gui::EditorGuiListener*)editorGuiListener);
    if (editorGui != NULL)
        delete ((Gui::EditorGui*)editorGui);
    if (fontManager != NULL)
        delete ((Renderer::FontManager*)fontManager);
    if (editorHolder != NULL)
        [editorHolder release];
    if (popupMenu != NULL)
        [popupMenu release];
    [super dealloc];
}

- (BOOL)mapViewFocused {
    return [self editorGui]->mapViewFocused();
}

- (IBAction)createEntityFromPopupMenu:(id)sender {
    NSMenuItem* item = (NSMenuItem*)sender;
    std::string definitionName = [[item title] cStringUsingEncoding:NSASCIIStringEncoding];
    [self editor]->createEntityAtClickPos(definitionName);
}

- (IBAction)createEntityFromMainMenu:(id)sender {
    NSMenuItem* item = (NSMenuItem*)sender;
    std::string definitionName = [[item title] cStringUsingEncoding:NSASCIIStringEncoding];
    [self editor]->createEntityAtDefaultPos(definitionName);
}

- (BOOL)validateMenuItem:(NSMenuItem *)menuItem {
    Controller::Editor* editor = [self editor];
    Model::Map& map = editor->map();
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
    
    return false;
}

- (BOOL)acceptsFirstResponder {
    return YES;
}

- (void)awakeFromNib {
    [[self window] setAcceptsMouseMovedEvents:YES];
    flags = [NSEvent modifierFlags];
}

- (void)drawRect:(NSRect)dirtyRect {
    if (fontManager == NULL)
        fontManager = new Renderer::FontManager(new Renderer::MacStringFactory());
    
    if (editorGui == NULL) {
        NSString* skinPath = [[NSBundle mainBundle] pathForResource:@"DefaultSkin" ofType:@"png"];
        std::string skinPathCpp([skinPath cStringUsingEncoding:NSASCIIStringEncoding]);
        editorGui = new Gui::EditorGui(*[self editor], *(Renderer::FontManager*)fontManager, skinPathCpp);
        editorGuiListener = new Gui::EditorGuiListener((Gui::EditorGui *)editorGui, self);
    }

    NSRect viewport = [self visibleRect];

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(NSMinX(viewport), NSMaxX(viewport), NSMaxY(viewport), NSMinY(viewport), -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glViewport(NSMinX(viewport), NSMinY(viewport), NSWidth(viewport), NSHeight(viewport));
    
    [self editorGui]->resizeTo(NSWidth(viewport), NSHeight(viewport));
    [self editorGui]->render();
    
    [[self openGLContext] flushBuffer];
}

- (void)keyDown:(NSEvent *)theEvent {
    if (![self key:theEvent down:YES])
        [super keyDown:theEvent];
}

- (void)keyUp:(NSEvent *)theEvent {
    if (![self key:theEvent down:NO])
        [super keyUp:theEvent];
}

- (void)flagsChanged:(NSEvent *)theEvent {
    if (editorGui != NULL) {
        [self key:theEvent mask:NSShiftKeyMask gwenKey:Gwen::Key::Shift];
        [self key:theEvent mask:NSAlternateKeyMask gwenKey:Gwen::Key::Alt];
        [self key:theEvent mask:NSControlKeyMask gwenKey:Gwen::Key::Control];
        [self key:theEvent mask:NSCommandKeyMask gwenKey:Gwen::Key::Command];
    }
    flags = [theEvent modifierFlags];
}

- (void)scrollWheel:(NSEvent *)theEvent {
    if (editorGui != NULL) {
        float delta = [theEvent deltaX];
        if (delta == 0) delta = [theEvent deltaY];
        if (delta == 0) delta = [theEvent deltaZ];
        [self editorGui]->canvas()->InputMouseWheel(delta);
    }
}

- (void)mouseDown:(NSEvent *)theEvent {
    if (editorGui != NULL) [self editorGui]->canvas()->InputMouseButton(0, true);
}

- (void)mouseUp:(NSEvent *)theEvent {
    if (editorGui != NULL) [self editorGui]->canvas()->InputMouseButton(0, false);
}

- (void)mouseDragged:(NSEvent *)theEvent {
    [self mouseMoved:theEvent];
}

- (void)rightMouseDown:(NSEvent *)theEvent {
    if (editorGui != NULL) [self editorGui]->canvas()->InputMouseButton(1, true);
}

- (void)rightMouseUp:(NSEvent *)theEvent {
    if (editorGui != NULL) {
        if (![self editorGui]->canvas()->InputMouseButton(1, false)) {
            if (popupMenu == nil) {
                Controller::Editor* editor = [self editor];
                Model::Map& map = editor->map();
                Model::EntityDefinitionManager& entityDefinitionManager = map.entityDefinitionManager();
                Model::EntityDefinitionList pointEntities = entityDefinitionManager.definitions(Model::TB_EDT_POINT);
                Model::EntityDefinitionList brushEntities = entityDefinitionManager.definitions(Model::TB_EDT_BRUSH);
                
                popupMenu = [[NSMenu alloc] initWithTitle:@"Map View Popup Menu"];
                NSMenuItem* pointEntityMenuItem = [popupMenu addItemWithTitle:@"Create Point Entity" action:NULL keyEquivalent:@""];
                NSMenu* pointEntityMenu = [[NSMenu alloc] initWithTitle:@"Create Point Entity Submenu"];
                [pointEntityMenuItem setSubmenu:[pointEntityMenu autorelease]];

                NSMenuItem* brushEntityMenuItem = [popupMenu addItemWithTitle:@"Create Brush Entity" action:NULL keyEquivalent:@""];
                NSMenu* brushEntityMenu = [[NSMenu alloc] initWithTitle:@"Create Brush Entity Submenu"];
                [brushEntityMenuItem setSubmenu:[brushEntityMenu autorelease]];
                
                Gui::createEntityMenu(pointEntityMenu, pointEntities, @selector(createEntityFromPopupMenu:));
                Gui::createEntityMenu(brushEntityMenu, brushEntities, @selector(createEntityFromPopupMenu:));
            }
            
            [NSMenu popUpContextMenu:popupMenu withEvent:theEvent forView:self];
        }
    }
}

- (void)rightMouseDragged:(NSEvent *)theEvent {
    [self mouseMoved:theEvent];
}

- (void)otherMouseDown:(NSEvent *)theEvent {
    if (editorGui != NULL) [self editorGui]->canvas()->InputMouseButton(2, true);
}

- (void)otherMouseUp:(NSEvent *)theEvent {
    if (editorGui != NULL) [self editorGui]->canvas()->InputMouseButton(2, false);
}

- (void)otherMouseDragged:(NSEvent *)theEvent {
    [self mouseMoved:theEvent];
}

- (void)mouseMoved:(NSEvent *)theEvent {
    NSPoint pos = [self convertPoint:[theEvent locationInWindow] fromView:nil];
    pos.y = [self visibleRect].size.height - pos.y;
    if (editorGui != NULL) [self editorGui]->canvas()->InputMouseMoved(pos.x, pos.y, theEvent.deltaX, -theEvent.deltaY);
}

@end
