//
//  Bsp.m
//  TrenchBroom
//
//  Created by Kristian Duske on 06.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "Bsp.h"
#import "IO.h"
#import "Texture.h"
#import "BspModel.h"
#import "BspFace.h"

int const BSP_HEADER_VERSION            = 0x0;
int const BSP_DIR_ENTITIES_ADDRESS      = 0x4;
int const BSP_DIR_ENTITIES_SIZE         = 0x8;
int const BSP_DIR_PLANES_ADDRESS        = 0xC;
int const BSP_DIR_PLANES_SIZE           = 0x10;
int const BSP_DIR_TEXTURES_ADDRESS      = 0x14;
int const BSP_DIR_TEXTURES_SIZE         = 0x18;
int const BSP_DIR_VERTICES_ADDRESS      = 0x1C;
int const BSP_DIR_VERTICES_SIZE         = 0x20;
int const BSP_DIR_VISILIST_ADDRESS      = 0x24;
int const BSP_DIR_VISILIST_SIZE         = 0x28;
int const BSP_DIR_NODES_ADDRESS         = 0x2C;
int const BSP_DIR_NODES_SIZE            = 0x30;
int const BSP_DIR_TEXINFOS_ADDRESS      = 0x34;
int const BSP_DIR_TEXINFOS_SIZE         = 0x38;
int const BSP_DIR_FACES_ADDRESS         = 0x3C;
int const BSP_DIR_FACES_SIZE            = 0x40;
int const BSP_DIR_LIGHTMAPS_ADDRESS     = 0x44;
int const BSP_DIR_LIGHTMAPS_SIZE        = 0x48;
int const BSP_DIR_CLIPNODES_ADDRESS     = 0x4C;
int const BSP_DIR_CLIPNODES_SIZE        = 0x50;
int const BSP_DIR_LEAVES_ADDRESS        = 0x54;
int const BSP_DIR_LEAVES_SIZE           = 0x58;
int const BSP_DIR_LEAF_FACES_ADDRESS    = 0x5C;
int const BSP_DIR_LEAF_FACES_SIZE       = 0x60;
int const BSP_DIR_EDGES_ADDRESS         = 0x64;
int const BSP_DIR_EDGES_SIZE            = 0x68;
int const BSP_DIR_FACE_EDGES_ADDRESS    = 0x6C;
int const BSP_DIR_FACE_EDGES_SIZE       = 0x70;
int const BSP_DIR_MODEL_ADDRESS         = 0x74;
int const BSP_DIR_MODEL_SIZE            = 0x78;

int const BSP_TEXTURE_DIR_COUNT         = 0x0;
int const BSP_TEXTURE_DIR_OFFSETS       = 0x4;
int const BSP_TEXTURE_NAME              = 0x0;
int const BSP_TEXTURE_NAME_LENGTH       = 0x10;
int const BSP_TEXTURE_WIDTH             = 0x10;
int const BSP_TEXTURE_HEIGHT            = 0x14;
int const BSP_TEXTURE_MIP0              = 0x18;
int const BSP_TEXTURE_MIP1              = 0x1C;
int const BSP_TEXTURE_MIP2              = 0x20;
int const BSP_TEXTURE_MIP3              = 0x24;

int const BSP_VERTEX_SIZE               = 0xC;
int const BSP_VERTEX_X                  = 0x0;
int const BSP_VERTEX_Y                  = 0x4;
int const BSP_VERTEX_Z                  = 0x8;

int const BSP_EDGE_SIZE                 = 0x4;
int const BSP_EDGE_VERTEX0              = 0x0;
int const BSP_EDGE_VERTEX1              = 0x2;

int const BSP_FACE_SIZE                 = 0x14;
int const BSP_FACE_EDGE_INDEX           = 0x4;
int const BSP_FACE_EDGE_COUNT           = 0x8;
int const BSP_FACE_TEXINFO              = 0xA;

int const BSP_TEXINFO_SIZE              = 0x28;
int const BSP_TEXINFO_S_AXIS            = 0x0;
int const BSP_TEXINFO_S_OFFSET          = 0xC;
int const BSP_TEXINFO_T_AXIS            = 0x10;
int const BSP_TEXINFO_T_OFFSET          = 0x1C;
int const BSP_TEXINFO_TEXTURE           = 0x20;

int const BSP_FACE_EDGE_SIZE            = 0x4;

int const BSP_MODEL_SIZE                = 0x40;
int const BSP_MODEL_ORIGIN              = 0x18;
int const BSP_MODEL_FACE_INDEX          = 0x38;
int const BSP_MODEL_FACE_COUNT          = 0x3C;

@interface Bsp (private)

