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

static int BUF_SIZE = 16;

@implementation MapWriter

- (id)initWithMap:(id <Map>)theMap {
    if (self = [self init]) {
        map = [theMap retain];
        buffer = malloc(BUF_SIZE);
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

- (void)writeEntity:(id <Entity>)theEntity toStream:(NSOutputStream *)theStream {
    [self writeString:@"{\n" toStream:theStream];
    
    NSDictionary* properties = [theEntity properties];
    NSEnumerator* keyEn = [properties keyEnumerator];
    NSString* key;
    while ((key = [keyEn nextObject])) {
        NSString* value = [properties objectForKey:key];
        [self writeString:@"\""     toStream:theStream];
        [self writeString:key       toStream:theStream];
        [self writeString:@"\" \""  toStream:theStream];
        [self writeString:value     toStream:theStream];
        [self writeString:@"\"\n"   toStream:theStream];
    }

    NSEnumerator* brushEn = [[theEntity brushes] objectEnumerator];
    id <Brush> brush;
    while ((brush = [brushEn nextObject]))
        [self writeBrush:brush toStream:theStream];
    
    [self writeString:@"}\n" toStream:theStream];
}

- (void)writeToStream:(NSOutputStream *)theStream {
    id <Entity> worldspawn = [map worldspawn:NO];
    if (worldspawn != nil)
        [self writeEntity:worldspawn toStream:theStream];
    
    NSEnumerator* entityEn = [[map entities] objectEnumerator];
    id <Entity> entity;
    while ((entity = [entityEn nextObject]))
        if (entity != worldspawn)
            [self writeEntity:entity toStream:theStream];
}

- (void)writeToFileAtPath:(NSString *)thePath {
    NSOutputStream* stream = [[NSOutputStream alloc] initToFileAtPath:thePath append:NO];
    [stream open];
    [self writeToStream:stream];
    [stream close];
    [stream release];
}

- (void)dealloc {
    [map release];
    free(buffer);
    [super dealloc];
}

@end
