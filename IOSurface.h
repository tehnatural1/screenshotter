#include <CoreFoundation/CoreFoundation.h>


#if __has_feature(objc_class_property)
#define IOSFC_SWIFT_NAME(name) __attribute__((swift_name(#name)))
#else
#define IOSFC_SWIFT_NAME(name)
#endif

typedef CF_OPTIONS(uint32_t, IOSurfaceLockOptions)
{
    // If you are not going to modify the data while you hold the lock, you should set this flag to avoid invalidating
    // any existing caches of the buffer contents.  This flag should be passed both to the lock and unlock functions.
    // Non-symmetrical usage of this flag will result in undefined behavior.
    kIOSurfaceLockReadOnly  =   0x00000001,
    
    // If you want to detect/avoid a potentially expensive paging operation (such as readback from a GPU to system memory)
    // when you lock the buffer, you may include this flag.   If locking the buffer requires a readback, the lock will
    // fail with an error return of kIOReturnCannotLock.
    kIOSurfaceLockAvoidSync =   0x00000002,
};

typedef struct CF_BRIDGED_TYPE(id) CF_BRIDGED_MUTABLE_TYPE(IOSurface) __IOSurface *IOSurfaceRef IOSFC_SWIFT_NAME(IOSurfaceRef);

/* kIOSurfaceAllocSize    - CFNumber of the total allocation size of the buffer including all planes.    
   Defaults to BufferHeight * BytesPerRow if not specified.   Must be specified for
   dimensionless buffers. */
extern const CFStringRef kIOSurfaceAllocSize                                API_AVAILABLE(macos(10.6), ios(11.0), watchos(4.0), tvos(11.0));

/* kIOSurfaceWidth  - CFNumber for the width of the IOSurface buffer in pixels.   Required for planar IOSurfaces. */
extern const CFStringRef kIOSurfaceWidth                                    API_AVAILABLE(macos(10.6), ios(11.0), watchos(4.0), tvos(11.0));

/* kIOSurfaceHeight - CFNumber for the height of the IOSurface buffer in pixels.  Required for planar IOSurfaces. */
extern const CFStringRef kIOSurfaceHeight                                   API_AVAILABLE(macos(10.6), ios(11.0), watchos(4.0), tvos(11.0));

/* kIOSurfaceBytesPerRow - CFNumber for the bytes per row of the buffer.   If not specified, IOSurface will first calculate
   the number full elements required on each row (by rounding up), multiplied by the bytes per element
   for this buffer.   That value will then be appropriately aligned. */
extern const CFStringRef kIOSurfaceBytesPerRow                              API_AVAILABLE(macos(10.6), ios(11.0), watchos(4.0), tvos(11.0));

/* Optional properties for non-planar two dimensional images */
 
/* kIOSurfaceBytesPerElement - CFNumber for the total number of bytes in an element.  Default to 1. */
extern const CFStringRef kIOSurfaceBytesPerElement                          API_AVAILABLE(macos(10.6), ios(11.0), watchos(4.0), tvos(11.0));

/* kIOSurfaceElementWidth   - CFNumber for how many pixels wide each element is.   Defaults to 1. */ 
extern const CFStringRef kIOSurfaceElementWidth                             API_AVAILABLE(macos(10.6), ios(11.0), watchos(4.0), tvos(11.0));

/* kIOSurfaceElementHeight  - CFNumber for how many pixels high each element is.   Defaults to 1. */ 
extern const CFStringRef kIOSurfaceElementHeight                            API_AVAILABLE(macos(10.6), ios(11.0), watchos(4.0), tvos(11.0));

/* kIOSurfaceOffset - CFNumber for the starting offset into the buffer.  Defaults to 0. */
extern const CFStringRef kIOSurfaceOffset                                   API_AVAILABLE(macos(10.6), ios(11.0), watchos(4.0), tvos(11.0));

/* Properties for planar surface buffers */

/* kIOSurfacePlaneInfo    - CFArray describing each image plane in the buffer as a CFDictionary.   The CFArray must have at least one entry. */
extern const CFStringRef kIOSurfacePlaneInfo                                API_AVAILABLE(macos(10.6), ios(11.0), watchos(4.0), tvos(11.0));

