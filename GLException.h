//
//  GLException.h
//  TrenchBroom
//
//  Created by Kristian Duske on 24.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface GLException : NSException {

}

+ (void)raise;

@end
