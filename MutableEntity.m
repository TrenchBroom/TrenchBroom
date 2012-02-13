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

#import "MutableEntity.h"
#import "Brush.h"
#import "MutableBrush.h"
#import "IdGenerator.h"
#import "VBOMemBlock.h"
#import "PickingHitList.h"
#import "PickingHit.h"
#import "EntityDefinition.h"
#import "EntityDefinitionManager.h"

@interface MutableEntity (private)

- (void)validate;

@end

@implementation MutableEntity (private)

- (void)validate {
    if (entityDefinition == nil || [entityDefinition type] == EDT_BRUSH) {
        if ([brushes count] > 0) {
            NSEnumerator* brushEn = [brushes objectEnumerator];
            MutableBrush* brush = [brushEn nextObject];
            
            bounds = *[brush bounds];
            while ((brush = [brushEn nextObject]))
                mergeBoundsWithBounds(&bounds, [brush bounds], &bounds);
            
            centerOfBounds(&bounds, &center);

            TVector3f diff;
            subV3f(&bounds.max, &center, &diff);
            float dist = lengthV3f(&diff);
            diff.x = dist;
            diff.y = dist;
            diff.z = dist;
            subV3f(&center, &diff, &maxBounds.min);
            addV3f(&center, &diff, &maxBounds.max);
        } else {
            bounds.min = NullVector;
            bounds.max = NullVector;
            center = NullVector;
            maxBounds.min = NullVector;
            maxBounds.max = NullVector;
        }
    } else if ([entityDefinition type] == EDT_POINT) {
        bounds = *[entityDefinition bounds];
        
        TVector3f of;
        setV3f(&of, &origin);
        
        addV3f(&bounds.min, &of, &bounds.min);
        addV3f(&bounds.max, &of, &bounds.max);
        centerOfBounds(&bounds, &center);

        TVector3f diff;
        subV3f(&bounds.max, &center, &diff);
        float dist = lengthV3f(&diff);
        diff.x = dist;
        diff.y = dist;
        diff.z = dist;
        subV3f(&center, &diff, &maxBounds.min);
        addV3f(&center, &diff, &maxBounds.max);
    } else {
        bounds.min = NullVector;
        bounds.max = NullVector;
        center = NullVector;
        maxBounds.min = NullVector;
        maxBounds.max = NullVector;
    }
    
    valid = YES;
}

@end

@implementation MutableEntity

- (id)init {
    if ((self = [super init])) {
        entityId = [[[IdGenerator sharedGenerator] getId] retain];
		properties = [[NSMutableDictionary alloc] init];
		brushes = [[NSMutableArray alloc] init];
        filePosition = -1;
    }
    
    return self;
}

- (id)initWithProperties:(NSDictionary *)theProperties {
    if ((self = [self init])) {
        NSEnumerator* keyEn = [[theProperties allKeys] objectEnumerator];
        NSString* key;
        while ((key = [keyEn nextObject])) {
            NSString* value = [theProperties objectForKey:key];
            [self setProperty:key value:value];
        }
    }
    
    return self;
}


- (void) dealloc {
    [boundsMemBlock free];
    [entityId release];
	[properties release];
	[brushes release];
    [entityDefinition decUsageCount];
    [entityDefinition release];
    [angle release];
	[super dealloc];
}

- (id)copyWithZone:(NSZone *)zone {
    MutableEntity* result = [[MutableEntity allocWithZone:zone] initWithProperties:properties];
    [result->entityId release];
    result->entityId = [entityId retain];
    
    if (entityDefinition != nil)
        [result setEntityDefinition:entityDefinition];
    [result setMap:map];
    [result setFilePosition:filePosition];
    
    NSEnumerator* brushEn = [brushes objectEnumerator];
    id <Brush> brush;
    while ((brush = [brushEn nextObject])) {
        id <Brush> brushCopy = (id <Brush>) [brush copy];
        [result addBrush:brushCopy];
        [brushCopy release];
    }
    
    return result;
}

- (void)addBrush:(MutableBrush *)brush {
    NSAssert(brush != nil, @"brush must not be nil");
    
    if (entityDefinition != nil && [entityDefinition type] != EDT_BRUSH)
            [NSException raise:NSInternalInconsistencyException format:@"Cannot add brush to point or base entity"];
    
    
    [brushes addObject:brush];
    [brush setEntity:self];
    valid = NO;
}

