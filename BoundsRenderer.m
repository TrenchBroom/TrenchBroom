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
        valid = NO;
        boundsSet = YES;
    }
}

- (void)render {
    if (!boundsSet)
        return;
    
    TVector3f cPos, center, size, diff;
    cPos = *[camera position];
    centerOfBounds(&bounds, &center);
    sizeOfBounds(&bounds, &size);
    subV3f(&center, &cPos, &diff);

    if (!valid) {
        [xStr release];
        xStr = nil;
        [yStr release];
        yStr = nil;
        [zStr release];
        zStr = nil;
            
        NSFont* font = [NSFont systemFontOfSize:9];
        xStr = [[fontManager glStringFor:[NSString stringWithFormat:@"%.0f", size.x] font:font] retain];
        yStr = [[fontManager glStringFor:[NSString stringWithFormat:@"%.0f", size.y] font:font] retain];
        zStr = [[fontManager glStringFor:[NSString stringWithFormat:@"%.0f", size.z] font:font] retain];
        valid = YES;
    }

    BOOL renderZ = YES;
    TVector3f xStart, xMid, xEnd, yStart, yMid, yEnd, zStart, zMid, zEnd;

    // X guide
    if (diff.y >= 0) {
        xStart = bounds.min;
        xStart.y -= 10;
        xEnd = xStart;
        xEnd.x = bounds.max.x;
    } else {
        xStart = bounds.min;
        xStart.y = bounds.max.y + 10;
        xEnd = xStart;
        xEnd.x = bounds.max.x;
    }
    
    // Y guide
    if (diff.x >= 0) {
        yStart = bounds.min;
        yStart.x -= 10;
        yEnd = yStart;
        yEnd.y = bounds.max.y;
    } else {
        yStart = bounds.min;
        yStart.x = bounds.max.x + 10;
        yEnd = yStart;
        yEnd.y = bounds.max.y;
    }
    
    if (diff.z >= 0) {
        xStart.z = bounds.max.z;
        xEnd.z = bounds.max.z;
        yStart.z = bounds.max.z;
        yEnd.z = bounds.max.z;
    }

    // Z Guide
    if (cPos.x <= bounds.min.x && cPos.y <= bounds.max.y) {
        zStart = bounds.min;
        zStart.y = bounds.max.y;
        zStart.x -= 10;
        zStart.y += 10;
        zEnd = zStart;
        zEnd.z = bounds.max.z;
    } else if (cPos.x <= bounds.max.x && cPos.y >= bounds.max.y) {
        zStart = bounds.max;
        zStart.x += 10;
        zStart.y += 10;
        zEnd = zStart;
        zEnd.z = bounds.min.z;
    } else if (cPos.x >= bounds.max.x && cPos.y >= bounds.min.y) {
        zStart = bounds.max;
        zStart.y = bounds.min.y;
        zStart.x += 10;
        zStart.y -= 10;
        zEnd = zStart;
        zEnd.z = bounds.min.z;
    } else if (cPos.x >= bounds.min.x && cPos.y <= bounds.min.y) {
        zStart = bounds.min;
        zStart.x -= 10;
        zStart.y -= 10;
        zEnd = zStart;
        zEnd.z = bounds.max.z;
    } else {
        // above, inside or below, don't render Z guide
        renderZ = NO;
    }
    
    subV3f(&xEnd, &xStart, &xMid);
    scaleV3f(&xMid, 0.5f, &xMid);
    addV3f(&xMid, &xStart, &xMid);
    
    subV3f(&yEnd, &yStart, &yMid);
    scaleV3f(&yMid, 0.5f, &yMid);
    addV3f(&yMid, &yStart, &yMid);
    
    subV3f(&zEnd, &zStart, &zMid);
    scaleV3f(&zMid, 0.5f, &zMid);
    addV3f(&zMid, &zStart, &zMid);
    
    glEnable(GL_LINE_STIPPLE);
    glLineStipple(5, 0x5555);
    glBegin(GL_LINES);
    glVertexV3f(&xStart);
    glVertexV3f(&xEnd);
    glVertexV3f(&yStart);
    glVertexV3f(&yEnd);
    if (renderZ) {
        glVertexV3f(&zStart);
        glVertexV3f(&zEnd);
    }
    glEnd();
    glDisable(GL_LINE_STIPPLE);
}

- (void)dealloc {
    [xStr release];
    [yStr release];
    [zStr release];

    [fontManager release];
    [camera release];
    [super dealloc];
}

@end
