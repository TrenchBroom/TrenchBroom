//
//  MapWriter.m
//  TrenchBroom
//
//  Created by Kristian Duske on 06.03.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "MapWriter.h"
#import "Map.h"
#import "Entity.h"
#import "Brush.h"
#import "Face.h"
#import "SelectionManager.h"

static int BUF_SIZE = 16;

@implementation MapWriter

- (id)init {
    if ((self = [super init])) {
        buffer = malloc(BUF_SIZE);
    }

    return self;
}

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

- (void)writeString:(NSString *)theString toStream:(NSOutputStream *)theStream {
    NSUInteger count;
    
    NSRange range = NSMakeRange(0, [theString length]);
    
    while (range.length > 0) {
        [theString getBytes:buffer 
                  maxLength:BUF_SIZE 
                 usedLength:&count 
                   encoding:NSASCIIStringEncoding 
                    options:0 
                      range:range 
             remainingRange:&range];
        [theStream write:buffer maxLength:count];
    }
}

- (void)writePoint:(TVector3i *)thePoint toStream:(NSOutputStream *)theStream {
    NSString* t = [[NSString alloc] initWithFormat:@"( %i %i %i )", thePoint->x, thePoint->y, thePoint->z];
    [self writeString:t toStream:theStream];
    [t release];
}

- (void)writeFace:(id <Face>)theFace toStream:(NSOutputStream *)theStream {
    [self writePoint:[theFace point1] toStream:theStream];
    [self writeString:@" " toStream:theStream];
    [self writePoint:[theFace point2] toStream:theStream];
    [self writeString:@" " toStream:theStream];
    [self writePoint:[theFace point3] toStream:theStream];
    
    NSString* flags = [[NSString alloc] initWithFormat:@" %@ %i %i %f %f %f\n", [theFace texture], [theFace xOffset], [theFace yOffset], [theFace rotation], [theFace xScale], [theFace yScale]];
    [self writeString:flags toStream:theStream];
    [flags release];
}

- (void)writeBrush:(id <Brush>)theBrush toStream:(NSOutputStream *)theStream {
    [self writeString:@"{\n" toStream:theStream];
    
    NSEnumerator* faceEn = [[theBrush faces] objectEnumerator];
    id <Face> face;
    while ((face = [faceEn nextObject]))
        [self writeFace:face toStream:theStream];
    
    [self writeString:@"}\n" toStream:theStream];
}

- (void)writeEntityHeaderToStream:(NSOutputStream *)theStream {
    [self writeString:@"{\n" toStream:theStream];
}

- (void)writeProperty:(NSString *)theKey value:(NSString *)theValue toStream:(NSOutputStream *)theStream {
    [self writeString:@"\""     toStream:theStream];
    [self writeString:theKey    toStream:theStream];
    [self writeString:@"\" \""  toStream:theStream];
    [self writeString:theValue  toStream:theStream];
    [self writeString:@"\"\n"   toStream:theStream];
}

- (void)writeEntityFooterToStream:(NSOutputStream *)theStream {
    [self writeString:@"}\n" toStream:theStream];
}

- (void)writeEntity:(id <Entity>)theEntity toStream:(NSOutputStream *)theStream {
    [self writeEntityHeaderToStream:theStream];
    
    NSDictionary* properties = [theEntity properties];
    NSEnumerator* keyEn = [properties keyEnumerator];
    NSString* key;
    while ((key = [keyEn nextObject])) {
        NSString* value = [properties objectForKey:key];
        [self writeProperty:key value:value toStream:theStream];
    }

    NSEnumerator* brushEn = [[theEntity brushes] objectEnumerator];
    id <Brush> brush;
    while ((brush = [brushEn nextObject]))
        [self writeBrush:brush toStream:theStream];

    [self writeEntityFooterToStream:theStream];
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
                            [self writeEntityHeaderToStream:theStream];
                            [self writeProperty:ClassnameKey value:WorldspawnClassname toStream:theStream];
                            worldspawnStarted = YES;
                        }
                        
                        [self writeBrush:brush toStream:theStream];
                    }
                }
                
                if (worldspawnStarted)
                    [self writeEntityFooterToStream:theStream];
                

                NSEnumerator* entityEn = [entities objectEnumerator];
                id <Entity> entity;
                while ((entity = [entityEn nextObject]))
                    if (![entity isWorldspawn])
                        [self writeEntity:entity toStream:theStream];
                break;
            }
            case SM_BRUSHES: {
                [self writeEntityHeaderToStream:theStream];
                [self writeProperty:ClassnameKey value:WorldspawnClassname toStream:theStream];

                NSEnumerator* brushEn = [[selection selectedBrushes] objectEnumerator];
                id <Brush> brush;
                while ((brush = [brushEn nextObject]))
                    [self writeBrush:brush toStream:theStream];
                
                [self writeEntityFooterToStream:theStream];
                break;
            }
            case SM_FACES: {
                NSEnumerator* faceEn = [[selection selectedFaces] objectEnumerator];
                id <Face> face;
                while ((face = [faceEn nextObject]))
                    [self writeFace:face toStream:theStream];
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

- (void)dealloc {
    free(buffer);
    [super dealloc];
}

@end
