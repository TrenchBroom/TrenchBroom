//
//  AliasRenderer.m
//  TrenchBroom
//
//  Created by Kristian Duske on 12.06.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "AliasRenderer.h"
#import "Alias.h"
#import "AliasSkin.h"
#import "AliasFrame.h"
#import "Texture.h"
#import "VBOBuffer.h"
#import "VBOMemBlock.h"
#import "Math.h"
#import "Entity.h"

@implementation AliasRenderer

- (id)initWithAlias:(Alias *)theAlias skinIndex:(int)theSkinIndex vbo:(VBOBuffer *)theVbo palette:(NSData *)thePalette {
    NSAssert(theAlias != nil, @"alias must not be nil");
    NSAssert(theSkinIndex >= 0, @"skin index must be at least 0");
    NSAssert(theVbo != nil, @"VBO must not be nil");
    NSAssert(thePalette != nil, @"palette must not be nil");
    
    if ((self = [self init])) {
        alias = [theAlias retain];
        skinIndex = theSkinIndex;
        vbo = [theVbo retain];
        palette = [thePalette retain];
    }
    
    return self;
}

- (void)dealloc {
    [alias release];
    [vbo release];
    [block free];
    [texture release];
    [palette release];
    [super dealloc];
}

- (void)renderWithEntity:(id<Entity>)theEntity {
    [self renderAtOrigin:[theEntity origin] angle:[theEntity angle]];
}

- (void)renderAtOrigin:(TVector3i *)theOrigin angle:(NSNumber *)theAngle {
    if (block == nil) {
        AliasSkin* skin = [alias skinWithIndex:skinIndex];
        texture = [[Texture alloc] initWithName:[alias name] skin:skin index:0 palette:palette];
        
        AliasFrame* frame = [alias firstFrame];
        triangleCount = [frame triangleCount];
        int vertexSize = 3 * 8;
        block = [vbo allocMemBlock:triangleCount * vertexSize * sizeof(float)];
        [vbo mapBuffer];
        
        int offset = 0;
        for (int i = 0; i < triangleCount; i++) {
            const TFrameTriangle* triangle = [frame triangleAtIndex:i];
            for (int j = 0; j < 3; j++) {
                const TFrameVertex* vertex = &triangle->vertices[j];
                // GL_T2F_N3F_V3F format
                offset = [block writeVector2f:&vertex->texCoords offset:offset];
                offset = [block writeVector3f:&vertex->norm offset:offset];
                offset = [block writeVector3f:&vertex->position offset:offset];
            }
        }
        
        [vbo unmapBuffer];
    }
    
    glTranslatef(theOrigin->x, theOrigin->y, theOrigin->z);
    
    if (theAngle != nil) {
        int intAngle = [theAngle intValue];
        if (intAngle == -1)
            glRotatef(90, 1, 0, 0);
        else if (intAngle == 1)
            glRotatef(-90, 1, 0, 0);
        else
            glRotatef(-intAngle, 0, 0, 1);
    }
    
    glEnable(GL_TEXTURE_2D);
    glPolygonMode(GL_FRONT, GL_FILL);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    [texture activate];
    
    glInterleavedArrays(GL_T2F_N3F_V3F, 0, (const GLvoid *)(long)[block address]);
    glDrawArrays(GL_TRIANGLES, 0, triangleCount * 3);
    
    [texture deactivate];
}

- (const TVector3f *)center {
    return [[alias firstFrame] center];
}

- (const TBoundingBox *)bounds {
    return [[alias firstFrame] bounds];
}

- (const TBoundingBox *)maxBounds {
    return [[alias firstFrame] maxBounds];
}

@end
