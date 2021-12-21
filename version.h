#define VER_MAJOR 22000
#define VER_MINOR 376
#define VER_BUILD_HI    40
#define VER_BUILD_LO 15
#define VER_FLAGS   VS_FF_PRERELEASE


// The Binary form of the version numbers
#define VER_FILE    VER_MAJOR, VER_MINOR, VER_BUILD_HI, VER_BUILD_LO
#define VER_PRODUCT VER_MAJOR, VER_MINOR, VER_BUILD_HI, VER_BUILD_LO

#define VER_STR(arg) #arg

// The String form of the version numbers
#define VER_FILE_STRING VALUE "FileVersion", "22000.376.40.15"
#define VER_PRODUCT_STRING VALUE "ProductVersion", "22000.376.40.15"
