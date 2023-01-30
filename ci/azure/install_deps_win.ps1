$COMPILER=$Env:COMPILER
$ARCH=$Env:ARCH

$src_dir=$pwd


if (!(Test-Path deps)) {
	mkdir deps
}
cd deps

echo "Build deps..."

git clone --depth 1 https://github.com/analogdevicesinc/libtinyiiod.git
cd libtinyiiod
if ($ARCH -eq "Win32") {
    mkdir build-win32
    cd build-win32
} else {
    mkdir build-x64
    cd build-x64
}

cmake -G "$COMPILER" -A "$ARCH" -DCMAKE_SYSTEM_PREFIX_PATH="C:" -DBUILD_EXAMPLES=OFF ..
cmake --build . --target install --config Release

echo "Downloading deps..."
cd C:\
wget http://swdownloads.analog.com/cse/build/libiio-win-deps-libusb1.0.24.zip -OutFile "libiio-win-deps.zip"
7z x -y "C:\libiio-win-deps.zip"

set PATH=%PATH%;"C:\Program Files (x86)\Inno Setup 6"

mkdir libiio
cd libiio
wget https://github.com/analogdevicesinc/libiio/releases/download/v0.24/Windows-VS-2019-x64.zip -OutFile "libiio.zip"
7z x -y "C:\libiio.zip"
dir 
cd ..
