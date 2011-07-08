//
//  BspRenderer.m
//  TrenchBroom
//
//  Created by Kristian Duske on 07.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "BspRenderer.h"
#import "Bsp.h"
#import "BspModel.h"
#import "BspFace.h"
#import "VBOBuffer.h"
#import "VBOMemBlock.h"
#import "IntData.h"
#import "Entity.h"

@implementation BspRenderer

- (id)initWithBsp:(Bsp *)theBsp vbo:(VBOBuffer *)theVbo {
    NSAssert(theBsp != nil, @"BSP must not be nil");
    NSAssert(theVbo != nil, @"VBO must not be nil");
    
    if ((self = [self init])) {
        bsp = [theBsp retain];
        vbo = [theVbo retain];
        textures = [[NSMutableSet alloc] init];
        indices = [[NSMutableDictionary alloc] init];
        counts = [[NSMutableDictionary alloc] init];
    }
    
    return self;
}

- (void)dealloc {
    [bsp release];
    [block free];
    [vbo release];
    [textures release];
    [indices release];
    [counts release];
    [super dealloc];
}

- (void)renderWithEntity:(id<Entity>)theEntity {
    if (block == nil) {
        [vbo mapBuffer];
        
        BspModel* model = [[bsp models] objectAtIndex:0];
        int modelVertexCount = [model vertexCount];

        block = [vbo allocMemBlock:modelVertexCount * 5 * sizeof(float)];
        int offset = 0;
        
        NSEnumerator* faceEn = [[model faces] objectEnumerator];
        BspFace* face;
        while ((face = [faceEn nextObject])) {
            TTextureInfo* texInfo = [face textureInfo];
            Texture* texture = texInfo->texture;
            
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
            
            [textures addObject:texture];
            [indexBuffer appendInt:offset / (5 * sizeof(float))];
            [countBuffer appendInt:[face vertexCount]];
            
            for (int i = 0; i < [face vertexCount]; i++) {
                TVector3f* vertex = [face vertexAtIndex:i];
                TVector2f texCoords;
                [face texCoords:&texCoords forVertex:vertex];
                
                offset = [block writeVector2f:&texCoords offset:offset];
                offset = [block writeVector3f:vertex offset:offset];
            }
        }
        
        [vbo unmapBuffer];
    }
    
    TVector3i* origin = [theEntity origin];
    glTranslatef(origin->x, origin->y, origin->z);
    
    NSNumber* angle = [theEntity angle];
    if (angle != nil) {
        int intAngle = [angle intValue];
        if (intAngle == -1)
            glRotatef(90, 1, 0, 0);
        else if (intAngle == 1)
            glRotatef(-90, 1, 0, 0);
        else
            glRotatef(intAngle, 0, 0, 1);
    }
    
    glEnable(GL_TEXTURE_2D);
    glPolygonMode(GL_FRONT, GL_FILL);
    glInterleavedArrays(GL_T2F_V3F, 0, (const GLvoid *)[block address]);
    
    NSEnumerator* textureEn = [textures objectEnumerator];
    Texture* texture;
    while ((texture = [textureEn nextObject])) {
        IntData* indexBuffer = [indices objectForKey:[texture name]];
        IntData* countBuffer = [counts objectForKey:[texture name]];
        
        const void* indexBytes = [indexBuffer bytes];
        const void* countBytes = [countBuffer bytes];
        int primCount = [indexBuffer count];
        
        [texture activate];
        glMultiDrawArrays(GL_POLYGON, indexBytes, countBytes, primCount);

    }
}
@end
