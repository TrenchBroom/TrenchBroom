//
//  NSFileManagerPRAdditions.h
//  TrenchBroom
//
//  Created by Kristian Duske on 06.03.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface NSFileManager (AppSupportCategory)

- (NSString *)findSystemFolderType:(int)folderType forDomain:(int)domain;
- (NSString *)findApplicationSupportFolder;
@end