- (void)removeBrush:(MutableBrush *)brush {
    NSAssert(brush != nil, @"brush must not be nil");

    if (entityDefinition != nil && [entityDefinition type] != EDT_BRUSH)
            [NSException raise:NSInternalInconsistencyException format:@"Cannot remove brush from point or base entity"];
    
    [brush setEntity:nil];
    [brushes removeObject:brush];
    valid = NO;
}

- (void)brushChanged:(MutableBrush *)brush {
    valid = NO;
}

- (void)translateBy:(const TVector3i *)theDelta {
    if (entityDefinition == nil || [entityDefinition type] != EDT_POINT)
        return;
    
    if (!valid)
        [self validate];
    
    TVector3i o;
    addV3i([self origin], theDelta, &o);
    [self setProperty:OriginKey value:[NSString stringWithFormat:@"%i %i %i", o.x, o.y, o.z]];
}

- (void)rotate90CW:(EAxis)theAxis center:(const TVector3i *)theCenter {
    if (entityDefinition == nil || [entityDefinition type] != EDT_POINT)
        return;

    if (!valid)
        [self validate];
    
    TVector3i o, ci, d;
    roundV3f(&center, &ci);
    subV3i([self origin], &ci, &d);
    subV3i(&ci, theCenter, &ci);
    
    rotate90CWV3i(&ci, theAxis, &ci);
    
    addV3i(&ci, theCenter, &ci);
    addV3i(&ci, &d, &o);
    
    [self setProperty:OriginKey value:[NSString stringWithFormat:@"%i %i %i", o.x, o.y, o.z]];

    if ([self angle] == nil)
        [self setProperty:AngleKey value:@"0"];
    
    int a = [[self angle] intValue];
    TVector3f direction;
    
    if (a >= 0) {
        direction.x = cos(a * M_PI / 180);
        direction.y = sin(a * M_PI / 180);
        direction.z = 0;
    } else if (a == -1) {
        direction = ZAxisPos;
    } else if (a == -2) {
        direction = ZAxisNeg;
    } else {
        return;
    }
    
    rotate90CWV3f(&direction, theAxis, &direction);
    if (direction.z > 0.9) {
        [self setProperty:AngleKey value:@"-1"];
    } else if (direction.z < -0.9) {
        [self setProperty:AngleKey value:@"-2"];
    } else {
        if (direction.z != 0) {
            direction.z = 0;
            normalizeV3f(&direction, &direction);
        }
        
        a = roundf(acos(direction.x) * 180 / M_PI);
        if (direction.y > 0)
            a = 360 - a;
        
        [self setProperty:AngleKey value:[NSString stringWithFormat:@"%i", a]];
    }

}

- (void)rotate90CCW:(EAxis)theAxis center:(const TVector3i *)theCenter {
    if (entityDefinition == nil || [entityDefinition type] != EDT_POINT)
        return;

    if (!valid)
        [self validate];
    
    TVector3i o, ci, d;
    roundV3f(&center, &ci);
    subV3i([self origin], &ci, &d);
    subV3i(&ci, theCenter, &ci);

    rotate90CCWV3i(&ci, theAxis, &ci);
    
    addV3i(&ci, theCenter, &ci);
    addV3i(&ci, &d, &o);
    
    [self setProperty:OriginKey value:[NSString stringWithFormat:@"%i %i %i", o.x, o.y, o.z]];
    
    if ([self angle] == nil)
        [self setProperty:AngleKey value:@"0"];
    
    int a = [[self angle] intValue];
    TVector3f direction;
    
    if (a >= 0) {
        direction.x = cos(a * M_PI / 180);
        direction.y = sin(a * M_PI / 180);
        direction.z = 0;
    } else if (a == -1) {
        direction = ZAxisPos;
    } else if (a == -2) {
        direction = ZAxisNeg;
    } else {
        return;
    }
    
    rotate90CCWV3f(&direction, theAxis, &direction);
    if (direction.z > 0.9) {
        [self setProperty:AngleKey value:@"-1"];
    } else if (direction.z < -0.9) {
        [self setProperty:AngleKey value:@"-2"];
    } else {
        if (direction.z != 0) {
            direction.z = 0;
            normalizeV3f(&direction, &direction);
        }
        
        a = roundf(acos(direction.x) * 180 / M_PI);
        if (direction.y > 0)
            a = 360 - a;
        
        [self setProperty:AngleKey value:[NSString stringWithFormat:@"%i", a]];
    }

}

