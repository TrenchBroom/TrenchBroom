//
//  EntityModel.m
//  TrenchBroom
//
//  Created by Kristian Duske on 10.06.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "Alias.h"
#import "AliasSkin.h"
#import "AliasNormals.h"
#import "AliasFrame.h"
#import "AliasFrameGroup.h"
#import "IO.h"

int const MDL_HEADER_SCALE = 0x8;
int const MDL_HEADER_ORIGIN = 0x14;
int const MDL_HEADER_RADIUS = 0x20;
int const MDL_HEADER_OFFSETS = 0x24;
int const MDL_HEADER_NUMSKINS = 0x30;
int const MDL_HEADER_SKINWIDTH = 0x34;
int const MDL_HEADER_SKINHEIGHT = 0x38;
int const MDL_HEADER_NUMVERTS = 0x3C;
int const MDL_HEADER_NUMTRIS = 0x40;
int const MDL_HEADER_NUMFRAMES = 0x44;
int const MDL_HEADER_SYNCTYPE = 0x48;
int const MDL_HEADER_FLAGS = 0x4C;
int const MDL_HEADER_SIZE = 0x50;
int const MDL_SKINS = 0x54;
int const MDL_SKIN_GROUP = 0x0;
int const MDL_SKIN_NUMPICS = 0x4;
int const MDL_SKIN_TIME = 0x8;
int const MDL_SINGLE_SKIN = 0x4;
int const MDL_MULTI_SKIN_NUMPICS = 0x4;
int const MDL_MULTI_SKIN_TIMES = 0x8;
int const MDL_MULTI_SKIN = 0xC;
int const MDL_VERTEX_ONSEAM = 0x0;
int const MDL_VERTEX_S = 0x4;
int const MDL_VERTEX_T = 0x8;
int const MDL_VERTEX_SIZE = 0xC;
int const MDL_TRI_FRONT = 0x0;
int const MDL_TRI_VERTICES = 0x4;
int const MDL_TRI_SIZE = 0x10;
int const MDL_FRAME_VERTEX_SIZE = 0x4;
int const MDL_SINGLE_FRAME_MIN = 0x4;
int const MDL_SINGLE_FRAME_MAX = 0x8;
int const MDL_SINGLE_FRAME_NAME = 0xC;
int const MDL_SINGLE_FRAME_NAME_SIZE = 0x10;
int const MDL_SINGLE_FRAME_VERTICES = 0x1C;
int const MDL_MULTI_FRAME_NUMFRAMES = 0x4;
int const MDL_MULTI_FRAME_MIN = 0x8;
int const MDL_MULTI_FRAME_MAX = 0xC;
int const MDL_MULTI_FRAME_TIMES = 0x10;

void readFrameVertex(NSData* data, int address, TPackedFrameVertex* vertex) {
    vertex->x = readUChar(data, address);
    vertex->y = readUChar(data, address + 1);
    vertex->z = readUChar(data, address + 2);
    vertex->i = readUChar(data, address + 3);
}

void unpackFrameVertex(const TPackedFrameVertex* packedVertex, const TVector3f* origin, const TVector3f* size, TFrameVertex* vertex) {
    vertex->position.x = size->x * packedVertex->x + origin->x;
    vertex->position.y = size->y * packedVertex->y + origin->y;
    vertex->position.z = size->z * packedVertex->z + origin->z;
    vertex->norm = AliasNormals[packedVertex->i];
}

void setTexCoordinates(const TSkinVertex* skinVertex, BOOL front, int skinWidth, int skinHeight, TFrameVertex* vertex) {
    vertex->texCoords.x = (float)skinVertex->s / skinWidth;
    vertex->texCoords.y = (float)skinVertex->t / skinHeight;
    if (skinVertex->onseam && !front)
        vertex->texCoords.x += 0.5f;
}

AliasFrame* readFrame(NSData* data, int address, TVector3f* origin, TVector3f* scale, int skinWidth, int skinHeight, TSkinVertex* vertices, int vertexCount, TSkinTriangle* triangles, int triangleCount) {
    NSString* name = readString(data, NSMakeRange(address + MDL_SINGLE_FRAME_NAME, MDL_SINGLE_FRAME_NAME_SIZE));
    
    TPackedFrameVertex packedFrameVertices[vertexCount];
    for (int i = 0; i < vertexCount; i++)
        readFrameVertex(data, address + MDL_SINGLE_FRAME_VERTICES + i * MDL_FRAME_VERTEX_SIZE, &packedFrameVertices[i]);
    
    TFrameTriangle frameTriangles[triangleCount];
    for (int i = 0; i < triangleCount; i++) {
        for (int j = 0; j < 3; j++) {
            int index = triangles[i].vertices[j];
            unpackFrameVertex(&packedFrameVertices[index], origin, scale, &frameTriangles[i].vertices[j]);
            setTexCoordinates(&vertices[index], triangles[i].front, skinWidth, skinHeight, &frameTriangles[i].vertices[j]);
        }
    }
    
    return [[[AliasFrame alloc] initWithName:name triangles:frameTriangles triangleCount:triangleCount] autorelease];
}

@implementation Alias

