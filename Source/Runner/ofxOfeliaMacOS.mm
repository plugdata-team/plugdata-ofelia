#import <Cocoa/Cocoa.h>

void toggleDockIconVisibility(bool showIcon) {
    id app = [NSApplication sharedApplication];

    // Show or hide the dock icon based on the bool argument
    if (showIcon) {
        [app setActivationPolicy:NSApplicationActivationPolicyRegular];
    } else {
        [app setActivationPolicy:NSApplicationActivationPolicyAccessory];
    }
}