/* kIOSurfacePlaneWidth  - CFNumber for the width of this plane in pixels.  Required for image planes. */
extern const CFStringRef kIOSurfacePlaneWidth                               API_AVAILABLE(macos(10.6), ios(11.0), watchos(4.0), tvos(11.0));

/* kIOSurfacePlaneHeight  - CFNumber for the height of this plane in pixels.  Required for image planes. */
extern const CFStringRef kIOSurfacePlaneHeight                              API_AVAILABLE(macos(10.6), ios(11.0), watchos(4.0), tvos(11.0));

/* kIOSurfacePlaneBytesPerRow    - CFNumber for the bytes per row of this plane.  If not specified, IOSurface will first calculate
   the number full elements required on each row (by rounding up), multiplied by the bytes per element
   for this plane.   That value will then be appropriately aligned. */
extern const CFStringRef kIOSurfacePlaneBytesPerRow                         API_AVAILABLE(macos(10.6), ios(11.0), watchos(4.0), tvos(11.0));

/* kIOSurfacePlaneOffset  - CFNumber for the offset into the buffer for this plane.  If not specified then IOSurface
   will lay out each plane sequentially based on the previous plane's allocation size. */
extern const CFStringRef kIOSurfacePlaneOffset                              API_AVAILABLE(macos(10.6), ios(11.0), watchos(4.0), tvos(11.0));

/* kIOSurfacePlaneSize    - CFNumber for the total data size of this plane.  Defaults to plane height * plane bytes per row if not specified. */
extern const CFStringRef kIOSurfacePlaneSize                                API_AVAILABLE(macos(10.6), ios(11.0), watchos(4.0), tvos(11.0));

/* Optional properties for planar surface buffers */

/* kIOSurfacePlaneBase    - CFNumber for the base offset into the buffer for this plane. Optional, defaults to the plane offset */
extern const CFStringRef kIOSurfacePlaneBase                                API_AVAILABLE(macos(10.6), ios(11.0), watchos(4.0), tvos(11.0));

/* kIOSurfacePlaneBitsPerElement    - CFNumber for the bits per element of this plane.  Optional, default is 1. 
   For use in cases where kIOSurfacePlaneBytesPerElement doesn't allow sufficient precision. */
extern const CFStringRef kIOSurfacePlaneBitsPerElement                      API_AVAILABLE(macos(10.13), ios(11.0), watchos(4.0), tvos(11.0));

/* kIOSurfacePlaneBytesPerElement    - CFNumber for the bytes per element of this plane.  Optional, default is 1. */
extern const CFStringRef kIOSurfacePlaneBytesPerElement                     API_AVAILABLE(macos(10.6), ios(11.0), watchos(4.0), tvos(11.0));

/* kIOSurfacePlaneElementWidth    - CFNumber for the element width of this plane.  Optional, default is 1. */
extern const CFStringRef kIOSurfacePlaneElementWidth                        API_AVAILABLE(macos(10.6), ios(11.0), watchos(4.0), tvos(11.0));

/* kIOSurfacePlaneElementHeight   - CFNumber for the element height of this plane.  Optional, default is 1. */
extern const CFStringRef kIOSurfacePlaneElementHeight                       API_AVAILABLE(macos(10.6), ios(11.0), watchos(4.0), tvos(11.0));

/* Optional properties global to the entire IOSurface */

/* kIOSurfaceCacheMode  - CFNumber for the CPU cache mode to be used for the allocation.  Default is kIOMapDefaultCache. */
extern const CFStringRef kIOSurfaceCacheMode                                API_AVAILABLE(macos(10.6), ios(11.0), watchos(4.0), tvos(11.0));

/* kIOSurfaceIsGlobal   - CFBoolean     If true, the IOSurface may be looked up by any task in the system by its ID. */
extern const CFStringRef kIOSurfaceIsGlobal                                 API_DEPRECATED("Global surfaces are insecure",macos(10.6,10.11), ios(11.0,11.0), watchos(4.0,4.0), tvos(11.0,11.0));

