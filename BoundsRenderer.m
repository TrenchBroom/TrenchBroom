//
//  BrushGuideRenderer.m
//  TrenchBroom
//
//  Created by Kristian Duske on 03.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "BoundsRenderer.h"
#import <OpenGL/gl.h>
#import "Brush.h"
#import "Camera.h"
#import "GLFontManager.h"
#import "GLString.h"
#import "BoundingBox.h"
#import "Vector3f.h"
#import "Matrix4f.h"

@implementation BoundsRenderer

- (id)init {
    if (self = [super init]) {
        brushes = [[NSMutableSet alloc] init];
        brushCounts = [[NSMutableDictionary alloc] init];
    }
    
    return self;
}

- (id)initWithCamera:(Camera *)theCamera fontManager:(GLFontManager *)theFontManager font:(NSFont *)theFont {
    NSAssert(theCamera != nil, @"camera must not be nil");
    NSAssert(theFontManager != nil, @"font manager must not be nil");
    NSAssert(theFont != nil, @"font must not be nil");
    
    if (self = [self init]) {
        camera = [theCamera retain];
        fontManager = [theFontManager retain];
        font = [theFont retain];
    }
    
    return self;
}

- (void)addBrush:(id <Brush>)brush {
    NSAssert(brush != nil, @"brush must not be nil");

    NSNumber* count = [brushCounts objectForKey:[brush brushId]];
    if (count == nil) {
        count = [[NSNumber alloc] initWithInt:1];
        [brushes addObject:brush];
        valid = NO;
    } else {
        count = [[NSNumber alloc] initWithInt:[count intValue] + 1];
    }
    [brushCounts setObject:count forKey:[brush brushId]];
    [count release];
    
}

- (void)removeBrush:(id <Brush>)brush {
    NSAssert(brush != nil, @"brush must not be nil");

    NSNumber* count = [brushCounts objectForKey:[brush brushId]];
    if (count != nil) {
        if ([count intValue] == 1) {
            [brushes removeObject:brush];
            [brushCounts removeObjectForKey:[brush brushId]];
            valid = NO;
        } else {
            count = [[NSNumber alloc] initWithInt:[count intValue] - 1];
            [brushCounts setObject:count forKey:[brush brushId]];
            [count release];
        }
    }
}

