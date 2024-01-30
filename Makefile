GCC_BIN="`xcrun --sdk iphoneos --find clang`"
SDK="`xcrun --sdk iphoneos --show-sdk-path`"
ARCH_FLAGS=-arch arm64e

LDFLAGS	=\
	-F$(SDK)/System/Library/Frameworks/\
	-framework CoreFoundation\
	-framework CoreGraphics\
	-framework IOSurface\
	-framework IOKit\
	-mios-version-min=13.0\
	-I.

GCC_ARM = $(GCC_BIN) -mios-version-min=13.0 -Os -Wimplicit -isysroot $(SDK) $(ARCH_FLAGS)

screenshot: screenshot.c
	@$(GCC_ARM) $(LDFLAGS) -o $@ $^
	codesign -s - --entitlements Entitlements.plist $@

clean:
	rm -f screenshot *.o
