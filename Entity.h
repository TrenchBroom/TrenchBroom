//
//  Entitiy.h
//  TrenchBroom
//
//  Created by Kristian Duske on 15.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface Entity : NSObject {
	NSMutableSet* brushes;
	NSMutableDictionary* properties;
}

- (id)initWithKey:(NSString *)key value:(NSString *)value;

@end
