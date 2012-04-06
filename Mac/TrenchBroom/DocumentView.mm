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

#import "DocumentView.h"
#import "EditorGui.h"
#import "VecMath.h"
#import <OpenGL/gl.h>
#import <OpenGL/glu.h>
#import <math.h>
#import "Gwen/InputHandler.h"

using namespace TrenchBroom;
using namespace TrenchBroom::Gui;

@interface DocumentView (Private)
- (void)renderTimerFired:(NSNotification*)theNotification;
- (void)key:(NSEvent*)theEvent down:(BOOL)down;
- (void)key:(NSEvent*)theEvent mask:(NSUInteger)theMask gwenKey:(int)theGwenKey;
@end

@implementation DocumentView (Private)
- (void)renderTimerFired:(NSNotification*)theNotification {
    [self setNeedsDisplay:YES];
}

- (void)key:(NSEvent*)theEvent down:(BOOL)down {
    if (editorGui == NULL) return;
    switch ([theEvent keyCode]) {
        case 36:
            ((EditorGui *)editorGui)->canvas()->InputKey(Gwen::Key::Return, down);
            break;
        case 51:
            ((EditorGui *)editorGui)->canvas()->InputKey(Gwen::Key::Backspace, down);
            break;
        case 117:
            ((EditorGui *)editorGui)->canvas()->InputKey(Gwen::Key::Delete, down);
            break;
        case 123:
            ((EditorGui *)editorGui)->canvas()->InputKey(Gwen::Key::Left, down);
            break;
        case 124:
            ((EditorGui *)editorGui)->canvas()->InputKey(Gwen::Key::Right, down);
            break;
        case 48:
            ((EditorGui *)editorGui)->canvas()->InputKey(Gwen::Key::Tab, down);
            break;
        case 49:
            ((EditorGui *)editorGui)->canvas()->InputKey(Gwen::Key::Space, down);
            break;
        case 115:
            ((EditorGui *)editorGui)->canvas()->InputKey(Gwen::Key::Home, down);
            break;
        case 119:
            ((EditorGui *)editorGui)->canvas()->InputKey(Gwen::Key::End, down);
            break;
        case 126:
            ((EditorGui *)editorGui)->canvas()->InputKey(Gwen::Key::Up, down);
            break;
        case 125:
            ((EditorGui *)editorGui)->canvas()->InputKey(Gwen::Key::Down, down);
            break;
        case 53:
            ((EditorGui *)editorGui)->canvas()->InputKey(Gwen::Key::Escape, down);
            break;
        default:
            // http://stackoverflow.com/questions/891594/nsstring-to-wchar-t
            if (down) {
                const char* utf16 = [[theEvent characters] cStringUsingEncoding:NSUTF16LittleEndianStringEncoding];
                wchar_t c = *utf16;
                ((EditorGui *)editorGui)->canvas()->InputCharacter(c);
            }
            break;
    }
}

- (void)key:(NSEvent*)theEvent mask:(NSUInteger)theMask gwenKey:(int)theGwenKey; {
    if ((flags & theMask) != ([theEvent modifierFlags] & theMask)) {
        BOOL down = ([theEvent modifierFlags] & theMask) == theMask;
        ((EditorGui *)editorGui)->canvas()->InputKey(theGwenKey, down);
    }
}
@end

@implementation DocumentView

- (void)prepareOpenGL {
    GLint swapInt = 1;
    [[self openGLContext] setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];
}

- (BOOL)acceptsFirstResponder {
    return YES;
}

- (void)awakeFromNib {
    // set up a render loop
    NSTimer* renderTimer = [NSTimer timerWithTimeInterval:0.001 target:self selector:@selector(renderTimerFired:) userInfo:nil repeats:YES];
    [[NSRunLoop currentRunLoop] addTimer:renderTimer forMode:NSDefaultRunLoopMode];
    [[NSRunLoop currentRunLoop] addTimer:renderTimer forMode:NSEventTrackingRunLoopMode]; //Ensure timer fires during resize
    
    [[self window] setAcceptsMouseMovedEvents:YES];
    flags = [NSEvent modifierFlags];
}

- (void)drawRect:(NSRect)dirtyRect {
    if (editorGui == NULL) {
        NSString* skinPath = [[NSBundle mainBundle] pathForResource:@"DefaultSkin" ofType:@"png"];
        string skinPathCpp([skinPath cStringUsingEncoding:NSASCIIStringEncoding]);
        editorGui = new EditorGui(skinPathCpp);
    }

    NSRect viewport = [self visibleRect];
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(NSMinX(viewport), NSMaxX(viewport), NSMaxY(viewport), NSMinY(viewport), -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glViewport(NSMinX(viewport), NSMinY(viewport), NSWidth(viewport), NSHeight(viewport));
    
    ((EditorGui *)editorGui)->resizeTo(NSWidth(viewport), NSHeight(viewport));
    ((EditorGui *)editorGui)->render();
    
    [[self openGLContext] flushBuffer];
}

- (void)keyDown:(NSEvent *)theEvent {
    [self key:theEvent down:YES];
}

- (void)keyUp:(NSEvent *)theEvent {
    [self key:theEvent down:NO];
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
        ((EditorGui *)editorGui)->canvas()->InputMouseWheel(delta);
    }
}

- (void)mouseDown:(NSEvent *)theEvent {
    if (editorGui != NULL) ((EditorGui *)editorGui)->canvas()->InputMouseButton(0, true);
}

- (void)mouseUp:(NSEvent *)theEvent {
    if (editorGui != NULL) ((EditorGui *)editorGui)->canvas()->InputMouseButton(0, false);
}

- (void)mouseDragged:(NSEvent *)theEvent {
    [self mouseMoved:theEvent];
}

- (void)rightMouseDown:(NSEvent *)theEvent {
    if (editorGui != NULL) ((EditorGui *)editorGui)->canvas()->InputMouseButton(1, true);
}

- (void)rightMouseUp:(NSEvent *)theEvent {
    if (editorGui != NULL) ((EditorGui *)editorGui)->canvas()->InputMouseButton(1, false);
}

- (void)rightMouseDragged:(NSEvent *)theEvent {
    [self mouseMoved:theEvent];
}

- (void)otherMouseDown:(NSEvent *)theEvent {
    if (editorGui != NULL) ((EditorGui *)editorGui)->canvas()->InputMouseButton(2, true);
}

- (void)otherMouseUp:(NSEvent *)theEvent {
    if (editorGui != NULL) ((EditorGui *)editorGui)->canvas()->InputMouseButton(2, false);
}

- (void)otherMouseDragged:(NSEvent *)theEvent {
    [self mouseMoved:theEvent];
}

- (void)mouseMoved:(NSEvent *)theEvent {
    NSPoint pos = [self convertPoint:[theEvent locationInWindow] fromView:nil];
    if (editorGui != NULL) ((EditorGui *)editorGui)->canvas()->InputMouseMoved(pos.x, pos.y, theEvent.deltaX, theEvent.deltaY);
}

@end
