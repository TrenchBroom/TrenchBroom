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

#import "EntityView.h"
#import "Math.h"
#import "Camera.h"
#import "EntityDefinition.h"
#import "GLResources.h"
#import "GLFontManager.h"
#import "GLString.h"
#import "EntityDefinitionLayout.h"
#import "EntityDefinitionLayoutCell.h"
#import "EntityRendererManager.h"
#import "EntityRenderer.h"
#import "DragImageWindowController.h"
#import "EntityDefinitionViewTarget.h"

@interface EntityView (private)

- (NSImage *)dragImageWithBounds:(NSRect)theBounds;

@end

@implementation EntityView (private)

- (NSImage *)dragImageWithBounds:(NSRect)theBounds {
    [[self openGLContext] makeCurrentContext];
    
    int byteWidth = NSWidth(theBounds) * 4;
    byteWidth = (byteWidth + 3) & ~3;
    
    NSBitmapImageRep* bitmap = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes:NULL
                                                                       pixelsWide:NSWidth(theBounds)
                                                                       pixelsHigh:NSHeight(theBounds)
                                                                    bitsPerSample:8
                                                                  samplesPerPixel:4
                                                                         hasAlpha:YES
                                                                         isPlanar:NO
                                                                   colorSpaceName:NSDeviceRGBColorSpace
                                                                      bytesPerRow:byteWidth
                                                                     bitsPerPixel:8 * 4];
    
    glPixelStorei(GL_PACK_ALIGNMENT, 4);	/* Force 4-byte alignment */
    glPixelStorei(GL_PACK_ROW_LENGTH, 0);
    glPixelStorei(GL_PACK_SKIP_ROWS, 0);
    glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
    
    glReadPixels(NSMinX(theBounds), NSMinY(theBounds), NSWidth(theBounds), NSHeight(theBounds), GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, [bitmap bitmapData]);

    NSImage* image = [[NSImage alloc] initWithSize:theBounds.size];
    [image lockFocusFlipped:YES];
    [bitmap draw];
    [image unlockFocus];
    
    [bitmap release];
    return [image autorelease];
}

@end

@implementation EntityView

- (void)draggedImage:(NSImage *)image beganAt:(NSPoint)screenPoint {
    [dragImageWindowController beginDrag:dragImage mouseLocation:screenPoint];
}

- (void)draggedImage:(NSImage *)image movedTo:(NSPoint)screenPoint {
    [dragImageWindowController dragTo:screenPoint];
}

- (void)draggedImage:(NSImage *)image endedAt:(NSPoint)screenPoint operation:(NSDragOperation)operation {
    [dragImageWindowController endDragWithOperation:operation];
    [dragImage release];
}

- (void)awakeFromNib {
    [super awakeFromNib];
    
    NSString* dragPlaceholderPath = [[NSBundle mainBundle] pathForResource:@"DragPlaceholder" ofType:@"png"];
    dragPlaceholder = [[NSImage alloc] initWithContentsOfFile:dragPlaceholderPath];
    
    dragImageWindowController = [[DragImageWindowController alloc] initWithWindowNibName:@"DragImageWindow"];
    mods = [[NSArray alloc] init];
}

- (BOOL)isFlipped {
    return YES;
}

- (BOOL)acceptsFirstResponder {
    return YES;
}

- (void)reshape {
    if (layout != nil) {
        NSRect frame = [self frame];
        [layout setWidth:NSWidth(frame)];
        
        [[self openGLContext] makeCurrentContext];
        float h =  fmaxf([layout height], NSHeight([[self superview] bounds]));
        
        [[self superview] setNeedsDisplay:YES];
        [self setFrameSize:NSMakeSize(NSWidth(frame), h)];
        [self setNeedsDisplay:YES];
    }
}

- (BOOL)isCameraModifierPressed:(NSEvent *)event {
    return ([event modifierFlags] & NSShiftKeyMask) != 0;
}

