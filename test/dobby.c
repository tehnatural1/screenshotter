
#include <AudioToolbox/AudioUnit.h>
#include <AudioToolbox/AUComponent.h>
#include "dobby.h"

OSStatus (*AudioUnitProcess_orig) ( AudioUnit                       inUnit, 
			                        AudioUnitRenderActionFlags*     __nullable ioActionFlags, 
									const AudioTimeStamp*           inTimeStamp, 
									UInt32                          inNumberFrames, 
									AudioBufferList*                ioData );

OSStatus
    AudioUnitProcess_hook
    (
        AudioUnit                       inUnit, 
        AudioUnitRenderActionFlags*     __nullable ioActionFlags, 
        const AudioTimeStamp*           inTimeStamp, 
        UInt32                          inNumberFrames, 
        AudioBufferList*                ioData
    )
{
    OSStatus status = 0;

    if( NULL == AudioUnitProcess_orig )
    {
        printf( "Not setup!\n" );
    }
    else {
        printf( "Calling original\n" );
        status = AudioUnitProcess_orig( inUnit,
                                        ioActionFlags,
                                        inTimeStamp,
                                        inNumberFrames,
                                        ioData );
        printf( "Status: %X\n", status );
    }

    return status;
}

int main()
{
    // /System/Library/Frameworks/AudioToolbox.framework/Versions/A/AudioToolbox
    // /System/Library/Frameworks/AudioUnit.framework/Versions/A/AudioUnit
    // /System/Library/PrivateFrameworks/AudioToolboxCore.framework/Versions/A/AudioToolboxCore

    int dobby_status = 0;
    void* AudioUnitProcess_addr = NULL;

    // Scan dylib shared cache and attempt to resolve memory address
    if( NULL == ( AudioUnitProcess_addr = DobbySymbolResolver( NULL, "AudioUnitProcess" ) ) )
    {
        printf( "Did not find address for: AudioUnitProcess\n" );
    }
    else {
        // Attempt to hook AudioUnitProcess
        if( 0 != ( dobby_status = DobbyHook( AudioUnitProcess_addr,
                                            AudioUnitProcess_hook,
                                            (void**)&AudioUnitProcess_orig ) ) )
        {
            printf( "hook FAILURE\n" );
        }
        else {
            printf( "Hook SUCCESS\n" );
            OSStatus x = AudioUnitProcess( NULL, 0, 0, 0, NULL );
            printf( "STATUS: %X\n", x );

            // Restore patched code
            if( 0 != ( dobby_status = DobbyDestroy( AudioUnitProcess_addr ) ) )
            {
                printf( "DobbyDestroy() Failed! Original method is not restored.\n" );
            }
            else {
                printf( "Successfully restored patched code\n" );
            }
        }
    }
}
