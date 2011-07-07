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
int const BSP_DIR_FACELIST_ADDRESS      = 0x5C;
int const BSP_DIR_FACELIST_SIZE         = 0x60;
int const BSP_DIR_FACEEDGES_ADDRESS     = 0x64;
int const BSP_DIR_FACEEDGES_SIZE        = 0x68;
int const BSP_DIR_EDGES_ADDRESS         = 0x6C;
int const BSP_DIR_EDGES_SIZE            = 0x70;
int const BSP_DIR_MODELLIST_ADDRESS     = 0x74;
int const BSP_DIR_MODELLIST_SIZE        = 0x78;

int const BSP_TEXTURE_DIR_COUNT         = 0x0;
int const BSP_TEXTURE_DIR_OFFSETS       = 0x4;
int const BSP_TEXTURE_NAME              = 0x0;
int const BSP_TEXTURE_NAME_LENGTH       = 0x10;
int const BSP_TEXTURE_WIDTH             = 0x10;
int const BSP_TEXTURE_HEIGHT            = 0x14;
int const BSP_TEXTURE_MIP0              = 0x1C;
int const BSP_TEXTURE_MIP1              = 0x20;
int const BSP_TEXTURE_MIP2              = 0x14;
int const BSP_TEXTURE_MIP3              = 0x28;

int const BSP_VERTEX_SIZE               = 0xC;
int const BSP_VERTEX_X                  = 0x0;
int const BSP_VERTEX_Y                  = 0x4;
int const BSP_VERTEX_Z                  = 0x8;

int const BSP_EDGE_SIZE                 = 0x4;
int const BSP_EDGE_VERTEX0              = 0x0;
int const BSP_EDGE_VERTEX1              = 0x2;

@interface Bsp (private)

- (NSArray *)readTextures:(NSData *)theData address:(int)theAddress palette:(NSData *)thePalette;
- (void)readVertices:(NSData *)theData address:(int)theAddress count:(int)theCount result:(TVector3f *)theResult;
- (void)readEdges:(NSData *)theData address:(int)theAddress count:(int)theCount vertices:(TVector3f *)theVertices result:(TEdge *)theResult;

@end

@implementation Bsp (private)

- (NSArray *)readTextures:(NSData *)theData address:(int)theAddress palette:(NSData *)thePalette {
    int count = readInt(theData, theAddress + BSP_TEXTURE_DIR_COUNT);
    
    NSMutableArray* textures = [[NSMutableArray alloc] initWithCapacity:count];
    for (int i = 0; i < count; i++) {
        int address = theAddress + readInt(theData, theAddress + BSP_TEXTURE_DIR_OFFSETS + i * 4);
        NSString* name = readString(theData, NSMakeRange(address + BSP_TEXTURE_NAME, BSP_TEXTURE_NAME_LENGTH));
        int width = readInt(theData, address + BSP_TEXTURE_WIDTH);
        int height = readInt(theData, address + BSP_TEXTURE_HEIGHT);
        int mip0Address = address + readInt(theData, address + BSP_TEXTURE_MIP0);
        NSData* image = [theData subdataWithRange:NSMakeRange(mip0Address, width * height)];
        
        Texture* texture = [[Texture alloc] initWithName:name image:image width:width height:height palette:thePalette];
        [textures addObject:texture];
        [texture release];
    }
    
    return [textures autorelease];
}

- (void)readVertices:(NSData *)theData address:(int)theAddress count:(int)theCount result:(TVector3f *)theResult {
    for (int i = 0; i < theCount; i++) {
        int vertexAddress = theAddress + i * BSP_VERTEX_SIZE;
        theResult[i].x = readFloat(theData, vertexAddress + BSP_VERTEX_X);
        theResult[i].y = readFloat(theData, vertexAddress + BSP_VERTEX_Y);
        theResult[i].z = readFloat(theData, vertexAddress + BSP_VERTEX_Z);
    }
}

- (void)readEdges:(NSData *)theData address:(int)theAddress count:(int)theCount vertices:(TVector3f *)theVertices result:(TEdge *)theResult {
    for (int i = 0; i < theCount; i++) {
        int edgeAddress = theAddress + i * BSP_EDGE_SIZE;
        int vertexIndex0 = readShort(theData, edgeAddress + BSP_EDGE_VERTEX0);
        int vertexIndex1 = readShort(theData, edgeAddress + BSP_EDGE_VERTEX1);
        theResult[i].vertex0 = &theVertices[vertexIndex0];
        theResult[i].vertex1 = &theVertices[vertexIndex1];
    }
}

@end

@implementation Bsp

- (id)init {
    if ((self = [super init])) {
        models = [[NSMutableArray alloc] init];
    }
    
    return self;
}

- (id)initWithData:(NSData *)theData palette:(NSData *)thePalette {
    NSAssert(theData != nil, @"data must not be nil");
    NSAssert(thePalette != nil, @"palette must not be nil");
    
    if ((self = [self init])) {
        int textureAddress = readInt(theData, BSP_DIR_TEXINFOS_ADDRESS);
        NSArray* textures = [self readTextures:theData address:textureAddress palette:thePalette];
        
        int verticesAddress = readInt(theData, BSP_DIR_VERTICES_ADDRESS);
        int vertexCount = readInt(theData, BSP_DIR_VERTICES_SIZE) / BSP_VERTEX_SIZE;
        TVector3f vertices[vertexCount];
        [self readVertices:theData address:verticesAddress count:vertexCount result:vertices];
        
        int edgesAddress = readInt(theData, BSP_DIR_EDGES_ADDRESS);
        int edgeCount = readInt(theData, BSP_DIR_EDGES_SIZE) / BSP_EDGE_SIZE;
        TEdge edges[edgeCount];
        [self readEdges:theData address:edgesAddress count:edgeCount vertices:vertices result:edges];
    }
    
    return self;
}

- (void)dealloc {
    [models release];
    [super dealloc];
}

@end