- (void)mouseDown:(NSEvent *)theEvent {
    NSPoint clickPoint = [self convertPoint:[theEvent locationInWindow] fromView:nil];
    if ([theEvent clickCount] == 1) {
        if ([self isCameraModifierPressed:theEvent]) {
            draggedEntityDefinition = [layout entityDefinitionAt:clickPoint];
            dragDistance.x = 0;
            dragDistance.y = 0;
        } else {
            EntityDefinitionLayoutCell* cell = [layout cellAt:clickPoint];
            EntityDefinition* definition = [cell entityDefinition];
            
            NSRect visibleRect = [self visibleRect];
            NSRect definitionBounds = [cell entityDefinitionBounds];
            NSRect imageBounds = NSMakeRect(NSMinX(definitionBounds), NSHeight(visibleRect) - NSMaxY(definitionBounds) + NSMinY(visibleRect), NSWidth(definitionBounds), NSHeight(definitionBounds));
        
            dragImage = [[self dragImageWithBounds:imageBounds] retain];
            NSPasteboard* pasteboard = [NSPasteboard pasteboardWithName:NSDragPboard];

            NSString* definitionName = [definition name];
            [pasteboard declareTypes:[NSArray arrayWithObject:EntityDefinitionType] owner:nil];
            [pasteboard setData:[definitionName dataUsingEncoding:NSUTF8StringEncoding] forType:EntityDefinitionType];

            imageOffset = NSMakeSize(clickPoint.x - NSMinX(definitionBounds), clickPoint.y - NSMinY(definitionBounds));
            
            NSPoint imageLocation = definitionBounds.origin;
            imageLocation.y += NSHeight(definitionBounds);
            
            [self dragImage:dragPlaceholder at:imageLocation offset:NSMakeSize(0, 0) event:theEvent pasteboard:pasteboard source:self slideBack:NO];
        }
    } else if ([theEvent clickCount] == 2) {
        EntityDefinition* entityDefinition = [layout entityDefinitionAt:clickPoint];
        if (entityDefinition != nil)
            [target entityDefinitionSelected:entityDefinition];
    }
}

- (void)mouseDragged:(NSEvent *)theEvent {
    if (draggedEntityDefinition != nil) {
        dragDistance.x += [theEvent deltaX];
        dragDistance.y += [theEvent deltaY];
        [self setNeedsDisplay:YES];
    }
}

- (void)mouseUp:(NSEvent *)theEvent {
    if (draggedEntityDefinition != nil) {
        draggedEntityDefinition = nil;
        dragDistance.x = 0;
        dragDistance.y = 0;
        [self setNeedsDisplay:YES];
    }
}

- (void)drawRect:(NSRect)dirtyRect {
    NSRect visibleRect = [self visibleRect];
    
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    if (glResources != nil) {
        glEnable(GL_DEPTH_TEST);
        glFrontFace(GL_CW);
        glEnable(GL_CULL_FACE);
        glEnable(GL_BLEND);
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(1.0, 1.0);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glShadeModel(GL_FLAT);
        glEnable(GL_TEXTURE_2D);
        
        EntityRendererManager* entityRendererManager = [glResources entityRendererManager];
        [entityRendererManager activate];
        
        TVector3f origin = {0, 0, 0};
        
        NSEnumerator* rowEn = [[layout rows] objectEnumerator];
        NSArray* row;
        while ((row = [rowEn nextObject])) {
            NSEnumerator* cellEn = [row objectEnumerator];
            EntityDefinitionLayoutCell* cell;
            while ((cell = [cellEn nextObject])) {
                EntityDefinition* definition = [cell entityDefinition];
                id <EntityRenderer> renderer = [entityRendererManager entityRendererForDefinition:definition mods:mods];
                
                const TVector3f* center;
                const TBoundingBox* maxBounds;
                
                if (renderer != nil) {
                    center = [renderer center];
                    maxBounds = [renderer maxBounds];
                } else {
                    center = [definition center];
                    maxBounds = [definition maxBounds];
                }
                
                NSRect definitionBounds = [cell entityDefinitionBounds];
                NSRect cameraBounds = NSMakeRect(NSMinX(definitionBounds), NSHeight(visibleRect) - NSMaxY(definitionBounds) + NSMinY(visibleRect), NSWidth(definitionBounds), NSHeight(definitionBounds));
                glViewport(NSMinX(cameraBounds), NSMinY(cameraBounds), NSWidth(cameraBounds), NSHeight(cameraBounds));
                
                glMatrixMode(GL_PROJECTION);
                glLoadIdentity();
                gluPerspective(90, NSWidth(cameraBounds) / NSHeight(cameraBounds), 0.01f, 1000);
                
                TVector3f pos = maxBounds->max;
                subV3f(&pos, center, &pos);
                
                glMatrixMode(GL_MODELVIEW);
                glLoadIdentity();
                gluLookAt(pos.x,
                          pos.y,
                          pos.z,
                          0,
                          0,
                          0,
                          0,
                          0,
                          1);
                
                if (definition == draggedEntityDefinition) {
                    glRotatef(dragDistance.y, 1, -1, 0);
                    glRotatef(dragDistance.x + 20, 0, 0, 1);
                } else {
                    glRotatef(15, 0, 0, 1);
                }
                
                glTranslatef(-center->x, -center->y, -center->z);
                
                if (renderer != nil) {
                    [renderer renderAtOrigin:&origin angle:nil];
                } else {
                    const TVector4f* color = [definition color];
                    glColor4f(color->x, color->y, color->z, 1);
                    
                    TVector3f size;
                    sizeOfBounds([definition bounds], &size);
                    glScalef(size.x, size.y, size.z);
                    
                    glBegin(GL_LINE_LOOP);
                    glVertex3f(-0.5, -0.5, -0.5);
                    glVertex3f(+0.5, -0.5, -0.5);
                    glVertex3f(+0.5, -0.5, +0.5);
                    glVertex3f(-0.5, -0.5, +0.5);
                    glVertex3f(-0.5, +0.5, +0.5);
                    glVertex3f(+0.5, +0.5, +0.5);
                    glVertex3f(+0.5, +0.5, -0.5);
                    glVertex3f(-0.5, +0.5, -0.5);
                    glEnd();
                    
                    glBegin(GL_LINES);
                    glVertex3f(-0.5, -0.5, -0.5);
                    glVertex3f(-0.5, -0.5, +0.5);
                    glVertex3f(-0.5, +0.5, -0.5);
                    glVertex3f(-0.5, +0.5, +0.5);
                    glVertex3f(+0.5, -0.5, -0.5);
                    glVertex3f(+0.5, +0.5, -0.5);
                    glVertex3f(+0.5, -0.5, +0.5);
                    glVertex3f(+0.5, +0.5, +0.5);
                    glEnd();
                }
            }
        }
        
        [entityRendererManager deactivate];
        
        glViewport(0, 0, NSWidth(visibleRect), NSHeight(visibleRect));
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluOrtho2D(NSMinX(visibleRect), 
                   NSMaxX(visibleRect), 
                   NSMinY(visibleRect), 
                   NSMaxY(visibleRect));
        
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        gluLookAt(0, 0, 1, 0, 0, -1, 0, 1, 0);
        
        glDisable(GL_POLYGON_OFFSET_FILL);
        glPolygonMode(GL_FRONT, GL_FILL);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_TEXTURE_2D);
        
        GLFontManager* fontManager = [glResources fontManager];
        [fontManager activate];
        
        glTranslatef(0, 2 * NSMinY(visibleRect), 0);
        
        rowEn = [[layout rows] objectEnumerator];
        while ((row = [rowEn nextObject])) {
            NSEnumerator* cellEn = [row objectEnumerator];
            EntityDefinitionLayoutCell* cell;
            while ((cell = [cellEn nextObject])) {
                GLString* nameString = [cell nameString];

                glPushMatrix();
                NSRect nameBounds = [cell nameBounds];
                glTranslatef(NSMinX(nameBounds),  NSHeight(visibleRect) - NSMaxY(nameBounds), 0);
                glColor4f(1, 1, 1, 1);
                [nameString render];
                glPopMatrix();
            }
        }
        
        [fontManager deactivate];
    }
    
    [[self openGLContext] flushBuffer];
}

