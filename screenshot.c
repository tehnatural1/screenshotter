#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include "IOSurface.h"

#include <IOKit/IOKitLib.h>
#include <CoreGraphics/CGImage.h>
#include <CoreFoundation/CoreFoundation.h>

#define YES 1

#define PORT_DEFAULT (0) // kIOMainPortDefault  kIOMasterPortDefault

#define kIOSurfaceLockReadOnly     0x00000001
#define kIOSurfaceLockAvoidSync    0x00000002

typedef void *IOMobileFramebufferRef;

typedef io_connect_t IOMobileFramebufferConnection;
typedef kern_return_t IOMobileFramebufferReturn;
typedef io_service_t IOMobileFramebufferService;
typedef void * CoreSurfaceBufferRef;
typedef IOReturn IOSurfaceAcceleratorReturn;
typedef struct __IOSurfaceAccelerator *IOSurfaceAcceleratorRef;

IOMobileFramebufferReturn
    (*IOMobileFramebufferOpen)
    (
        IOMobileFramebufferService service,
        task_port_t owningTask,
        unsigned int type,
        IOMobileFramebufferConnection * connection
    );

IOMobileFramebufferReturn
    (*IOMobileFramebufferGetLayerDefaultSurface)
    (
        IOMobileFramebufferConnection connection,
        int surface,
        CoreSurfaceBufferRef *ptr
    );

extern
int
    IOSurfaceLock
    (
        IOSurfaceRef surface,
        uint32_t options,
        uint32_t *seed
    );

extern
int
    IOSurfaceUnlock
    (
        IOSurfaceRef surface,
        uint32_t options,
        uint32_t *seed
    );

extern
int
    IOSurfaceAcceleratorCreate
    (
        CFAllocatorRef allocator,
        int type,
        IOSurfaceAcceleratorRef *accel
    );

extern
unsigned int
    IOSurfaceAcceleratorTransferSurface
    (
        IOSurfaceAcceleratorRef accelerator,
        IOSurfaceRef dest,
        IOSurfaceRef src,
        void*,
        void*,
        void*,
        void*
    );

static
void
    takeScreenShot
    (

    )
{
    IOMobileFramebufferConnection connect;
    kern_return_t result;

    IOSurfaceRef screenSurface = NULL;

    printf( "Calling IOServiceGetmatchingService()\n" );

    io_service_t framebufferService = IOServiceGetMatchingService( PORT_DEFAULT, IOServiceMatching( "AppleH1CLCD" ) );
    if( !framebufferService )
        framebufferService = IOServiceGetMatchingService( PORT_DEFAULT, IOServiceMatching( "AppleM2CLCD" ) );
    if( !framebufferService )
        framebufferService = IOServiceGetMatchingService( PORT_DEFAULT, IOServiceMatching( "AppleCLCD" ) );
    
    if( !framebufferService )
        printf( "Failed to get a matching FrameBuffer Service!\n" );


    printf( "calling IOMobileFrameBufferOpen()\n" );
    if( KERN_SUCCESS != ( result = IOMobileFramebufferOpen( framebufferService, mach_task_self(), 0, &connect ) ) )
    {
        printf( "IOMobileFrameBufferOpen() Failed\n" );
    }

    printf( "calling IOMobileFrameBufferGetLayerDefaultSurface()\n" );
    if( KERN_SUCCESS != ( result = IOMobileFramebufferGetLayerDefaultSurface(connect, 0, &screenSurface) ) )
    {
        printf( "IOMobileFramebufferGetLayerDefaultSurface() Failed\n" );
    }

    uint32_t aseed;
    
    printf( "calling IOSurfaceLock()\n" );
    if( KERN_SUCCESS != ( result = IOSurfaceLock(screenSurface, kIOSurfaceLockReadOnly, &aseed) ) )
    {
        printf( "IOSurfaceLock() Failed\n" );
    }
    
    printf( "Getting Width & Height\n" );
    uint32_t width = IOSurfaceGetWidth(screenSurface);
    uint32_t height = IOSurfaceGetHeight(screenSurface);
    printf( "Width: %u, height: %u\n", width, height );

    CFMutableDictionaryRef dict;
    int pitch = width*4, size = width*height*4;
    int bPE=4;
    char pixelFormat[4] = {'A','R','G','B'};
    dict = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFDictionarySetValue(dict, kIOSurfaceIsGlobal, kCFBooleanTrue);
    CFDictionarySetValue(dict, kIOSurfaceBytesPerRow, CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &pitch));
    CFDictionarySetValue(dict, kIOSurfaceBytesPerElement, CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &bPE));
    CFDictionarySetValue(dict, kIOSurfaceWidth, CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &width));
    CFDictionarySetValue(dict, kIOSurfaceHeight, CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &height));
    CFDictionarySetValue(dict, kIOSurfacePixelFormat, CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, pixelFormat));
    CFDictionarySetValue(dict, kIOSurfaceAllocSize, CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &size));

    IOSurfaceRef destSurf = NULL;
    
    printf( "calling IOSurfaceCreate()\n" );
    if( NULL == ( destSurf = IOSurfaceCreate(dict) ) )
    {
        printf( "IOSurfaceCreate() Failed\n" );
    }
    else
    {

    } // IOSurfaceCreate()

    void* outAcc;
    if( KERN_SUCCESS != ( result = IOSurfaceAcceleratorCreate(NULL, 0, &outAcc) ) )
    {
        printf( "IOSurfaceAcceleratorCreate() Failed\n" );
    }

    if( KERN_SUCCESS != ( result = IOSurfaceAcceleratorTransferSurface(outAcc, screenSurface, destSurf, dict, NULL, NULL, NULL) ) )
    {
        printf( "IOSurfaceAcceleratorTransferSurface() Failed\n" );
    }

    if( KERN_SUCCESS != ( result = IOSurfaceUnlock(screenSurface, kIOSurfaceLockReadOnly, &aseed) ) )
    {
        printf( "IOSurfaceUnlock() Failed\n" );
    }

    CFRelease(outAcc);

    CGDataProviderRef provider = NULL;
    
    if( NULL == ( provider = CGDataProviderCreateWithData(NULL, IOSurfaceGetBaseAddress(destSurf), (width * height * 4), NULL) ) )
    {
        printf( "CGDataProviderCreateWithData() Failed\n" );
    }
    
    CGImageRef cgImage= NULL;
    
    if( NULL == ( cgImage = CGImageCreate(width, height, 8,
                                    8*4, IOSurfaceGetBytesPerRow(destSurf),
                                    CGColorSpaceCreateDeviceRGB(), kCGImageAlphaNoneSkipFirst |kCGBitmapByteOrder32Little,
                                    provider, NULL,
                                    YES, kCGRenderingIntentDefault) ) )
    {
        printf( "CGImageCreate() Failed\n" );
    }

    // UIImageWriteToSavedPhotosAlbum(cgImage, nil, nil, nil);
}


int main()
{
    printf( "now\n" );

    void* iohandle = dlopen( "/System/Library/PrivateFrameworks/IOMobileFramebuffer.framework/IOMobileFramebuffer", RTLD_LAZY );

    IOMobileFramebufferOpen = dlsym( iohandle, "IOMobileFramebufferOpen" );
    IOMobileFramebufferGetLayerDefaultSurface = dlsym( iohandle, "IOMobileFramebufferGetLayerDefaultSurface" );

    takeScreenShot();
}
