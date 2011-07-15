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
#import "GLUtils.h"

@implementation BoundsRenderer

- (id)initWithCamera:(Camera *)theCamera fontManager:(GLFontManager *)theFontManager {
    NSAssert(theCamera != nil, @"camera must not be nil");
    NSAssert(theFontManager != nil, @"font manager must not be nil");
    
    if ((self = [self init])) {
        camera = [theCamera retain];
        fontManager = [theFontManager retain];
        boundsSet = NO;
        valid = NO;
    }
    
    return self;
}

- (void)setBounds:(TBoundingBox *)theBounds {
    if (theBounds == NULL) {
        boundsSet = NO;
    } else {
        bounds = *theBounds;
        boundsSet = YES;
    }
    valid = NO;
}

- (void)renderColor:(float *)theColor {
    if (!boundsSet)
        return;
    
    TVector3f cPos, center, size, diff;
    cPos = *[camera position];
    centerOfBounds(&bounds, &center);
    sizeOfBounds(&bounds, &size);
    subV3f(&center, &cPos, &diff);

    if (!valid) {
        NSFont* font = [NSFont systemFontOfSize:9];
        for (int i = 0; i < 3; i++) {
            [glStrings[i] release];
            glStrings[i] = nil;
        }
            
        glStrings[0] = [[fontManager glStringFor:[NSString stringWithFormat:@"%.0f", size.x] font:font] retain];
        glStrings[1] = [[fontManager glStringFor:[NSString stringWithFormat:@"%.0f", size.y] font:font] retain];
        glStrings[2] = [[fontManager glStringFor:[NSString stringWithFormat:@"%.0f", size.z] font:font] retain];
        valid = YES;
    }

    int maxi = 3;
    TVector3f gv[3][4];

    // X guide
    if (diff.y >= 0) {
        gv[0][0] = bounds.min;
        gv[0][0].y -= 5;
        gv[0][1] = gv[0][0];
        gv[0][1].y -= 5;
        gv[0][2] = gv[0][1];
        gv[0][2].x = bounds.max.x;
        gv[0][3] = gv[0][0];
        gv[0][3].x = bounds.max.x;
    } else {
        gv[0][0] = bounds.min;
        gv[0][0].y = bounds.max.y + 5;
        gv[0][1] = gv[0][0];
        gv[0][1].y += 5;
        gv[0][2] = gv[0][1];
        gv[0][2].x = bounds.max.x;
        gv[0][3] = gv[0][0];
        gv[0][3].x = bounds.max.x;
    }
    
    // Y guide
    if (diff.x >= 0) {
        gv[1][0] = bounds.min;
        gv[1][0].x -= 5;
        gv[1][1] = gv[1][0];
        gv[1][1].x -= 5;
        gv[1][2] = gv[1][1];
        gv[1][2].y = bounds.max.y;
        gv[1][3] = gv[1][0];
        gv[1][3].y = bounds.max.y;
    } else {
        gv[1][0] = bounds.min;
        gv[1][0].x = bounds.max.x + 5;
        gv[1][1] = gv[1][0];
        gv[1][1].x += 5;
        gv[1][2] = gv[1][1];
        gv[1][2].y = bounds.max.y;
        gv[1][3] = gv[1][0];
        gv[1][3].y = bounds.max.y;
    }
    
    if (diff.z >= 0)
        for (int i = 0; i < 2; i++)
            for (int j = 0; j < 4; j++)
                gv[i][j].z = bounds.max.z;

    // Z Guide
    if (cPos.x <= bounds.min.x && cPos.y <= bounds.max.y) {
        gv[2][0] = bounds.min;
        gv[2][0].x -= 3.5f;
        gv[2][0].y = bounds.max.y + 3.5f;
        gv[2][1] = gv[2][0];
        gv[2][1].x -= 3.5f;
        gv[2][1].y += 3.5f;
        gv[2][2] = gv[2][1];
        gv[2][2].z = bounds.max.z;
        gv[2][3] = gv[2][0];
        gv[2][3].z = bounds.max.z;
    } else if (cPos.x <= bounds.max.x && cPos.y >= bounds.max.y) {
        gv[2][0] = bounds.max;
        gv[2][0].x += 3.5f;
        gv[2][0].y += 3.5f;
        gv[2][1] = gv[2][0];
        gv[2][1].x += 3.5f;
        gv[2][1].y += 3.5f;
        gv[2][2] = gv[2][1];
        gv[2][2].z = bounds.min.z;
        gv[2][3] = gv[2][0];
        gv[2][3].z = bounds.min.z;
    } else if (cPos.x >= bounds.max.x && cPos.y >= bounds.min.y) {
        gv[2][0] = bounds.max;
        gv[2][0].y = bounds.min.y;
        gv[2][0].x += 3.5f;
        gv[2][0].y -= 3.5f;
        gv[2][1] = gv[2][0];
        gv[2][1].x += 3.5f;
        gv[2][1].y -= 3.5f;
        gv[2][2] = gv[2][1];
        gv[2][2].z = bounds.min.z;
        gv[2][3] = gv[2][0];
        gv[2][3].z = bounds.min.z;
    } else if (cPos.x >= bounds.min.x && cPos.y <= bounds.min.y) {
        gv[2][0] = bounds.min;
        gv[2][0].x -= 3.5f;
        gv[2][0].y -= 3.5f;
        gv[2][1] = gv[2][0];
        gv[2][1].x -= 3.5f;
        gv[2][1].y -= 3.5f;
        gv[2][2] = gv[2][1];
        gv[2][2].z = bounds.max.z;
        gv[2][3] = gv[2][0];
        gv[2][3].z = bounds.max.z;
    } else {
        // above, inside or below, don't render Z guide
        maxi = 2;
    }
    
    TVector3f p[3];
    for (int i = 0; i < 3; i++) {
        subV3f(&gv[i][2], &gv[i][1], &p[i]);
        scaleV3f(&p[i], 0.5f, &p[i]);
        addV3f(&gv[i][1], &p[i], &p[i]);

        float dist = [camera distanceTo:&p[i]];
        if (dist <= 500) {
            if (dist >= 400) {
                glColor4f(theColor[0], theColor[1], theColor[2], theColor[3] - theColor[3] * (dist - 400) / 100);
            } else {
                glColor4f(theColor[0], theColor[1], theColor[2], theColor[3]);
            }
            
            glBegin(GL_LINE_STRIP);
            for (int j = 0; j < 4; j++)
                glVertexV3f(&gv[i][j]);
            glEnd();
        }
    }
    
    [fontManager activate];
    for (int i = 0; i < maxi; i++) {
        float dist = [camera distanceTo:&p[i]];
        if (dist <= 500) {
            if (dist >= 400) {
                glColor4f(theColor[0], theColor[1], theColor[2], theColor[3] - theColor[3] * (dist - 400) / 100);
            } else {
                glColor4f(theColor[0], theColor[1], theColor[2], theColor[3]);
            }
            float factor = dist / 300;
            NSSize size = [glStrings[i] size];
            
            glPushMatrix();
            glTranslatef(p[i].x, p[i].y, p[i].z);
            [camera setBillboardMatrix];
            glScalef(factor, factor, 0);
            glTranslatef(-size.width / 2, 0, 0);
            [glStrings[i] render];
            glPopMatrix();
        }
    }
    [fontManager deactivate];
}

- (void)dealloc {
    for (int i = 0; i < 3; i++)
        [glStrings[i] release];

    [fontManager release];
    [camera release];
    [super dealloc];
}

@end