- (void)render {
    if (!valid) {
        [bounds release];
        bounds = nil;
        [widthStr release];
        widthStr = nil;
        [heightStr release];
        heightStr = nil;
        [depthStr release];
        depthStr = nil;
        
        if ([brushes count] > 0) {
            NSEnumerator* brushEn = [brushes objectEnumerator];
            id <Brush> brush = [brushEn nextObject];
            bounds = [[BoundingBox alloc] initWithBounds:[brush bounds]];
            while ((brush = [brushEn nextObject]))
                [bounds mergeBounds:[brush bounds]];
            
            NSString* width = [[NSString alloc] initWithFormat:@"%.0f", [[bounds size] x]];
            NSString* height = [[NSString alloc] initWithFormat:@"%.0f", [[bounds size] y]];
            NSString* depth = [[NSString alloc] initWithFormat:@"%.0f", [[bounds size] z]];
            
            widthStr = [[fontManager glStringFor:width font:font] retain];
            heightStr = [[fontManager glStringFor:height font:font] retain];
            depthStr = [[fontManager glStringFor:depth font:font] retain];

            [width release];
            [height release];
            [depth release];
        }
        
        valid = YES;
    }
    
    if ([brushes count] > 0) {
        glPolygonMode(GL_FRONT, GL_FILL);
        glDisable(GL_TEXTURE_2D);
        [fontManager activate];
        
        Vector3f* p = [[Vector3f alloc] init];
        Vector3f* x = [[Vector3f alloc] init];
        Vector3f* y = [[Vector3f alloc] init];
        Vector3f* z = [[Vector3f alloc] init];
        Matrix3f* m = [[Matrix4f alloc] init];
        
        glBegin(GL_LINES);
        glVertex3f([[bounds min] x], [[bounds min] y] - 10, [[bounds min] z] + [[bounds size] z] / 2);
        glVertex3f([[bounds max] x], [[bounds min] y] - 10, [[bounds min] z] + [[bounds size] z] / 2);
        glVertex3f([[bounds min] x], [[bounds min] y] - 10, [[bounds min] z] + [[bounds size] z] / 2);
        glVertex3f([[bounds min] x], [[bounds min] y] - 5, [[bounds min] z] + [[bounds size] z] / 2);
        glVertex3f([[bounds max] x], [[bounds min] y] - 10, [[bounds min] z] + [[bounds size] z] / 2);
        glVertex3f([[bounds max] x], [[bounds min] y] - 5, [[bounds min] z] + [[bounds size] z] / 2);

        glVertex3f([[bounds min] x] - 10, [[bounds min] y], [[bounds min] z] + [[bounds size] z] / 2);
        glVertex3f([[bounds min] x] - 10, [[bounds max] y], [[bounds min] z] + [[bounds size] z] / 2);
        glVertex3f([[bounds min] x] - 10, [[bounds min] y], [[bounds min] z] + [[bounds size] z] / 2);
        glVertex3f([[bounds min] x] - 5, [[bounds min] y], [[bounds min] z] + [[bounds size] z] / 2);
        glVertex3f([[bounds min] x] - 10, [[bounds max] y], [[bounds min] z] + [[bounds size] z] / 2);
        glVertex3f([[bounds min] x] - 5, [[bounds max] y], [[bounds min] z] + [[bounds size] z] / 2);
        
        glVertex3f([[bounds max] x] + 10, [[bounds min] y] + [[bounds size] y] / 2, [[bounds min] z]) ;
        glVertex3f([[bounds max] x] + 10, [[bounds min] y] + [[bounds size] y] / 2, [[bounds max] z]) ;
        glVertex3f([[bounds max] x] + 10, [[bounds min] y] + [[bounds size] y] / 2, [[bounds min] z]);
        glVertex3f([[bounds max] x] + 5, [[bounds min] y] + [[bounds size] y] / 2, [[bounds min] z]);
        glVertex3f([[bounds max] x] + 10, [[bounds min] y] + [[bounds size] y] / 2, [[bounds max] z]);
        glVertex3f([[bounds max] x] + 5, [[bounds min] y] + [[bounds size] y] / 2, [[bounds max] z]);
        glEnd();
        
        [p setFloat:[bounds min]];
        [p addX:[[bounds size] x] / 2 y:-10 z:[[bounds size] z] / 2];

        [z setFloat:[camera position]];
        [z sub:p];
        [z normalize];
        
        [x setFloat:[camera up]];
        [x cross:z];
        [x normalize];
        
        [y setFloat:z];
        [y cross:x];
        [y normalize];
        
        [m setIdentity];
        [m setColumn:0 values:x];
        [m setColumn:1 values:y];
        [m setColumn:2 values:z];
        [m setColumn:3 values:p];
        [m setColumn:3 row:3 value:1];
        
        glPushMatrix();
        glMultMatrixf([m columnMajor]);
        glTranslatef(-[widthStr size].width / 2, 0, 0);
        [widthStr render];
        glPopMatrix();
        
        [p setFloat:[bounds min]];
        [p addX:-10 y:[[bounds size] y] / 2 z:[[bounds size] z] / 2];
        
        [z setFloat:[camera position]];
        [z sub:p];
        [z normalize];
        
        [x setFloat:[camera up]];
        [x cross:z];
        [x normalize];
        
        [y setFloat:z];
        [y cross:x];
        [y normalize];
        
        [m setIdentity];
        [m setColumn:0 values:x];
        [m setColumn:1 values:y];
        [m setColumn:2 values:z];
        [m setColumn:3 values:p];
        [m setColumn:3 row:3 value:1];
        
        glPushMatrix();
        glMultMatrixf([m columnMajor]);
        glTranslatef(-[heightStr size].width / 2, 0, 0);
        [heightStr render];
        glPopMatrix();
        
        [p setFloat:[bounds min]];
        [p addX:[[bounds size] x] + 10 y:[[bounds size] y] / 2 z:[[bounds size] z] / 2];
        
        [z setFloat:[camera position]];
        [z sub:p];
        [z normalize];
        
        [x setFloat:[camera up]];
        [x cross:z];
        [x normalize];
        
        [y setFloat:z];
        [y cross:x];
        [y normalize];
        
        [m setIdentity];
        [m setColumn:0 values:x];
        [m setColumn:1 values:y];
        [m setColumn:2 values:z];
        [m setColumn:3 values:p];
        [m setColumn:3 row:3 value:1];
        
        glPushMatrix();
        glMultMatrixf([m columnMajor]);
        glTranslatef(0, -[depthStr size].height / 2, 0);
        [depthStr render];
        glPopMatrix();

        [p release];
        [x release];
        [y release];
        [z release];
        [m release];
        
        [fontManager deactivate];
    }
}

- (void)dealloc {
    [widthStr release];
    [heightStr release];
    [depthStr release];

    [fontManager release];
    [font release];
    [camera release];
    [brushes release];
    [brushCounts release];
    [super dealloc];
}

@end
