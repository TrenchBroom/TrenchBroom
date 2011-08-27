//
//  PreferencesWindowController.m
//  TrenchBroom
//
//  Created by Kristian Duske on 28.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "PreferencesController.h"
#import "QuakePathFormatter.h"
#import "MapWindowController.h"
#import "PreferencesManager.h"

static NSString* const GeneralToolbarItemIdentifier    = @"GeneralToolbarItem";
static NSString* const CompilerToolbarItemIdentifier   = @"CompilerToolbarItem";

static PreferencesController* sharedInstance = nil;

@interface PreferencesController (private)

- (void)updateExecutableList;
- (void)preferencesDidChange:(NSNotification *)notification;
- (void)activateToolbarItemWithId:(NSString *)toolbarItemIdentifier view:(NSView *)view animate:(BOOL)animate;

@end

@implementation PreferencesController (private)

- (void)updateExecutableList {
    NSFileManager* fileManager = [NSFileManager defaultManager];
    NSWorkspace* workspace = [NSWorkspace sharedWorkspace];
    
    PreferencesManager* preferences = [PreferencesManager sharedManager];
    NSString* quakePath = [preferences quakePath];
    BOOL directory;
    BOOL exists = [fileManager fileExistsAtPath:quakePath isDirectory:&directory];
    
    [quakeExecutablePopUp removeAllItems];
    NSMenu* menu = [quakeExecutablePopUp menu];
    if (exists && directory) {
        NSArray* contents = [fileManager contentsOfDirectoryAtPath:quakePath error:NULL];
        NSEnumerator* filenameEn = [contents objectEnumerator];
        NSString* filename;
        while ((filename = [filenameEn nextObject])) {
            NSString* filePath = [NSString pathWithComponents:[NSArray arrayWithObjects:quakePath, filename, nil]];
            [fileManager fileExistsAtPath:filePath isDirectory:&directory];
            if (directory && [@"app" isEqualToString:[filePath pathExtension]] && [workspace isFilePackageAtPath:filePath]) {
                NSString* appname = [filename stringByDeletingPathExtension];
                NSMenuItem* menuItem = [[NSMenuItem alloc] initWithTitle:appname action:NULL keyEquivalent:@""];
                [menuItem setImage:[workspace iconForFile:filePath]];
                [menu addItem:menuItem];
                [menuItem release];
            }
        }
    }
    
    NSString* executable = [preferences quakeExecutable];
    if (executable != nil)
        [quakeExecutablePopUp selectItemWithTitle:[executable stringByDeletingPathExtension]];
    [quakeExecutablePopUp synchronizeTitleAndSelectedItem];
}

- (void)preferencesDidChange:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    if (DefaultsQuakePath != [userInfo objectForKey:DefaultsKey])
        return;

    [self updateExecutableList];
}

- (void)activateToolbarItemWithId:(NSString *)toolbarItemIdentifier view:(NSView *)newView animate:(BOOL)animate {
    NSView* contentView = [[self window] contentView];
    NSView* oldView = [[contentView subviews] count] > 0 ? [[contentView subviews] objectAtIndex:0] : nil;
    
    NSRect oldWindowFrame = [[self window] frame];
    NSRect newViewFrame = [newView frame];
    
    float rest = NSHeight(oldWindowFrame) - NSHeight([contentView frame]);
    float heightDiff = NSHeight(newViewFrame) - NSHeight([contentView frame]);
    
    NSRect newFrame = NSMakeRect(NSMinX(oldWindowFrame), NSMinY(oldWindowFrame) - heightDiff, NSWidth(newViewFrame), NSHeight(newViewFrame) + rest);
    [[self window] setFrame:newFrame display:YES animate:animate];

    newViewFrame.origin.y = 0;
    [newView setFrame:newViewFrame];

    [contentView addSubview:newView];
    [oldView removeFromSuperview];
    [contentView setNeedsDisplay:YES];
    
    [toolbar setSelectedItemIdentifier:toolbarItemIdentifier];
}

@end

@implementation PreferencesController

+ (PreferencesController *)sharedPreferences {
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

- (oneway void)release {
    //do nothing
}

- (id)autorelease {
    return self;
}

- (id)init {
    if ((self = [super init])) {
        toolbarItemToViewMap = [[NSMutableDictionary alloc] init];
    }
    
    
    return self;
}

- (void)dealloc {
    [toolbarItemToViewMap release];
    [super dealloc];
}

- (NSString *)windowNibName {
    return @"PreferencesWindow";
}

- (void)windowDidLoad {
    [toolbarItemToViewMap setObject:generalView forKey:GeneralToolbarItemIdentifier];
    [toolbarItemToViewMap setObject:compilerView forKey:CompilerToolbarItemIdentifier];
    
    PreferencesManager* preferences = [PreferencesManager sharedManager];
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center addObserver:self selector:@selector(preferencesDidChange:) name:DefaultsDidChange object:preferences];
    [self updateExecutableList];
    [self activateToolbarItemWithId:GeneralToolbarItemIdentifier view:generalView animate:NO];
}

- (IBAction)chooseQuakePath:(id)sender {
    NSOpenPanel* openPanel = [NSOpenPanel openPanel];
    [openPanel setCanChooseFiles:NO];
    [openPanel setCanChooseDirectories:YES];
    [openPanel setAllowsMultipleSelection:NO];
    [openPanel setAllowsOtherFileTypes:NO];
    [openPanel setTitle:@"Choose Quake Path"];
    [openPanel setNameFieldLabel:@"Quake Path"];
    [openPanel setCanCreateDirectories:NO];
    
    if ([openPanel runModal] == NSFileHandlingPanelOKButton) {
        NSEnumerator* urlEn = [[openPanel URLs] objectEnumerator];
        NSURL* url;
        while ((url = [urlEn nextObject])) {
            NSString* quakePath = [url path];
            if (quakePath != nil) {
                PreferencesManager* preferences = [PreferencesManager sharedManager];
                [preferences setQuakePath:quakePath];
            }
        }
    }
}

- (PreferencesManager *)preferences {
    return [PreferencesManager sharedManager];
}

- (IBAction)generalToolbarItemSelected:(id)sender {
    [self activateToolbarItemWithId:GeneralToolbarItemIdentifier view:generalView animate:YES];
}

- (IBAction)compilerToolbarItemSelected:(id)sender {
    [self activateToolbarItemWithId:CompilerToolbarItemIdentifier view:compilerView animate:YES];
}

@end