/* kIOSurfacePixelFormat - CFNumber     A 32-bit unsigned integer that stores the traditional Mac OS X buffer format  */
extern const CFStringRef kIOSurfacePixelFormat                              API_AVAILABLE(macos(10.6), ios(11.0), watchos(4.0), tvos(11.0));

/* kIOSurfacePixelSizeCastingAllowed - If false the creator promises that there will be no pixel size casting when used on the GPU.  Default is true.  */
extern const CFStringRef kIOSurfacePixelSizeCastingAllowed                  API_AVAILABLE(macos(10.12), ios(11.0), watchos(4.0), tvos(11.0));

/* kIOSurfacePlaneComponentBitDepths   - CFArray[CFNumber] for bit depth of each component in this plane. */
extern const CFStringRef kIOSurfacePlaneComponentBitDepths                  API_AVAILABLE(macos(10.13), ios(11.0), watchos(4.0), tvos(11.0));

/* kIOSurfacePlaneComponentBitOffsets   - CFArray[CFNumber] for bit offset of each component in this plane, (low bit zero, high bit 7). For example 'BGRA' would be {0, 8, 16, 24} */
extern const CFStringRef kIOSurfacePlaneComponentBitOffsets                 API_AVAILABLE(macos(10.13), ios(11.0), watchos(4.0), tvos(11.0));

// This key may be used to specify a name for the IOSurface either at creation time, or may be used with IOSurfaceSetValue() to
// set it dynamically.  If not provided, the name will be set based on the binary containing the address calling into IOSurface.framework
// to create it.
extern const CFStringRef kIOSurfaceName                                     API_AVAILABLE(macos(11.0), ios(14.0), watchos(7.0), tvos(14.0));

/* Create a brand new IOSurface object */
IOSurfaceRef _Nullable IOSurfaceCreate(CFDictionaryRef properties)
    API_AVAILABLE(macos(10.6), ios(11.0), watchos(4.0), tvos(11.0));

/* "Lock" or "Unlock" a IOSurface for reading or writing.

    The term "lock" is used loosely in this context, and is simply used along with the
    "unlock" information to put a bound on CPU access to the raw IOSurface data.
    
    If the seed parameter is non-NULL, IOSurfaceLock() will store the buffer's
    internal modification seed value at the time you made the lock call.   You can compare
    this value to a value returned previously to determine of the contents of the buffer
    has been changed since the last lock.
    
    In the case of IOSurfaceUnlock(), the seed value returned will be the internal
    seed value at the time of the unlock.  If you locked the buffer for writing, this value
    will be incremented as the unlock is performed and the new value will be returned.
    
    See the kIOSurfaceLock enums for more information.
    
    Note: Locking and unlocking a IOSurface is not a particularly cheap operation,
    so care should be taken to avoid the calls whenever possible.   The seed values are 
    particularly useful for keeping a cache of the buffer contents.
*/
kern_return_t IOSurfaceLock(IOSurfaceRef buffer, IOSurfaceLockOptions options, uint32_t * _Nullable seed)
    API_AVAILABLE(macos(10.6), ios(11.0), watchos(4.0), tvos(11.0));
kern_return_t IOSurfaceUnlock(IOSurfaceRef buffer, IOSurfaceLockOptions options, uint32_t * _Nullable seed)
    API_AVAILABLE(macos(10.6), ios(11.0), watchos(4.0), tvos(11.0));

size_t IOSurfaceGetWidth(IOSurfaceRef buffer)
    API_AVAILABLE(macos(10.6), ios(11.0), watchos(4.0), tvos(11.0));

size_t IOSurfaceGetHeight(IOSurfaceRef buffer)
    API_AVAILABLE(macos(10.6), ios(11.0), watchos(4.0), tvos(11.0));

size_t IOSurfaceGetBytesPerRow(IOSurfaceRef buffer)
    API_AVAILABLE(macos(10.6), ios(11.0), watchos(4.0), tvos(11.0));

void *IOSurfaceGetBaseAddress(IOSurfaceRef buffer)
    API_AVAILABLE(macos(10.6), ios(11.0), watchos(4.0), tvos(11.0));