- (void)rotate:(const TQuaternion *)theRotation center:(const TVector3f *)theRotationCenter {
    if (entityDefinition == nil || [entityDefinition type] != EDT_POINT)
        return;
    
    if (!valid)
        [self validate];

    TVector3f originf, offset;
    setV3f(&originf, [self origin]);
    subV3f(&center, &originf, &offset);

    subV3f(&center, theRotationCenter, &center);
    rotateQ(theRotation, &center, &center);
    addV3f(&center, theRotationCenter, &center);
    subV3f(&center, &offset, &originf);

    TVector3i newOrigin;
    roundV3f(&originf, &newOrigin);
    [self setProperty:OriginKey value:[NSString stringWithFormat:@"%i %i %i", newOrigin.x, newOrigin.y, newOrigin.z]];
    
    if ([self angle] == nil)
        [self setProperty:AngleKey value:@"0"];

    int a = [[self angle] intValue];
    TVector3f direction;
    
    if (a >= 0) {
        direction.x = cos(a * M_PI / 180);
        direction.y = sin(a * M_PI / 180);
        direction.z = 0;
    } else if (a == -1) {
        direction = ZAxisPos;
    } else if (a == -2) {
        direction = ZAxisNeg;
    } else {
        return;
    }
    
    rotateQ(theRotation, &direction, &direction);
    if (direction.z > 0.9) {
        [self setProperty:AngleKey value:@"-1"];
    } else if (direction.z < -0.9) {
        [self setProperty:AngleKey value:@"-2"];
    } else {
        if (direction.z != 0) {
            direction.z = 0;
            normalizeV3f(&direction, &direction);
        }

        a = roundf(acos(direction.x) * 180 / M_PI);
        if (direction.y > 0)
            a = 360 - a;
        
        [self setProperty:AngleKey value:[NSString stringWithFormat:@"%i", a]];
    }
}

- (void)flipAxis:(EAxis)theAxis center:(const TVector3i *)theCenter {
    if (entityDefinition == nil || [entityDefinition type] != EDT_POINT)
        return;
    
    if (!valid)
        [self validate];
    
    TVector3i o, ci, d;
    roundV3f(&center, &ci);
    subV3i([self origin], &ci, &d);
    subV3i(&ci, theCenter, &ci);
    
    switch (theAxis) {
        case A_X:
            ci.x *= -1;
            break;
        case A_Y:
            ci.y *= -1;
            break;
        default:
            ci.z *= -1;
            break;
    }
    
    addV3i(&ci, theCenter, &ci);
    addV3i(&ci, &d, &o);
    
    [self setProperty:OriginKey value:[NSString stringWithFormat:@"%i %i %i", o.x, o.y, o.z]];
    
    if ([self angle] == nil)
        [self setProperty:AngleKey value:@"0"];
    
    int a = [[self angle] intValue];
    if (a >= 0)
        a = (a + 180) % 360;
    else if (a == -1)
        a = -2;
    else if (a == -2)
        a = -1;
    [self setProperty:AngleKey value:[NSString stringWithFormat:@"%i", a]];
}

- (void)replaceProperties:(NSDictionary *)theProperties {
    NSAssert(theProperties != nil, @"properties must not be nil");
    
    NSString* newClassname = [theProperties objectForKey:ClassnameKey];
    if (![[self classname] isEqualToString:newClassname])
        [NSException raise:NSInvalidArgumentException format:@"new property set contains no or different classname"];
    
    [properties removeAllObjects];
    valid = NO;
    [angle release];
    angle = nil;
    
    NSEnumerator* keyEn = [theProperties keyEnumerator];
    NSString* key;
    while ((key = [keyEn nextObject])) {
        NSString* value = [theProperties objectForKey:key];
        [self setProperty:key value:value];
    }
}

- (void)setProperty:(NSString *)key value:(NSString *)value {
    NSAssert(key != nil, @"property key must not be nil");
    
    if ([key isEqualToString:ClassnameKey] && [self classname] != nil) {
        NSLog(@"Cannot overwrite classname property");
        return;
    } else if ([key isEqualToString:OriginKey]) {
        if (!parseV3i(value, NSMakeRange(0, [value length]), &origin)) {
            NSLog(@"Invalid origin value: '%@'", value);
            return;
        }
        valid = NO;
    } else if ([key isEqualToString:AngleKey]) {
        [angle release];
        if (value != nil)
            angle = [[NSNumber alloc] initWithInt:[value intValue]];
        else
            angle = nil;
    }
    
    NSString* oldValue = [self propertyForKey:key];
    BOOL exists = oldValue != nil;
    
    if (exists && [oldValue isEqualToString:value])
        return;
    
    [properties setObject:value forKey:key];
    valid = NO;
}

