/*
Copyright (C) 2010-2012 Kristian Duske

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

#import "MapWriter.h"
#import "Map.h"
#import "Entity.h"
#import "Brush.h"
#import "Face.h"
#import "SelectionManager.h"

static char buffer[256];

void writeString(char* theString, int length, NSOutputStream* theStream) {
    [theStream write:(void *)theString maxLength:length];
}

void writeFace(id <Face> theFace, NSOutputStream* theStream) {
    const TVector3i* p1 = [theFace point1];
    const TVector3i* p2 = [theFace point2];
    const TVector3i* p3 = [theFace point3];
    int length = sprintf(buffer,  "( %i %i %i ) ( %i %i %i ) ( %i %i %i ) %s %i %i %f %f %f\n",
                         p1->x, p1->y, p1->z,
                         p2->x, p2->y, p2->z,
                         p3->x, p3->y, p3->z,
                         [[[theFace texture] name] cStringUsingEncoding:NSASCIIStringEncoding],
                         [theFace xOffset], 
                         [theFace yOffset], 
                         [theFace rotation], 
                         [theFace xScale], 
                         [theFace yScale]);
    writeString(buffer, length, theStream);
}

void writeProperty(NSString* theKey, NSString* theValue, NSOutputStream* theStream) {
    int length = sprintf(buffer, "\"%s\" \"%s\"", [theKey cStringUsingEncoding:NSASCIIStringEncoding], [theValue cStringUsingEncoding:NSASCIIStringEncoding]);
    writeString(buffer, length, theStream);
}

@implementation MapWriter

- (id)initWithMap:(id <Map>)theMap {
    NSAssert(theMap != nil, @"map must not be nil");
    
    if ((self = [self init])) {
        map = theMap;
    }
    
    return self;
}

- (id)initWithSelection:(SelectionManager *)theSelection {
    NSAssert(theSelection != nil, @"selection must not be nil");

    if ((self = [self init])) {
        selection = theSelection;
    }
    
    return self;
}

- (void)writeBrush:(id <Brush>)theBrush toStream:(NSOutputStream *)theStream {
    writeString("{\n", 2, theStream);
    
    NSEnumerator* faceEn = [[theBrush faces] objectEnumerator];
    id <Face> face;
    while ((face = [faceEn nextObject]))
        writeFace(face, theStream);
    
    writeString("}\n", 2, theStream);
}

- (void)writeEntity:(id <Entity>)theEntity toStream:(NSOutputStream *)theStream {
    writeString("{\n", 2, theStream);
    
    NSDictionary* properties = [theEntity properties];
    NSEnumerator* keyEn = [properties keyEnumerator];
    NSString* key;
    while ((key = [keyEn nextObject])) {
        NSString* value = [properties objectForKey:key];
        writeProperty(key, value, theStream);
    }

    NSEnumerator* brushEn = [[theEntity brushes] objectEnumerator];
    id <Brush> brush;
    while ((brush = [brushEn nextObject]))
        [self writeBrush:brush toStream:theStream];

    writeString("}\n", 2, theStream);
}

- (void)writeToStream:(NSOutputStream *)theStream {
    if (map != nil) {
        id <Entity> worldspawn = [map worldspawn:NO];
        if (worldspawn != nil)
            [self writeEntity:worldspawn toStream:theStream];
        
        NSEnumerator* entityEn = [[map entities] objectEnumerator];
        id <Entity> entity;
        while ((entity = [entityEn nextObject]))
            if (entity != worldspawn)
                [self writeEntity:entity toStream:theStream];
    }
    
    if (selection != nil) {
        switch ([selection mode]) {
            case SM_ENTITIES: {
                NSEnumerator* entityEn = [[selection selectedEntities] objectEnumerator];
                id <Entity> entity;
                while ((entity = [entityEn nextObject]))
                    if (![entity isWorldspawn])
                        [self writeEntity:entity toStream:theStream];
                break;
            }
            case SM_BRUSHES_ENTITIES: {
                NSArray* entities = [selection selectedEntities];

                BOOL worldspawnStarted = NO;
                NSEnumerator* brushEn = [[selection selectedBrushes] objectEnumerator];
                id <Brush> brush;
                while ((brush = [brushEn nextObject])) {
                    id <Entity> entity = [brush entity];
                    if ([entity isWorldspawn] || [entities indexOfObjectIdenticalTo:entity] == NSNotFound) {
                        if (!worldspawnStarted) {
                            writeString("{\n", 2, theStream);
                            writeProperty(ClassnameKey, WorldspawnClassname, theStream);
                            worldspawnStarted = YES;
                        }
                        
                        [self writeBrush:brush toStream:theStream];
                    }
                }
                
                if (worldspawnStarted)
                    writeString("}\n", 2, theStream);
                

                NSEnumerator* entityEn = [entities objectEnumerator];
                id <Entity> entity;
                while ((entity = [entityEn nextObject]))
                    if (![entity isWorldspawn])
                        [self writeEntity:entity toStream:theStream];
                break;
            }
            case SM_BRUSHES: {
                writeString("{\n", 2, theStream);
                writeProperty(ClassnameKey, WorldspawnClassname, theStream);

                NSEnumerator* brushEn = [[selection selectedBrushes] objectEnumerator];
                id <Brush> brush;
                while ((brush = [brushEn nextObject]))
                    [self writeBrush:brush toStream:theStream];
                
                writeString("}\n", 2, theStream);
                break;
            }
            case SM_FACES: {
                NSEnumerator* faceEn = [[selection selectedFaces] objectEnumerator];
                id <Face> face;
                while ((face = [faceEn nextObject]))
                    writeFace(face, theStream);
                break;
            }
            default:
                break;
        }
    }
}

- (void)writeToFileAtPath:(NSString *)thePath {
    NSOutputStream* stream = [[NSOutputStream alloc] initToFileAtPath:thePath append:NO];
    [stream open];
    [self writeToStream:stream];
    [stream close];
    [stream release];
}

- (void)writeToFileAtUrl:(NSURL *)theUrl {
    NSOutputStream* stream = [[NSOutputStream alloc] initWithURL:theUrl append:NO];
    [stream open];
    [self writeToStream:stream];
    [stream close];
    [stream release];
}

@end
