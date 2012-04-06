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

using namespace TrenchBroom;
using namespace TrenchBroom::Gui;

@interface DocumentView (Private)
- (void)renderTimerFired:(NSNotification*)theNotification;
@end

@implementation DocumentView (Private)
- (void)renderTimerFired:(NSNotification*)theNotification {
    [self setNeedsDisplay:YES];
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

- (void)mouseDown:(NSEvent *)theEvent {
    if (editorGui != NULL) ((EditorGui *)editorGui)->canvas()->InputMouseButton(0, true);
}

- (void)mouseUp:(NSEvent *)theEvent {
    if (editorGui != NULL) ((EditorGui *)editorGui)->canvas()->InputMouseButton(0, false);
}

- (void)rightMouseDown:(NSEvent *)theEvent {
    if (editorGui != NULL) ((EditorGui *)editorGui)->canvas()->InputMouseButton(1, true);
}

- (void)rightMouseUp:(NSEvent *)theEvent {
    if (editorGui != NULL) ((EditorGui *)editorGui)->canvas()->InputMouseButton(1, false);
}

- (void)mouseMoved:(NSEvent *)theEvent {
    NSPoint pos = [self convertPoint:[theEvent locationInWindow] fromView:nil];
    if (editorGui != NULL) ((EditorGui *)editorGui)->canvas()->InputMouseMoved(pos.x, pos.y, theEvent.deltaX, theEvent.deltaY);
}

- (void)mouseDragged:(NSEvent *)theEvent {
    NSPoint pos = [self convertPoint:[theEvent locationInWindow] fromView:nil];
    if (editorGui != NULL) ((EditorGui *)editorGui)->canvas()->InputMouseMoved(pos.x, pos.y, theEvent.deltaX, theEvent.deltaY);
}

@end
