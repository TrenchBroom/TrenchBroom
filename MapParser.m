//
//  MapParser.m
//  TrenchBroom
//
//  Created by Kristian Duske on 18.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "MapParser.h"
#import "MapDocument.h"
#import "Entity.h"
#import "Brush.h"
#import "Face.h"
#import "MapTokenizer.h"
#import "MapToken.h"
#import "Vector3i.h"

NSString* const InvalidTokenException = @"InvalidTokenException";

@implementation MapParser
- (id)init {
    if (self = [super init]) {
        p1 = [[Vector3i alloc] init];
        p2 = [[Vector3i alloc] init];
        p3 = [[Vector3i alloc] init];
    }
    
    return self;
}

- (id)initWithData:(NSData *)someData {
    if (someData == nil)
        [NSException raise:NSInvalidArgumentException format:@"data must not be nil"];
    
    if (self = [self init]) {
        size = [someData length];
        NSInputStream* stream = [[NSInputStream alloc] initWithData:someData];
        tokenizer = [[MapTokenizer alloc] initWithInputStream:stream];
        [stream release];
    }
    
    return self;
}

- (void)expect:(int)expectedType actual:(MapToken* )actualToken {
    if (!actualToken || ([actualToken type] & expectedType) == 0)
        [NSException raise:InvalidTokenException format:@"invalid token: %@, expected %@", actualToken, [MapToken typeName:expectedType]];
}

- (MapToken *)nextToken {
    MapToken* token = [tokenizer nextToken];
    while (token != nil && [token type] == TT_COM)
        token = [tokenizer nextToken];
    
    return token;
}

- (void)parseFace {
    MapToken* token = [self nextToken];
    [self expect:TT_DEC actual:token];
    [p1 setX:[[token data] intValue]];
    
    token = [self nextToken];
    [self expect:TT_DEC actual:token];
    [p1 setY:[[token data] intValue]];
    
    token = [self nextToken];
    [self expect:TT_DEC actual:token];
    [p1 setZ:[[token data] intValue]];
    
    token = [self nextToken];
    [self expect:TT_B_C actual:token];
    
    token = [self nextToken];
    [self expect:TT_B_O actual:token];
    
    token = [self nextToken];
    [self expect:TT_DEC actual:token];
    [p2 setX:[[token data] intValue]];
    
    token = [self nextToken];
    [self expect:TT_DEC actual:token];
    [p2 setY:[[token data] intValue]];
    
    token = [self nextToken];
    [self expect:TT_DEC actual:token];
    [p2 setZ:[[token data] intValue]];
    
    token = [self nextToken];
    [self expect:TT_B_C actual:token];
    
    token = [self nextToken];
    [self expect:TT_B_O actual:token];
    
    token = [self nextToken];
    [self expect:TT_DEC actual:token];
    [p3 setX:[[token data] intValue]];
    
    token = [self nextToken];
    [self expect:TT_DEC actual:token];
    [p3 setY:[[token data] intValue]];
    
    token = [self nextToken];
    [self expect:TT_DEC actual:token];
    [p3 setZ:[[token data] intValue]];
    
    token = [self nextToken];
    [self expect:TT_B_C actual:token];
    
    token = [self nextToken];
    [self expect:TT_STR actual:token];
    NSString* texture = [[token data] retain];
    
    token = [self nextToken];
    [self expect:TT_DEC actual:token];
    int xOffset = [[token data] intValue];
    
    token = [self nextToken];
    [self expect:TT_DEC actual:token];
    int yOffset = [[token data] intValue];
    
    token = [self nextToken];
    [self expect:TT_DEC | TT_FRAC actual:token];
    float rotation = [[token data] floatValue];
    
    token = [self nextToken];
    [self expect:TT_DEC | TT_FRAC actual:token];
    float xScale = [[token data] floatValue];
    
    token = [self nextToken];
    [self expect:TT_DEC | TT_FRAC actual:token];
    float yScale = [[token data] floatValue];
    
    Face* face = [brush createFaceWithPoint1:p1 point2:p2 point3:p3 texture:texture];
    [face setXOffset:xOffset];
    [face setYOffset:yOffset];
    [face setRotation:rotation];
    [face setXScale:xScale];
    [face setYScale:yScale];
    
    [texture release];
}

- (void)parseMap:(MapDocument *)theMap withProgressIndicator:(NSProgressIndicator *)theIndicator {
    [theIndicator setMaxValue:100];
    
    NSDate* startDate = [NSDate date];
    state = PS_DEF;
    map = [theMap retain];
    [map setPostNotifications:NO];
    
    MapToken* token;
    while ((token = [self nextToken])) {
        if ([token type] != TT_COM) {
            switch (state) {
                case PS_DEF:
                    [self expect:TT_CB_O actual:token];
                    state = PS_ENT;
                    entity = [map createEntity];
                    break;
                case PS_ENT:
                    switch ([token type]) {
                        case TT_STR: {
                            NSString* key = [[token data] retain];
                            token = [self nextToken];
                            [self expect:TT_STR actual:token];
                            NSString* value = [[token data] retain];
                            [entity setProperty:key value:value];
                            [key release];
                            [value release];
                            break;
                        }
                        case TT_CB_O:
                            state = PS_BRUSH;
                            brush = [entity createBrush];
                            break;
                        case TT_CB_C:
                            state = PS_DEF;
                            entity = nil;
                            break;
                        default:
                            break;
                    }
                    break;
                case PS_BRUSH:
                    switch ([token type]) {
                        case TT_B_O:
                            [self parseFace];
                            break;
                        case TT_CB_C:
                            state = PS_ENT;
                            brush = nil;
                            break;
                        default:
                            [self expect:TT_B_O | TT_CB_C actual:token];
                            break;
                    }
                    
                    break;
                default:
                    break;
            }
        }
        
        [theIndicator setDoubleValue:100 * [token charsRead] / (double)size];
        [[NSRunLoop currentRunLoop] runMode:NSModalPanelRunLoopMode beforeDate:[NSDate date]];
    }
    
    // just to make sure it reaches 100%
    [theIndicator setDoubleValue:100];
    
    NSTimeInterval duration = [startDate timeIntervalSinceNow];
    NSLog(@"Loaded map file in %f seconds", -duration);

    [map setPostNotifications:YES];
}

- (void)dealloc {
    [p1 release];
    [p2 release];
    [p3 release];
    
    [tokenizer release];
    [map release];
    [super dealloc];
}

@end
