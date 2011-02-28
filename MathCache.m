//
//  MathCache.m
//  TrenchBroom
//
//  Created by Kristian Duske on 28.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "MathCache.h"
#import "Vector3f.h"
#import "Vector3i.h"
#import "Line3D.h"
#import "Plane3D.h"

static MathCache* sharedInstance = nil;

@implementation MathCache

+ (MathCache *)sharedCache {
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

- (id)init {
    if (self = [super init]) {
        vector3fCache = [[NSMutableArray alloc] init];
        vector3iCache = [[NSMutableArray alloc] init];
        line3DCache = [[NSMutableArray alloc] init];
        plane3DCache = [[NSMutableArray alloc] init];
    }
    
    return self;
}

- (Vector3f *)vector3f {
    if ([vector3fCache count] == 0)
        return [[Vector3f alloc] init];
    
    Vector3f* vector = [vector3fCache lastObject];
    [vector retain];
    [vector3fCache removeLastObject];
    
    return vector;
}

- (void)returnVector3f:(Vector3f *)vector {
    [vector3fCache addObject:vector];
    [vector release];
}

- (Vector3i *)vector3i {
    if ([vector3iCache count] == 0)
        return [[Vector3i alloc] init];
    
    Vector3i* vector = [vector3iCache lastObject];
    [vector retain];
    [vector3iCache removeLastObject];
    
    return vector;
}

- (void)returnVector3i:(Vector3i *)vector {
    [vector3iCache addObject:vector];
    [vector release];
}

- (Line3D *)line3D {
    if ([line3DCache count] == 0)
        return [[Line3D alloc] init];
    
    Line3D* line = [line3DCache lastObject];
    [line retain];
    [line3DCache removeLastObject];
    
    return line;
}

- (void)returnLine3D:(Line3D *)line {
    [line3DCache addObject:line];
    [line release];
}

- (Plane3D *)plane3D {
    if ([plane3DCache count] == 0)
        return [[Plane3D alloc] init];
    
    Plane3D* plane = [plane3DCache lastObject];
    [plane retain];
    [plane3DCache removeLastObject];
    
    return plane;
}

- (void)returnPlane3D:(Plane3D *)plane {
    [plane3DCache addObject:plane];
    [plane release];
}

- (void)dealloc {
    [vector3fCache release];
    [vector3iCache release];
    [line3DCache release];
    [plane3DCache release];
    [super dealloc];
}
@end
