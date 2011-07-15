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
#import "Matrix4f.h"

@implementation BoundsRenderer

- (id)init {
    if ((self = [super init])) {
        brushes = [[NSMutableSet alloc] init];
        brushCounts = [[NSMutableDictionary alloc] init];
    }
    
    return self;
}

- (id)initWithCamera:(Camera *)theCamera fontManager:(GLFontManager *)theFontManager font:(NSFont *)theFont {
    NSAssert(theCamera != nil, @"camera must not be nil");
    NSAssert(theFontManager != nil, @"font manager must not be nil");
    NSAssert(theFont != nil, @"font must not be nil");
    
    if ((self = [self init])) {
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
        [widthStr release];
        widthStr = nil;
        [heightStr release];
        heightStr = nil;
        [depthStr release];
        depthStr = nil;
        
        if ([brushes count] > 0) {
            NSEnumerator* brushEn = [brushes objectEnumerator];
            id <Brush> brush = [brushEn nextObject];
            bounds = *[brush bounds];
            
            while ((brush = [brushEn nextObject]))
                mergeBoundsWithBounds(&bounds, [brush bounds], &bounds);
            
            TVector3f size;
            sizeOfBounds(&bounds, &size);
            
            NSString* width = [[NSString alloc] initWithFormat:@"%.0f", size.x];
            NSString* height = [[NSString alloc] initWithFormat:@"%.0f", size.y];
            NSString* depth = [[NSString alloc] initWithFormat:@"%.0f", size.z];
            
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
        
        TVector3f size, p, x, y, z;
        Matrix3f* m = [[Matrix4f alloc] init];
        sizeOfBounds(&bounds, &size);
        
        glBegin(GL_LINES);
        glVertex3f(bounds.min.x, bounds.min.y - 10, bounds.min.z + size.z / 2);
        glVertex3f(bounds.max.x, bounds.min.y - 10, bounds.min.z + size.z / 2);
        glVertex3f(bounds.min.x, bounds.min.y - 10, bounds.min.z + size.z / 2);
        glVertex3f(bounds.min.x, bounds.min.y - 5, bounds.min.z + size.z / 2);
        glVertex3f(bounds.max.x, bounds.min.y - 10, bounds.min.z + size.z / 2);
        glVertex3f(bounds.max.x, bounds.min.y - 5, bounds.min.z + size.z / 2);

        glVertex3f(bounds.min.x - 10, bounds.min.y, bounds.min.z + size.z / 2);
        glVertex3f(bounds.min.x - 10, bounds.max.y, bounds.min.z + size.z / 2);
        glVertex3f(bounds.min.x - 10, bounds.min.y, bounds.min.z + size.z / 2);
        glVertex3f(bounds.min.x - 5, bounds.min.y, bounds.min.z + size.z / 2);
        glVertex3f(bounds.min.x - 10, bounds.max.y, bounds.min.z + size.z / 2);
        glVertex3f(bounds.min.x - 5, bounds.max.y, bounds.min.z + size.z / 2);
        
        glVertex3f(bounds.max.x + 10, bounds.min.y + size.y / 2, bounds.min.z) ;
        glVertex3f(bounds.max.x + 10, bounds.min.y + size.y / 2, bounds.max.z) ;
        glVertex3f(bounds.max.x + 10, bounds.min.y + size.y / 2, bounds.min.z);
        glVertex3f(bounds.max.x + 5, bounds.min.y + size.y / 2, bounds.min.z);
        glVertex3f(bounds.max.x + 10, bounds.min.y + size.y / 2, bounds.max.z);
        glVertex3f(bounds.max.x + 5, bounds.min.y + size.y / 2, bounds.max.z);
        glEnd();
        
        p = bounds.min;
        p.x += size.x / 2;
        p.y -= 10;
        p.y += size.z / 2;

        subV3f([camera position], &p, &z);
        normalizeV3f(&z, &z);

        crossV3f([camera up], &z, &x);
        normalizeV3f(&x, &x);

        crossV3f(&z, &x, &y);
        normalizeV3f(&y, &y);
        
        [m setIdentity];
        [m setColumn:0 values:&x];
        [m setColumn:1 values:&y];
        [m setColumn:2 values:&z];
        [m setColumn:3 values:&p];
        [m setColumn:3 row:3 value:1];
        
        glPushMatrix();
        glMultMatrixf([m columnMajor]);
        glTranslatef(-[widthStr size].width / 2, 0, 0);
        [widthStr render];
        glPopMatrix();
        
        p = bounds.min;
        p.x -= 10;
        p.y += size.y / 2;
        p.z += size.z / 2;

        subV3f([camera position], &p, &z);
        normalizeV3f(&z, &z);

        crossV3f([camera up], &z, &x);
        normalizeV3f(&x, &x);
        
        crossV3f(&z, &x, &y);
        normalizeV3f(&y, &y);
        
        [m setIdentity];
        [m setColumn:0 values:&x];
        [m setColumn:1 values:&y];
        [m setColumn:2 values:&z];
        [m setColumn:3 values:&p];
        [m setColumn:3 row:3 value:1];
        
        glPushMatrix();
        glMultMatrixf([m columnMajor]);
        glTranslatef(-[heightStr size].width / 2, 0, 0);
        [heightStr render];
        glPopMatrix();
        
        p = bounds.min;
        p.x += size.x + 10;
        p.y += size.y / 2;
        p.z += size.z / 2;
        
        subV3f([camera position], &p, &z);
        normalizeV3f(&z, &z);
        
        crossV3f([camera up], &z, &x);
        normalizeV3f(&x, &x);
        
        crossV3f(&z, &x, &y);
        normalizeV3f(&y, &y);
        
        [m setIdentity];
        [m setColumn:0 values:&x];
        [m setColumn:1 values:&y];
        [m setColumn:2 values:&z];
        [m setColumn:3 values:&p];
        [m setColumn:3 row:3 value:1];
        
        glPushMatrix();
        glMultMatrixf([m columnMajor]);
        glTranslatef(0, -[depthStr size].height / 2, 0);
        [depthStr render];
        glPopMatrix();

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
