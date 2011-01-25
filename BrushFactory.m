//
//  BrushFactory.m
//  TrenchBroom
//
//  Created by Kristian Duske on 25.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "BrushFactory.h"
#import "Entity.h"
#import "Brush.h"
#import "Face.h"
#import "Vector3i.h"

static BrushFactory *sharedInstance = nil;

@implementation BrushFactory
+ (BrushFactory *)sharedFactory {
    @synchronized(self) {
        if (sharedInstance == nil)
            sharedInstance = [[self alloc] init];
    }
    return sharedInstance;
}

+ (id)allocWithZone:(NSZone *)zone {
    @synchronized(self) {
        if (sharedInstance == nil) {
            sharedInstance = [super allocWithZone:zone];
            return sharedInstance;  // assignment and return on first allocation
        }
    }
    return nil; // on subsequent allocation attempts return nil
}

- (Brush *)createCuboidFor:(Entity *)entity atCenter:(Vector3i *)center dimensions:(Vector3i *)dimensions texture:(NSString *)texture {
    if (entity == nil)
        [NSException raise:NSInvalidArgumentException format:@"entity must not be nil"];
    if (center == nil)
        [NSException raise:NSInvalidArgumentException format:@"center must not be nil"];
    if (dimensions == nil)
        [NSException raise:NSInvalidArgumentException format:@"dimensions must not be nil"];
    if (texture == nil)
        [NSException raise:NSInvalidArgumentException format:@"texture must not be nil"];
        

    int cx = [center x];
    int cy = [center y];
    int cz = [center z];
    
    int dx = [dimensions x] / 2;
    int dy = [dimensions y] / 2;
    int dz = [dimensions z] / 2;
    
    
    Vector3i* wsb = [[Vector3i alloc] initWithX:cx - dx y:cy - dy z:cz - dz];
    Vector3i* wst = [[Vector3i alloc] initWithX:cx - dx y:cy - dy z:cz + dz];
    Vector3i* wnb = [[Vector3i alloc] initWithX:cx - dx y:cy + dy z:cz - dz];
    Vector3i* wnt = [[Vector3i alloc] initWithX:cx - dx y:cy + dy z:cz + dz];
    Vector3i* esb = [[Vector3i alloc] initWithX:cx + dx y:cy - dy z:cz - dz];
    Vector3i* est = [[Vector3i alloc] initWithX:cx + dx y:cy - dy z:cz + dz];
    Vector3i* enb = [[Vector3i alloc] initWithX:cx + dx y:cy + dy z:cz - dz];
    Vector3i* ent = [[Vector3i alloc] initWithX:cx + dx y:cy + dy z:cz + dz];
    
    Brush* brush = [entity createBrush];
    [brush createFaceWithPoint1:wsb point2:wnb point3:wst texture:texture]; // west
    [brush createFaceWithPoint1:enb point2:esb point3:ent texture:texture]; // east
    [brush createFaceWithPoint1:wnb point2:enb point3:wnt texture:texture]; // north
    [brush createFaceWithPoint1:esb point2:wsb point3:est texture:texture]; // south
    [brush createFaceWithPoint1:est point2:wst point3:ent texture:texture]; // top
    [brush createFaceWithPoint1:enb point2:wnb point3:esb texture:texture]; // bottom
    
    [wsb release];
    [wst release];
    [wnb release];
    [wnt release];
    [esb release];
    [est release];
    [enb release];
    [ent release];
    
    return brush;
}

- (id)copyWithZone:(NSZone *)zone {
    return self;
}

- (id)retain {
    return self;
}

- (NSUInteger)retainCount {
    return UINT_MAX;  // denotes an object that cannot be released
}

- (void)release {
    //do nothing
}

- (id)autorelease {
    return self;
}

@end
