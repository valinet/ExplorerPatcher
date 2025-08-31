rmdir /s /q libs\zlib\build-x64
rmdir /s /q libs\zlib\build-arm64

if "%VSINSTALLDIR:~-1%"=="\" (
    set "EP_VSINSTALLDIR=%VSINSTALLDIR:~0,-1%"
) else (
    set "EP_VSINSTALLDIR=%VSINSTALLDIR%"
)

cmake libs/zlib -Blibs/zlib/build-x64 -G "Visual Studio 17 2022" -A x64 -D"CMAKE_GENERATOR_INSTANCE:PATH=%EP_VSINSTALLDIR%" -D"CMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded$<$<CONFIG:Debug>:Debug>" -DCMAKE_POLICY_DEFAULT_CMP0091=NEW
cmake libs/zlib -Blibs/zlib/build-arm64 -G "Visual Studio 17 2022" -A ARM64 -D"CMAKE_GENERATOR_INSTANCE:PATH=%EP_VSINSTALLDIR%" -D"CMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded$<$<CONFIG:Debug>:Debug>" -DCMAKE_POLICY_DEFAULT_CMP0091=NEW

cmake --build libs/zlib/build-x64 --config Release
cmake --build libs/zlib/build-arm64 --config Release
