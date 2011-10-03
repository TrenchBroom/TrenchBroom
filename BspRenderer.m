/*
Copyright (C) 2010-2011 Kristian Duske

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

#import "BspRenderer.h"
#import "Bsp.h"
#import "BspModel.h"
#import "BspFace.h"
#import "VBOBuffer.h"
#import "VBOMemBlock.h"
#import "IntData.h"
#import "Entity.h"
#import "BspTexture.h"
#import "Texture.h"

@implementation BspRenderer

- (id)initWithBsp:(Bsp *)theBsp vbo:(VBOBuffer *)theVbo palette:(NSData *)thePalette {
    NSAssert(theBsp != nil, @"BSP must not be nil");
    NSAssert(theVbo != nil, @"VBO must not be nil");
    NSAssert(thePalette != nil, @"palette must not be nil");
    
    if ((self = [self init])) {
        bsp = [theBsp retain];
        vbo = [theVbo retain];
        palette = [thePalette retain];
        textures = [[NSMutableDictionary alloc] init];
        indices = [[NSMutableDictionary alloc] init];
        counts = [[NSMutableDictionary alloc] init];
    }
    
    return self;
}

- (void)dealloc {
    [palette release];
    [bsp release];
    [block free];
    [vbo release];
    [textures release];
    [indices release];
    [counts release];
    [super dealloc];
}

- (void)renderWithEntity:(id<Entity>)theEntity {
    [self renderAtOrigin:[theEntity origin] angle:[theEntity angle]];
}

- (void)renderAtOrigin:(TVector3i *)theOrigin angle:(NSNumber *)theAngle {
    if (block == nil) {
        [vbo mapBuffer];
        
        BspModel* model = [[bsp models] objectAtIndex:0];
        int modelVertexCount = [model vertexCount];

        block = [vbo allocMemBlock:modelVertexCount * 5 * sizeof(float)];
        int address = [block address];
        uint8_t* vboBuffer = [vbo buffer];
        
        NSEnumerator* faceEn = [[model faces] objectEnumerator];
        BspFace* face;
        while ((face = [faceEn nextObject])) {
            TTextureInfo* texInfo = [face textureInfo];
            BspTexture* bspTexture = texInfo->texture;
            Texture* texture = [textures objectForKey:[bspTexture name]];
            if (texture == nil) {
                texture = [[Texture alloc] initWithBspTexture:bspTexture palette:palette];
                [textures setObject:texture forKey:[bspTexture name]];
                [texture release];
            }
            
            IntData* indexBuffer = [indices objectForKey:[texture name]];
            if (indexBuffer == nil) {
                indexBuffer = [[IntData alloc] init];
                [indices setObject:indexBuffer forKey:[texture name]];
                [indexBuffer release];
            }
            
            IntData* countBuffer = [counts objectForKey:[texture name]];
            if (countBuffer == nil) {
                countBuffer = [[IntData alloc] init];
                [counts setObject:countBuffer forKey:[texture name]];
                [countBuffer release];
            }
            
            [indexBuffer appendInt:(address - [block address]) / (5 * sizeof(float))];
            [countBuffer appendInt:[face vertexCount]];
            
            TVector3f* faceVertices = [face vertices];
            for (int i = 0; i < [face vertexCount]; i++) {
                TVector3f* vertex = &faceVertices[i];
                TVector2f texCoords;
                [face texCoords:&texCoords forVertex:vertex];
                
                address = writeVector2f(&texCoords, vboBuffer, address);
                address = writeVector3f(vertex, vboBuffer, address);
            }
        }
        
        [vbo unmapBuffer];
    }
    
    glTranslatef(theOrigin->x, theOrigin->y, theOrigin->z);
    
    if (theAngle != nil) {
        int intAngle = [theAngle intValue];
        if (intAngle == -1)
            glRotatef(90, 1, 0, 0);
        else if (intAngle == -2)
            glRotatef(-90, 1, 0, 0);
        else
            glRotatef(-intAngle, 0, 0, 1);
    }
    
    glEnable(GL_TEXTURE_2D);
    glPolygonMode(GL_FRONT, GL_FILL);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glInterleavedArrays(GL_T2F_V3F, 0, (const GLvoid *)(long)[block address]);
    
    NSEnumerator* textureEn = [textures objectEnumerator];
    Texture* texture;
    while ((texture = [textureEn nextObject])) {
        IntData* indexBuffer = [indices objectForKey:[texture name]];
        IntData* countBuffer = [counts objectForKey:[texture name]];
        
        const void* indexBytes = [indexBuffer bytes];
        const void* countBytes = [countBuffer bytes];
        long primCount = [indexBuffer count];
        
        [texture activate];
        glMultiDrawArrays(GL_POLYGON, indexBytes, countBytes, primCount);

    }
}


- (const TVector3f *)center {
    BspModel* bspModel = [[bsp models] objectAtIndex:0];
    return [bspModel center];
}

- (const TBoundingBox *)bounds {
    BspModel* bspModel = [[bsp models] objectAtIndex:0];
    return [bspModel bounds];
}

- (const TBoundingBox *)maxBounds {
    BspModel* bspModel = [[bsp models] objectAtIndex:0];
    return [bspModel maxBounds];
}
@end
