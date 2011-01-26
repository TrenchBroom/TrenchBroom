//
//  RenderContext3D.m
//  TrenchBroom
//
//  Created by Kristian Duske on 15.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "RenderContext3D.h"
#import <OpenGL/gl.h>
#import <OpenGL/glu.h>
#import "RenderBrush.h"
#import "Brush.h"
#import "Face.h"
#import "Vector3f.h"
#import "Vector2f.h"
#import "Camera.h"
#import "TextureManager.h"
#import "Texture.h"

@implementation RenderContext3D

- (id)initWithTextureManager:(TextureManager *)theTextureManager {
    if (theTextureManager == nil)
        [NSException raise:NSInvalidArgumentException format:@"texture manager must not be nil"];
    
    if (self = [self init]) {
        textureManager = [theTextureManager retain];
    }
    
    return self;
}

- (void)preRender {
}

- (void)renderBrush:(RenderBrush *)renderBrush {
    Brush* brush = [renderBrush brush];
    Vector2f* texCoords = [[Vector2f alloc] init];
    
    NSArray* faces = [brush faces];
    NSEnumerator* faceEn = [faces objectEnumerator];
    Face* face;
    while ((face = [faceEn nextObject])) {
        NSArray* vertices = [brush verticesForFace:face];
        NSString* textureName = [face texture];
//        Texture* texture = [textureManager textureForName:textureName];
//        if (texture != nil)
//            [texture activate];
        
        glBegin(GL_POLYGON);
        Vector3f* norm = [face norm];
        glNormal3f([norm x], [norm y], [norm z]);
        for (int i = 0; i < [vertices count]; i++) {
            Vector3f* vertex = [vertices objectAtIndex:i];
            [face texCoords:texCoords forVertex:vertex];
            
//            glTexCoord2f([texCoords x], [texCoords y]);
            glVertex3f([vertex x], [vertex y], [vertex z]);
        }
        glEnd();
    }
    
    [texCoords release];
}

- (void)updateView:(NSRect)bounds withCamera:(Camera *)camera {
    float fov = [camera fieldOfVision];
    float aspect = bounds.size.width / bounds.size.height;
    float near = [camera nearClippingPlane];
    float far = [camera farClippingPlane];
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(fov, aspect, near, far);
    
    Vector3f* pos = [camera position];
    Vector3f* dir = [camera direction];
    Vector3f* up = [camera up];

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt([pos x],
              [pos y],
              [pos z],
              [pos x] + [dir x],
              [pos y] + [dir y],
              [pos z] + [dir z],
              [up x],
              [up y],
              [up z]);
}

- (void)postRender {
}

- (void)dealloc {
    [textureManager release];
    [super dealloc];
}

@end