- (id)initWithName:(NSString *)theName data:(NSData *)theData {
    NSAssert(theName != nil, @"name must not be nil");
    NSAssert(theData != nil, @"data must not be nil");
    
    if ((self = [self init])) {
        name = [[NSString alloc] initWithString:theName];
        
        TVector3f scale = readVector3f(theData, MDL_HEADER_SCALE);
        TVector3f origin = readVector3f(theData, MDL_HEADER_ORIGIN);

        int skinCount = readLong(theData, MDL_HEADER_NUMSKINS);
        int skinWidth = readLong(theData, MDL_HEADER_SKINWIDTH);
        int skinHeight = readLong(theData, MDL_HEADER_SKINHEIGHT);
        int skinSize = skinWidth * skinHeight;
        
        int vertexCount = readLong(theData, MDL_HEADER_NUMVERTS);
        int triangleCount = readLong(theData, MDL_HEADER_NUMTRIS);
        int frameCount = readLong(theData, MDL_HEADER_NUMFRAMES);

        skins = [[NSMutableArray alloc] initWithCapacity:skinCount];
        int address = MDL_SKINS;
        for (int i = 0; i < skinCount; i++) {
            int skinGroup = readLong(theData, address);
            if (skinGroup == 0) {
                NSData* skinPicture = [theData subdataWithRange:NSMakeRange(address + MDL_SINGLE_SKIN, skinSize)];
                AliasSkin* skin = [[AliasSkin alloc] initSingleSkin:skinPicture width:skinWidth height:skinHeight];
                [skins addObject:skin];
                [skin release];
                address += 4 + skinSize;
            } else {
                int numPics = readLong(theData, address + MDL_MULTI_SKIN_NUMPICS);
                float times[numPics];
                NSMutableArray* skinPictures = [[NSMutableArray alloc] initWithCapacity:numPics];
                
                for (int j = 0; j < numPics; j++) {
                    times[j] = readFloat(theData, address + MDL_MULTI_SKIN_TIMES + j * 4);
                    NSData* skinPicture = [theData subdataWithRange:NSMakeRange(address + MDL_MULTI_SKIN_TIMES + numPics * 4 + j * skinSize, skinSize)];
                    [skinPictures addObject:skinPicture];
                    [skinPicture release];
                }
                
                AliasSkin* skin = [[AliasSkin alloc] initMultiSkin:skinPictures times:times width:skinWidth height:skinHeight];
                [skins addObject:skin];
                [skin release];
                [skinPictures release];
                
                address += 8 + numPics * (4 + skinSize);
            }
        }
        
        // now address points to the first skin vertex
        TSkinVertex vertices[vertexCount];
        for (int i = 0; i < vertexCount; i++) {
            vertices[i].onseam = readLong(theData, address + MDL_VERTEX_ONSEAM) != 0;
            vertices[i].s = readLong(theData, address + MDL_VERTEX_S);
            vertices[i].t = readLong(theData, address + MDL_VERTEX_T);
            address += MDL_VERTEX_SIZE;
        }
        
        // now address points to the first skin triangle
        TSkinTriangle triangles[triangleCount];
        for (int i = 0; i < triangleCount; i++) {
            triangles[i].front = readLong(theData, address + MDL_TRI_FRONT);
            for (int j = 0; j < 3; j++)
                triangles[i].vertices[j] = readLong(theData, address + MDL_TRI_VERTICES + j * 4);
            address += MDL_TRI_SIZE;
        }
        
        // now address points to the first frame
        frames = [[NSMutableArray alloc] init];
        for (int i = 0; i < frameCount; i++) {
            int type = readLong(theData, address);
            if (type == 0) { // single frame
                [frames addObject:readFrame(theData, address, &origin, &scale, skinWidth, skinHeight, vertices, vertexCount, triangles, triangleCount)];
            } else { // frame group
                int groupFrameCount = readLong(theData, address + MDL_MULTI_FRAME_NUMFRAMES);
                int timeAddress = address + MDL_MULTI_FRAME_TIMES;
                int frameAddress = address + MDL_MULTI_FRAME_TIMES + groupFrameCount * 0x4;
                
                float groupFrameTimes[groupFrameCount];
                NSMutableArray* groupFrames = [[NSMutableArray alloc] initWithCapacity:groupFrameCount];
                for (int j = 0; j < groupFrameCount; j++) {
                    groupFrameTimes[j] = readFloat(theData, timeAddress);
                    timeAddress += 0x4;
                    
                    [groupFrames addObject:readFrame(theData, frameAddress, &origin, &scale, skinWidth, skinHeight, vertices, vertexCount, triangles, triangleCount)];
                    frameAddress += MDL_SINGLE_FRAME_NAME_SIZE + (vertexCount + 2) * MDL_FRAME_VERTEX_SIZE;
                }
                
                AliasFrameGroup* frameGroup = [[AliasFrameGroup alloc] initWithFrames:groupFrames times:groupFrameTimes];
                [frames addObject:frameGroup];
                [frameGroup release];
                [groupFrames release];
            }
        }
    }
    
    return self;
}

- (void)dealloc {
    [name release];
    [frames release];
    [skins release];
    [super dealloc];
}

- (NSString *)name {
    return name;
}

- (AliasFrame *)firstFrame {
    id frame = [frames objectAtIndex:0];
    if ([frame isKindOfClass:[AliasFrame class]])
        return frame;
    
    AliasFrameGroup* group = (AliasFrameGroup *)frame;
    return [group frameAtIndex:0];
}

- (AliasSkin *)firstSkin {
    return [skins objectAtIndex:0];
}

- (AliasSkin *)skinWithIndex:(int)theSkinIndex {
    NSAssert(theSkinIndex >= 0 && theSkinIndex < [skins count], @"skin index out of range");
    return [skins objectAtIndex:theSkinIndex];
}

@end
