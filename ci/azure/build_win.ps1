$COMPILER=$Env:COMPILER
$ARCH=$Env:ARCH

$src_dir=$pwd


if ($ARCH -eq "Win32") {
    echo "Running cmake for $COMPILER on 32 bit..."
    mkdir build-win32
    cp .\iio-emu.iss.cmakein .\build-win32
    cd build-win32

    cmake -G "$COMPILER" -A "$ARCH" -DCMAKE_SYSTEM_PREFIX_PATH="C:" -DBUILD_TOOLS=ON -DLIBXML2_LIBRARIES="C:\\libs\\32\\libxml2.lib" -DLIBXML2_INCLUDE_DIR="C:\\include\\libxml2" -DLIBIIO_LIBRARIES:FILEPATH="C:\\libiio\\libiio.lib" -DLIBIIO_INCLUDEDIR:PATH="C:\\libiio\\" ..
    cmake --build . --config Release
    cp .\iio-emu.iss $env:BUILD_ARTIFACTSTAGINGDIRECTORY
} else {
    echo "Running cmake for $COMPILER on 64 bit..."
    mkdir build-x64
    cp .\iio-emu.iss.cmakein .\build-x64
    cd build-x64

    cmake -G "$COMPILER" -A "$ARCH" -DCMAKE_SYSTEM_PREFIX_PATH="C:" -DBUILD_TOOLS=OFF -DLIBXML2_LIBRARIES="C:\\libs\\64\\libxml2.lib" -DLIBXML2_INCLUDE_DIR="C:\\include\\libxml2" ..
    cmake --build . --config Release
    cp .\iio-emu.iss $env:BUILD_ARTIFACTSTAGINGDIRECTORY
}
