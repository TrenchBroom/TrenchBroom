//
//  BrushGuideRenderer.m
//  TrenchBroom
//
//  Created by Kristian Duske on 03.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "BrushGuideRenderer.h"
#import <OpenGL/gl.h>
#import "Brush.h"
#import "Camera.h"
#import "GLFontManager.h"
#import "BoundingBox.h"
#import "Vector3f.h"
#import "Matrix4f.h"

static NSString* WidthKey = @"Width";
static NSString* HeightKey = @"Height";
static NSString* DepthKey = @"Depth";

@implementation BrushGuideRenderer

- (id)init {
    if (self = [super init]) {
        brushes = [[NSMutableSet alloc] init];
        glStrings = [[NSMutableDictionary alloc] init];
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

    if ([brushes containsObject:brush])
        return;

    [brushes addObject:brush];
    
    BoundingBox* bounds = [brush bounds];
    NSString* width = [[NSString alloc] initWithFormat:@"%.0f", [[bounds size] x]];
    NSString* height = [[NSString alloc] initWithFormat:@"%.0f", [[bounds size] y]];
    NSString* depth = [[NSString alloc] initWithFormat:@"%.0f", [[bounds size] z]];
    
    GLString* widthStr = [fontManager glStringFor:width font:font];
    GLString* heightStr = [fontManager glStringFor:height font:font];
    GLString* depthStr = [fontManager glStringFor:depth font:font];
    
    NSMutableDictionary* brushStrings = [[NSMutableDictionary alloc] init];
    [brushStrings setObject:widthStr forKey:WidthKey];
    [brushStrings setObject:heightStr forKey:HeightKey];
    [brushStrings setObject:depthStr forKey:DepthKey];
    
    [glStrings setObject:brushStrings forKey:[brush brushId]];
    [brushStrings release];
    
    [width release];
    [height release];
    [depth release];
}

- (void)removeBrush:(id <Brush>)brush {
    NSAssert(brush != nil, @"brush must not be nil");

    if (![brushes containsObject:brush])
        return;
    
    [glStrings removeObjectForKey:[brush brushId]];
    [brushes removeObject:brush];
}

- (void)render {
    
    glPolygonMode(GL_FRONT, GL_FILL);
    glDisable(GL_TEXTURE_2D);
    [fontManager activate];
    
    Vector3f* p = [[Vector3f alloc] init];
    Vector3f* x = [[Vector3f alloc] init];
    Vector3f* y = [[Vector3f alloc] init];
    Vector3f* z = [[Vector3f alloc] init];
    Matrix3f* m = [[Matrix4f alloc] init];

    NSEnumerator* brushEn = [brushes objectEnumerator];
    id <Brush> brush;
    while ((brush = [brushEn nextObject])) {
        NSDictionary* brushStrings = [glStrings objectForKey:[brush brushId]];
        BoundingBox* bounds = [brush bounds];
        
        GLString* widthStr = [brushStrings objectForKey:WidthKey];
        [p setFloat:[bounds min]];
        [p addX:[[bounds size] x] / 2 y:0 z:0];

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
        [widthStr render];
        glPopMatrix();
        
        GLString* heightStr = [brushStrings objectForKey:HeightKey];
        [p setFloat:[bounds min]];
        [p addX:0 y:[[bounds size] y] / 2 z:0];
        
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
        [heightStr render];
        glPopMatrix();
        
        GLString* depthStr = [brushStrings objectForKey:DepthKey];
        [p setFloat:[bounds min]];
        [p addX:0 y:0 z:[[bounds size] z] / 2];
        
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
        [depthStr render];
        glPopMatrix();
    }
    
    [p release];
    [x release];
    [y release];
    [z release];
    [m release];
    
    [fontManager deactivate];
}

- (void)dealloc {
    [glStrings release];
    [fontManager release];
    [font release];
    [camera release];
    [brushes release];
    [super dealloc];
}

@end