- (void)removeProperty:(NSString *)key {
    NSAssert(key != nil, @"property key must not be nil");
    
    if ([key isEqualToString:ClassnameKey]) {
        NSLog(@"Cannot delete classname property");
        return;
    } else if ([key isEqualToString:OriginKey]) {
        NSLog(@"Cannot delete origin property");
        return;
    } else if ([key isEqualToString:AngleKey]) {
        [angle release];
        angle = nil;
    }

    NSString *oldValue = [self propertyForKey:key];
    if (oldValue == nil)
        return;
    
    [properties removeObjectForKey:key];
    valid = NO;
}

- (void)setEntityDefinition:(EntityDefinition *)theEntityDefinition {
    NSAssert(theEntityDefinition != nil, @"entity definition must not be nil");
    NSAssert(entityDefinition == nil, @"can't change entity definition");
    
    entityDefinition = [theEntityDefinition retain];
    [entityDefinition incUsageCount];
}

- (void)setMap:(id <Map>)theMap {
    map = theMap;
}

- (int)filePosition {
    return filePosition;
}

- (void)setFilePosition:(int)theFilePosition {
    filePosition = theFilePosition;
}

- (NSString *)description {
    return [properties description];
}

#pragma mark -
#pragma mark @implementation Entity

- (NSNumber *)entityId {
    return entityId;
}

- (id <Map>)map {
    return map;
}

- (NSArray *)brushes {
    return brushes;
}

- (NSString *)propertyForKey:(NSString *)key {
    return (NSString *)[properties objectForKey:key];
}

- (NSDictionary *)properties {
    return properties;
}

- (BOOL)isPropertyDeletable:(NSString *)theKey {
    return ![ClassnameKey isEqualToString:theKey] && ![OriginKey isEqualToString:theKey] && ![SpawnFlagsKey isEqualToString:theKey];
}

- (BOOL)isPropertyWritable:(NSString *)theKey {
    return ![ClassnameKey isEqualToString:theKey];
}

- (NSString *)spawnFlagsString {
    if (entityDefinition == nil)
        return @"<missing entity definition>";
    
    NSString* raw = [properties objectForKey:SpawnFlagsKey];
    if (raw == nil || [raw intValue] == 0)
        return @"<none>";
    
    int mask = [raw intValue];
    NSArray* spawnFlags = [entityDefinition flagsForMask:mask];
    if ([spawnFlags count] == 0)
        return @"<none>";
    
    NSEnumerator* spawnFlagEn = [spawnFlags objectEnumerator];
    SpawnFlag* spawnFlag = [spawnFlagEn nextObject];
    
    NSMutableString* result = [[NSMutableString alloc] initWithString:[spawnFlag name]];
    while ((spawnFlag = [spawnFlagEn nextObject]))
        [result appendFormat:@" %@", [spawnFlag name]];
    
    return [result autorelease];
}

- (NSString *)classname {
    return [self propertyForKey:ClassnameKey];
}

- (EntityDefinition *)entityDefinition {
    return entityDefinition;
}

- (BOOL)isWorldspawn {
    return [[self classname] isEqualToString:WorldspawnClassname];
}

- (TBoundingBox *)maxBounds {
    if (!valid)
        [self validate];
    
    return &maxBounds;
}

- (TBoundingBox *)bounds {
    if (!valid)
        [self validate];
    
    return &bounds;
}

- (TVector3f *)center {
    if (!valid)
        [self validate];

    return &center;
}

- (TVector3i *)origin {
    if (entityDefinition == nil || [entityDefinition type] != EDT_POINT)
        [NSException raise:NSInternalInconsistencyException format:@"Entity is not a point entity (ID %@)", entityId];
    
    return &origin;
}

- (NSNumber *)angle {
    return angle;
}

- (void)pick:(const TRay *)theRay hitList:(PickingHitList *)theHitList {
    if ([self isWorldspawn])
        return;
    
    float dist = intersectBoundsWithRay([self bounds], theRay, NULL);
    if (isnan(dist))
        return;
    
    TVector3f hitPoint;
    rayPointAtDistance(theRay, dist, &hitPoint);

    PickingHit* pickingHit = [[PickingHit alloc] initWithObject:self type:HT_ENTITY hitPoint:&hitPoint distance:dist];
    [theHitList addHit:pickingHit];
    [pickingHit release];
}

- (VBOMemBlock *)boundsMemBlock {
    return boundsMemBlock;
}

- (void)setBoundsMemBlock:(VBOMemBlock *)theBoundsMemBlock {
    [boundsMemBlock free];
    boundsMemBlock = theBoundsMemBlock;
}
@end