- (NSArray *)readTextures:(NSData *)theData address:(int)theAddress palette:(NSData *)thePalette;
- (void)readVertices:(NSData *)theData address:(int)theAddress count:(int)theCount result:(TVector3f *)theResult;
- (void)readEdges:(NSData *)theData address:(int)theAddress count:(int)theCount result:(TEdge *)theResult;
- (void)readFaces:(NSData *)theData address:(int)theAddress count:(int)theCount result:(TFace *)theResult;
- (void)readTexInfos:(NSData *)theData address:(int)theAddress count:(int)theCount textures:(NSArray *)theTextures result:(TTextureInfo *)theResult;
- (void)readFaceEdges:(NSData *)theData address:(int)theAddress count:(int)theCount result:(int *)theResult;

@end

@implementation Bsp (private)

- (NSArray *)readTextures:(NSData *)theData address:(int)theAddress palette:(NSData *)thePalette {
    int count = readLong(theData, theAddress + BSP_TEXTURE_DIR_COUNT);
    
    NSMutableArray* textures = [[NSMutableArray alloc] initWithCapacity:count];
    for (int i = 0; i < count; i++) {
        int address = theAddress + readLong(theData, theAddress + BSP_TEXTURE_DIR_OFFSETS + i * 4);
        NSString* textureName = readString(theData, NSMakeRange(address + BSP_TEXTURE_NAME, BSP_TEXTURE_NAME_LENGTH));
        int width = readULong(theData, address + BSP_TEXTURE_WIDTH);
        int height = readULong(theData, address + BSP_TEXTURE_HEIGHT);
        int mip0Address = address + readULong(theData, address + BSP_TEXTURE_MIP0);
        NSData* image = [theData subdataWithRange:NSMakeRange(mip0Address, width * height)];
        
        Texture* texture = [[Texture alloc] initWithName:textureName image:image width:width height:height palette:thePalette];
        [textures addObject:texture];
        [texture release];
    }
    
    return [textures autorelease];
}

- (void)readVertices:(NSData *)theData address:(int)theAddress count:(int)theCount result:(TVector3f *)theResult {
    for (int i = 0; i < theCount; i++)
        theResult[i] = readVector3f(theData, theAddress + i * BSP_VERTEX_SIZE);
}

- (void)readEdges:(NSData *)theData address:(int)theAddress count:(int)theCount result:(TEdge *)theResult {
    for (int i = 0; i < theCount; i++) {
        int edgeAddress = theAddress + i * BSP_EDGE_SIZE;
        theResult[i].vertex0 = readUShort(theData, edgeAddress + BSP_EDGE_VERTEX0);
        theResult[i].vertex1 = readUShort(theData, edgeAddress + BSP_EDGE_VERTEX1);
    }
}

- (void)readFaces:(NSData *)theData address:(int)theAddress count:(int)theCount result:(TFace *)theResult {
    for (int i = 0; i < theCount; i++) {
        int faceAddress = theAddress + i * BSP_FACE_SIZE;
        theResult[i].edgeIndex = readLong(theData, faceAddress + BSP_FACE_EDGE_INDEX);
        theResult[i].edgeCount = readUShort(theData, faceAddress + BSP_FACE_EDGE_COUNT);
        theResult[i].textureInfoIndex = readUShort(theData, faceAddress + BSP_FACE_TEXINFO);
    }
}

- (void)readTexInfos:(NSData *)theData address:(int)theAddress count:(int)theCount textures:(NSArray *)theTextures result:(TTextureInfo *)theResult {
    for (int i = 0; i < theCount; i++) {
        int texInfoAddress = theAddress + i * BSP_TEXINFO_SIZE;
        theResult[i].sAxis = readVector3f(theData, texInfoAddress + BSP_TEXINFO_S_AXIS);
        theResult[i].sOffset = readFloat(theData, texInfoAddress + BSP_TEXINFO_S_OFFSET);
        theResult[i].tAxis = readVector3f(theData, texInfoAddress + BSP_TEXINFO_T_AXIS);
        theResult[i].tOffset = readFloat(theData, texInfoAddress + BSP_TEXINFO_T_OFFSET);
        
        int textureIndex = readULong(theData, texInfoAddress + BSP_TEXINFO_TEXTURE);
        theResult[i].texture = [theTextures objectAtIndex:textureIndex];
    }
}

- (void)readFaceEdges:(NSData *)theData address:(int)theAddress count:(int)theCount result:(int *)theResult {
    for (int i = 0; i < theCount; i++)
        theResult[i] = readLong(theData, theAddress + i * BSP_FACE_EDGE_SIZE);
}

@end

@implementation Bsp

- (id)init {
    if ((self = [super init])) {
        models = [[NSMutableArray alloc] init];
    }
    
    return self;
}

