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
#import "EntityDefinitionManager.h"

static NSString* InvalidTokenException = @"InvalidTokenException";

@implementation MapParser

- (id)initWithData:(NSData *)someData {
    if ((self = [self init])) {
        size = [someData length];
        NSInputStream* stream = [[NSInputStream alloc] initWithData:someData];
        tokenizer = [[MapTokenizer alloc] initWithInputStream:stream];
        [stream release];
        tokens = [[NSMutableArray alloc] init];
    }
    
    return self;
}

- (void)expect:(int)expectedType actual:(MapToken* )actualToken {
    if (!actualToken || ([actualToken type] & expectedType) == 0)
        [NSException raise:InvalidTokenException format:@"invalid token: %@, expected %@", actualToken, [MapToken typeName:expectedType]];
}

- (MapToken *)nextToken {
    MapToken* token;
    if ([tokens count] > 0) {
        token = [tokens lastObject];
        [token retain];
        [tokens removeLastObject];
        [token autorelease];
    } else {
        token = [tokenizer nextToken];
        while (token != nil && [token type] == TT_COM)
            token = [tokenizer nextToken];
    }
    
    return token;
}

- (void)pushToken:(MapToken *)theToken {
    [tokens addObject:theToken];
}

- (MutableFace *)parseFace:(int)filePosition {
    TVector3i p1, p2, p3;
    
    MapToken* token = [self nextToken];
    [self expect:TT_DEC | TT_FRAC actual:token];
    p1.x = [[token data] intValue];
    
    token = [self nextToken];
    [self expect:TT_DEC | TT_FRAC actual:token];
    p1.y = [[token data] intValue];
    
    token = [self nextToken];
    [self expect:TT_DEC | TT_FRAC actual:token];
    p1.z = [[token data] intValue];
    
    token = [self nextToken];
    [self expect:TT_B_C actual:token];
    
    token = [self nextToken];
    [self expect:TT_B_O actual:token];
    
    token = [self nextToken];
    [self expect:TT_DEC | TT_FRAC actual:token];
    p2.x = [[token data] intValue];
    
    token = [self nextToken];
    [self expect:TT_DEC | TT_FRAC actual:token];
    p2.y = [[token data] intValue];
    
    token = [self nextToken];
    [self expect:TT_DEC | TT_FRAC actual:token];
    p2.z = [[token data] intValue];
    
    token = [self nextToken];
    [self expect:TT_B_C actual:token];
    
    token = [self nextToken];
    [self expect:TT_B_O actual:token];
    
    token = [self nextToken];
    [self expect:TT_DEC | TT_FRAC actual:token];
    p3.x = [[token data] intValue];
    
    token = [self nextToken];
    [self expect:TT_DEC | TT_FRAC actual:token];
    p3.y = [[token data] intValue];
    
    token = [self nextToken];
    [self expect:TT_DEC | TT_FRAC actual:token];
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
    [face setFilePosition:filePosition];
    
    [texture release];
    return [face autorelease];
}

- (void)parseMap:(id<Map>)theMap withProgressIndicator:(NSProgressIndicator *)theIndicator {
    if (theIndicator != nil)
        [theIndicator setMaxValue:100];
    
    NSDate* startDate = [NSDate date];
    EParserState state = PS_DEF;
    map = theMap;
    int progress = 0;
    
    MutableEntity* entity;
    MutableBrush* brush;
    
    MapToken* token;
    while ((token = [self nextToken])) {
        if ([token type] != TT_COM) {
            switch (state) {
                case PS_DEF:
                    [self expect:TT_CB_O actual:token];
                    state = PS_ENT;
                    entity = [[MutableEntity alloc] init];
                    [entity setFilePosition:[token line]];
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
                        case TT_CB_O: {
                            state = PS_BRUSH;
                            brush = [[MutableBrush alloc] initWithWorldBounds:[map worldBounds]];
                            [brush setFilePosition:[token line]];
                            break;
                        }
                        case TT_CB_C: {
                            [map addEntity:entity];
                            [entity release];
                            entity = nil;
                            state = PS_DEF;
                            break;
                        }
                        default:
                            break;
                    }
                    break;
                case PS_BRUSH:
                    switch ([token type]) {
                        case TT_B_O: {
                            MutableFace* face = [self parseFace:[token line]];
                            [brush addFace:face];
                            break;
                        }
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

- (EClipboardContents)parseClipboard:(NSMutableArray *)result worldBounds:(TBoundingBox *)theWorldBounds {
    EClipboardContents contents = CC_UNDEFINED;
    MapToken* token = [[MapToken alloc] initWithToken:[self nextToken]];
    [self expect:TT_B_O | TT_CB_O actual:token];
    if ([token type] == TT_CB_O) {
        MapToken* nextToken = [[MapToken alloc] initWithToken:[self nextToken]];
        if ([nextToken type] == TT_CB_O || TT_STR) {
            contents = CC_ENT;
        } else {
            contents = CC_BRUSH;
        }
        [self pushToken:nextToken];
        [nextToken release];
    } else {
        contents = CC_FACE;
    }
    [self pushToken:token];
    [token release];
    
    EParserState state = PS_DEF;
    MutableEntity* entity;
    MutableBrush* brush;
    
    while ((token = [self nextToken])) {
        if ([token type] != TT_COM) {
            switch (state) {
                case PS_DEF:
                    switch (contents) {
                        case CC_ENT:
                            [self expect:TT_CB_O actual:token];
                            state = PS_ENT;
                            entity = [[MutableEntity alloc] init];
                            [entity setFilePosition:[token line]];
                            break;
                        case CC_BRUSH:
                            [self expect:TT_CB_O actual:token];
                            state = PS_BRUSH;
                            entity = [[MutableEntity alloc] init];
                            [entity setFilePosition:[token line]];
                            break;
                        case CC_FACE:
                            [self expect:TT_B_O actual:token];
                            [result addObject:[self parseFace:[token line]]];
                            break;
                        default:
                            break;
                    }
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
                        case TT_CB_O: {
                            state = PS_BRUSH;
                            brush = [[MutableBrush alloc] initWithWorldBounds:theWorldBounds];
                            [brush setFilePosition:[token line]];
                            break;
                        }
                        case TT_CB_C: {
                            [result addObject:entity];
                            [entity release];
                            entity = nil;
                            state = PS_DEF;
                            break;
                        }
                        default:
                            break;
                    }
                    break;
                case PS_BRUSH:
                    switch ([token type]) {
                        case TT_B_O:
                            [self parseFace:[token line]];
                            break;
                        case TT_CB_C:
                            if (contents == CC_BRUSH)
                                [result addObject:brush];
                            else
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
    }
    
    return contents;
}

- (void)dealloc {
    [tokenizer release];
    [tokens release];
    [super dealloc];
}

@end
