#define VER_MAJOR 22621
#define VER_MINOR 3880
#define VER_BUILD_HI    66
#define VER_BUILD_LO 6
#define VER_FLAGS   VS_FF_PRERELEASE


// The Binary form of the version numbers
#define VER_FILE    VER_MAJOR, VER_MINOR, VER_BUILD_HI, VER_BUILD_LO
#define VER_PRODUCT VER_MAJOR, VER_MINOR, VER_BUILD_HI, VER_BUILD_LO

#define VER_STR(arg) #arg

#define STRINGIFYVER2(X)      #X
#define STRINGIFYVER(X)       STRINGIFYVER2(X)
#define VER_WITH_DOTS STRINGIFYVER(VER_MAJOR) "." STRINGIFYVER(VER_MINOR) "." STRINGIFYVER(VER_BUILD_HI) "." STRINGIFYVER(VER_BUILD_LO)

// The String form of the version numbers
#define VER_FILE_STRING VALUE "FileVersion", VER_WITH_DOTS
#define VER_PRODUCT_STRING VALUE "ProductVersion", VER_WITH_DOTS
