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
#import "VBOBuffer.h"
#import "VBOArrayEntry.h"

@implementation RenderContext3D

- (id)initWithTextureManager:(TextureManager *)theTextureManager vboBuffer:(VBOBuffer *)theVboBuffer {
    if (theTextureManager == nil)
        [NSException raise:NSInvalidArgumentException format:@"texture manager must not be nil"];
    if (theVboBuffer == nil)
        [NSException raise:NSInvalidArgumentException format:@"VBO buffer manager must not be nil"];
    
    if (self = [self init]) {
        textureManager = [theTextureManager retain];
        vboBuffer = [theVboBuffer retain];
        arrayInfoForTexture = [[NSMutableDictionary alloc] init];
    }
    
    return self;
}

- (void)preRender {
    glEnable(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glFrontFace(GL_CW);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPolygonMode(GL_FRONT, GL_FILL);
    glShadeModel(GL_FLAT);

    [arrayInfoForTexture removeAllObjects];
    [vboBuffer activate];
    [vboBuffer mapBuffer];
}

- (void)renderBrush:(RenderBrush *)renderBrush {
    NSDictionary* brushArrayInfos = [renderBrush prepareWithTextureManager:textureManager];
    NSEnumerator* textureNameEn = [brushArrayInfos keyEnumerator];
    NSString* textureName;
    while ((textureName = [textureNameEn nextObject])) {
        NSArray* brushInfos = [brushArrayInfos objectForKey:textureName];
        NSMutableArray* infos = [arrayInfoForTexture objectForKey:textureName];
        if (infos == nil) {
            infos = [[NSMutableArray alloc] initWithArray:brushInfos];
            [arrayInfoForTexture setObject:infos forKey:textureName];
            [infos release];
        } else {
            [infos addObjectsFromArray:brushInfos];
        }
    }
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
    [vboBuffer unmapBuffer];
    
    NSEnumerator* textureNameEn = [arrayInfoForTexture keyEnumerator];
    NSString* textureName;
    while ((textureName = [textureNameEn nextObject])) {
        NSArray* infos = [arrayInfoForTexture objectForKey:textureName];
        NSMutableData* indexArray = [[NSMutableData alloc] init];
        NSMutableData* countArray = [[NSMutableData alloc] init];

        NSEnumerator* infoEn = [infos objectEnumerator];
        VBOArrayEntry* entry;
        while ((entry = [infoEn nextObject])) {
            int index = [entry index];
            int count = [entry count];
            [indexArray appendBytes:(char *)&index length:sizeof(int)];
            [countArray appendBytes:(char *)&count length:sizeof(int)];
        }

        Texture* texture = [textureManager textureForName:textureName];
        [texture activate];
        
        const void* indexBuffer = [indexArray bytes];
        const void* countBuffer = [countArray bytes];
        
        glInterleavedArrays(GL_T2F_V3F, 0, NULL);
        glMultiDrawArrays(GL_POLYGON, indexBuffer, countBuffer, [infos count]);
        
        [indexArray release];
        [countArray release];
    }
    
    [vboBuffer deactivate];
}

- (void)dealloc {
    [arrayInfoForTexture release];
    [vboBuffer release];
    [textureManager release];
    [super dealloc];
}

@end