- (void)setGLResources:(GLResources *)theGLResources entityDefinitionManager:(EntityDefinitionManager *)theEntityDefinitionManager {
    [glResources release];
    glResources = [theGLResources retain];
    
    [entityDefinitionManager release];
    entityDefinitionManager = [theEntityDefinitionManager retain];
    
    if (glResources != nil && entityDefinitionManager != nil) {
        NSOpenGLContext* sharingContext = [[NSOpenGLContext alloc] initWithFormat:[self pixelFormat] shareContext:[glResources openGLContext]];
        [self setOpenGLContext:sharingContext];
        [sharingContext release];
        
        [layout release];
        
        GLFontManager* fontManager = [glResources fontManager];
        NSFont* font = [NSFont systemFontOfSize:13];
        layout = [[EntityDefinitionLayout alloc] initWithFontManager:fontManager font:font];
        [layout setEntityDefinitions:[entityDefinitionManager definitionsOfType:EDT_POINT sortCriterion:sortCriterion]];
    }
    
    [self reshape];
}

- (void)setMods:(NSArray *)theMods {
    [mods release];
    mods = [theMods retain];
    [self setNeedsDisplay:YES];
}

- (void)setEntityDefinitionFilter:(id <EntityDefinitionFilter>)theFilter {
    if (layout != nil) {
        [layout setEntityDefinitionFilter:theFilter];
        [self reshape];
    }
}

- (void)setSortCriterion:(EEntityDefinitionSortCriterion)criterion {
    sortCriterion = criterion;
    if (layout != nil) {
        [layout setEntityDefinitions:[entityDefinitionManager definitionsOfType:EDT_POINT sortCriterion:sortCriterion]];
        [self reshape];
    }
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [dragImageWindowController release];
    [dragPlaceholder release];
    [glResources release];
    [entityDefinitionManager release];
    [layout release];
    [mods release];
    [super dealloc];
}

@end