- (id)initWithName:(NSString *)theName data:(NSData *)theData palette:(NSData *)thePalette {
    NSAssert(theName != nil, @"name must not be nil");
    NSAssert(theData != nil, @"data must not be nil");
    NSAssert(thePalette != nil, @"palette must not be nil");
    
    if ((self = [self init])) {
        name = [theName retain];
        
        int textureAddress = readLong(theData, BSP_DIR_TEXTURES_ADDRESS);
        NSArray* textures = [self readTextures:theData address:textureAddress palette:thePalette];
        
        int texInfosAddress = readLong(theData, BSP_DIR_TEXINFOS_ADDRESS);
        int texInfoCount = readLong(theData, BSP_DIR_TEXINFOS_SIZE) / BSP_TEXINFO_SIZE;
        TTextureInfo* texInfos = malloc(texInfoCount * sizeof(TTextureInfo));
        [self readTexInfos:theData address:texInfosAddress count:texInfoCount textures:textures result:texInfos];
        
        int verticesAddress = readLong(theData, BSP_DIR_VERTICES_ADDRESS);
        int vertexCount = readLong(theData, BSP_DIR_VERTICES_SIZE) / BSP_VERTEX_SIZE;
        TVector3f* vertices = malloc(vertexCount * sizeof(TVector3f));
        [self readVertices:theData address:verticesAddress count:vertexCount result:vertices];
        
        int edgesAddress = readLong(theData, BSP_DIR_EDGES_ADDRESS);
        int edgeCount = readLong(theData, BSP_DIR_EDGES_SIZE) / BSP_EDGE_SIZE;
        TEdge* edges = malloc(edgeCount * sizeof(TEdge));
        [self readEdges:theData address:edgesAddress count:edgeCount result:edges];
        
        int facesAddress = readLong(theData, BSP_DIR_FACES_ADDRESS);
        int faceCount = readLong(theData, BSP_DIR_FACES_SIZE) / BSP_FACE_SIZE;
        TFace* faces = malloc(faceCount * sizeof(TFace));
        [self readFaces:theData address:facesAddress count:faceCount result:faces];
        
        int faceEdgesAddress = readLong(theData, BSP_DIR_FACE_EDGES_ADDRESS);
        int faceEdgesCount = readLong(theData, BSP_DIR_FACE_EDGES_SIZE) / BSP_FACE_EDGE_SIZE;
        int* faceEdges = malloc(faceEdgesCount * sizeof(int));
        [self readFaceEdges:theData address:faceEdgesAddress count:faceEdgesCount result:faceEdges];
        
        int modelsAddress = readLong(theData, BSP_DIR_MODEL_ADDRESS);
        int modelCount = readLong(theData, BSP_DIR_MODEL_SIZE) / BSP_MODEL_SIZE;
        for (int i = 0; i < modelCount; i++) {
            int modelAddress = modelsAddress + i * BSP_MODEL_SIZE;
            int faceIndex = readLong(theData, modelAddress + BSP_MODEL_FACE_INDEX);
            int faceCount = readLong(theData, modelAddress + BSP_MODEL_FACE_COUNT);
            int modelVertexCount = 0;
            
            NSMutableArray* bspFaces = [[NSMutableArray alloc] initWithCapacity:faceCount];
            for (int j = 0; j < faceCount; j++) {
                TFace* face = &faces[faceIndex + j];
                TTextureInfo* texInfo = &texInfos[face->textureInfoIndex];
                
                int faceVertexCount = face->edgeCount;
                TVector3f* faceVertices = malloc(faceVertexCount * sizeof(TVector3f));
                for (int k = 0; k < face->edgeCount; k++) {
                    int faceEdgeIndex = faceEdges[face->edgeIndex + k];
                    int vertexIndex;
                    if (faceEdgeIndex < 0)
                        vertexIndex = edges[-faceEdgeIndex].vertex1;
                    else
                        vertexIndex = edges[faceEdgeIndex].vertex0;
                    faceVertices[k] = vertices[vertexIndex];
                }
                
                BspFace* bspFace = [[BspFace alloc] initWithTextureInfo:texInfo vertices:faceVertices vertexCount:faceVertexCount];
                [bspFaces addObject:bspFace];
                [bspFace release];
                free(faceVertices);
                
                modelVertexCount += faceVertexCount;
            }
            
            BspModel* bspModel = [[BspModel alloc] initWithFaces:bspFaces vertexCount:modelVertexCount];
            [bspFaces release];
            
            [models addObject:bspModel];
            [bspModel release];
        }
        
        free(texInfos);
        free(vertices);
        free(edges);
        free(faces);
        free(faceEdges);
    }
    
    return self;
}

- (void)dealloc {
    [name release];
    [models release];
    [super dealloc];
}

- (NSString *)name {
    return name;
}

- (NSArray *)models {
    return models;
}

@end
