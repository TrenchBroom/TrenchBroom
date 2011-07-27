//
//  MapWriter.h
//  TrenchBroom
//
//  Created by Kristian Duske on 06.03.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@protocol Map;

@interface MapWriter : NSObject {
    @private
    id <Map> map;
    void* buffer;
}

- (id)initWithMap:(id <Map>)theMap;

- (void)writeToStream:(NSOutputStream *)theStream;
- (void)writeToFileAtPath:(NSString *)thePath;
- (void)writeToFileAtUrl:(NSURL *)theUrl;

@end
