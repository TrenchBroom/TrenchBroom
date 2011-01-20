//
//  MapParser.m
//  TrenchBroom
//
//  Created by Kristian Duske on 18.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "MapParser.h"
#import "Map.h"
#import "Entity.h"
#import "Brush.h"
#import "Face.h"
#import "MapTokenizer.h"
#import "MapToken.h"
#import "Vector3i.h"

NSString* const InvalidTokenException = @"InvalidTokenException";

@implementation MapParser
- (id)initWithData:(NSData *)someData {
    if (someData == nil)
        [NSException raise:NSInvalidArgumentException format:@"data must not be nil"];
    
    if (self = [self init]) {
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

- (void)parseFace {
    Vector3i* p1 = [[Vector3i alloc] init];
    Vector3i* p2 = [[Vector3i alloc] init];
    Vector3i* p3 = [[Vector3i alloc] init];
    @try {
        Token* token = [tokenizer nextToken];
        [self expect:TT_DEC actual:token];
        [p1 setX:[[token data] intValue]];

        token = [tokenizer nextToken];
        [self expect:TT_DEC actual:token];
        [p1 setY:[[token data] intValue]];

        token = [tokenizer nextToken];
        [self expect:TT_DEC actual:token];
        [p1 setZ:[[token data] intValue]];

        token = [tokenizer nextToken];
        [self expect:TT_B_C actual:token];

        token = [tokenizer nextToken];
        [self expect:TT_B_O actual:token];

        token = [tokenizer nextToken];
        [self expect:TT_DEC actual:token];
        [p2 setX:[[token data] intValue]];
        
        token = [tokenizer nextToken];
        [self expect:TT_DEC actual:token];
        [p2 setY:[[token data] intValue]];
        
        token = [tokenizer nextToken];
        [self expect:TT_DEC actual:token];
        [p2 setZ:[[token data] intValue]];

        token = [tokenizer nextToken];
        [self expect:TT_B_C actual:token];
        
        token = [tokenizer nextToken];
        [self expect:TT_B_O actual:token];

        token = [tokenizer nextToken];
        [self expect:TT_DEC actual:token];
        [p3 setX:[[token data] intValue]];
        
        token = [tokenizer nextToken];
        [self expect:TT_DEC actual:token];
        [p3 setY:[[token data] intValue]];
        
        token = [tokenizer nextToken];
        [self expect:TT_DEC actual:token];
        [p3 setZ:[[token data] intValue]];
        
        token = [tokenizer nextToken];
        [self expect:TT_B_C actual:token];
        
        token = [tokenizer nextToken];
        [self expect:TT_STR actual:token];
        NSString* texture = [token data];
        
        token = [tokenizer nextToken];
        [self expect:TT_DEC actual:token];
        int xOffset = [[token data] intValue];
        
        token = [tokenizer nextToken];
        [self expect:TT_DEC actual:token];
        int yOffset = [[token data] intValue];
        
        token = [tokenizer nextToken];
        [self expect:TT_DEC | TT_FRAC actual:token];
        float rotation = [[token data] floatValue];
        
        token = [tokenizer nextToken];
        [self expect:TT_DEC | TT_FRAC actual:token];
        float xScale = [[token data] floatValue];
        
        token = [tokenizer nextToken];
        [self expect:TT_DEC | TT_FRAC actual:token];
        float yScale = [[token data] floatValue];
        
        Face* face = [brush createFaceWithPoint1:p1 point2:p2 point3:p3 texture:texture];
        [face setXOffset:xOffset];
        [face setYOffset:yOffset];
        [face setRotation:rotation];
        [face setXScale:xScale];
        [face setYScale:yScale];
    }
    @finally {
        [p1 release];
        [p2 release];
        [p3 release];
    }
}

- (Map *)parse {
    state = PS_DEF;
    map = [[Map alloc] init];
    
    MapToken* token;
    while ((token = [tokenizer nextToken])) {
        switch (state) {
            case PS_DEF:
                [self expect:TT_CB_O actual:token];
                state = PS_ENT;
                entity = [map createEntity];
                break;
            case PS_ENT:
                switch ([token type]) {
                    case TT_STR: {
                        NSString* key = [token data];
                        token = [tokenizer nextToken];
                        [self expect:TT_STR actual:token];
                        NSString* value = [token data];
                        [entity setProperty:key value:value];
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
    
    return [map autorelease];
}

- (void)dealloc {
    [tokenizer release];
    [map release];
    [super dealloc];
}

@end
