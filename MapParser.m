//
//  MapParser.m
//  TrenchBroom
//
//  Created by Kristian Duske on 18.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "MapParser.h"
#import "Map.h"
#import "MutableEntity.h"
#import "MutableBrush.h"
#import "MutableFace.h"
#import "MapTokenizer.h"
#import "MapToken.h"

static NSString* InvalidTokenException = @"InvalidTokenException";

@implementation MapParser

- (id)initWithData:(NSData *)someData entityDefinitionManager:(EntityDefinitionManager *)theDefinitionManager {
    if (self = [self init]) {
        definitionManager = [theDefinitionManager retain];
        size = [someData length];
        NSInputStream* stream = [[NSInputStream alloc] initWithData:someData];
        tokenizer = [[MapTokenizer alloc] initWithInputStream:stream];
        [stream release];
    }
    
    return self;
}

- (id)initWithData:(NSData *)someData {
    return [self initWithData:someData entityDefinitionManager:nil];
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
    p1.x = [[token data] intValue];
    
    token = [self nextToken];
    [self expect:TT_DEC actual:token];
    p1.y = [[token data] intValue];
    
    token = [self nextToken];
    [self expect:TT_DEC actual:token];
    p1.z = [[token data] intValue];
    
    token = [self nextToken];
    [self expect:TT_B_C actual:token];
    
    token = [self nextToken];
    [self expect:TT_B_O actual:token];
    
    token = [self nextToken];
    [self expect:TT_DEC actual:token];
    p2.x = [[token data] intValue];
    
    token = [self nextToken];
    [self expect:TT_DEC actual:token];
    p2.y = [[token data] intValue];
    
    token = [self nextToken];
    [self expect:TT_DEC actual:token];
    p2.z = [[token data] intValue];
    
    token = [self nextToken];
    [self expect:TT_B_C actual:token];
    
    token = [self nextToken];
    [self expect:TT_B_O actual:token];
    
    token = [self nextToken];
    [self expect:TT_DEC actual:token];
    p3.x = [[token data] intValue];
    
    token = [self nextToken];
    [self expect:TT_DEC actual:token];
    p3.y = [[token data] intValue];
    
    token = [self nextToken];
    [self expect:TT_DEC actual:token];
    p3.z = [[token data] intValue];
    
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
    
    MutableFace* face = [[MutableFace alloc] init];
    [face setPoint1:&p1 point2:&p2 point3:&p3];
    [face setTexture:texture];
    [face setXOffset:xOffset];
    [face setYOffset:yOffset];
    [face setRotation:rotation];
    [face setXScale:xScale];
    [face setYScale:yScale];

    [brush addFace:face];
    
    [face release];
    [texture release];
}

- (void)parseMap:(id<Map>)theMap withProgressIndicator:(NSProgressIndicator *)theIndicator {
    if (theIndicator != nil)
        [theIndicator setMaxValue:100];
    
    NSDate* startDate = [NSDate date];
    state = PS_DEF;
    map = [theMap retain];
    int progress = 0;
    
    MapToken* token;
    while ((token = [self nextToken])) {
        if ([token type] != TT_COM) {
            switch (state) {
                case PS_DEF:
                    [self expect:TT_CB_O actual:token];
                    state = PS_ENT;
                    entity = [[MutableEntity alloc] initWithEntityDefinitionManager:definitionManager];
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
                            brush = [[MutableBrush alloc] init];
                            break;
                        case TT_CB_C:
                            [map addEntity:entity];
                            [entity release];
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
                            [entity addBrush:brush];
                            [brush release];
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
        
        if (theIndicator != nil) {
            int newProgress = 100 * [token charsRead] / size;
            if (newProgress != progress) {
                progress = newProgress;
                [theIndicator setDoubleValue:progress];
                [[NSRunLoop currentRunLoop] runMode:NSModalPanelRunLoopMode beforeDate:[NSDate date]];
            }
        }
    }
    
    // just to make sure it reaches 100%
    if (theIndicator != nil)
        [theIndicator setDoubleValue:100];
    
    NSTimeInterval duration = [startDate timeIntervalSinceNow];
    NSLog(@"Loaded map file in %f seconds", -duration);
}

- (void)dealloc {
    [definitionManager release];
    [tokenizer release];
    [map release];
    [super dealloc];
}

@end
