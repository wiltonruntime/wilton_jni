#/bin/bash

set -e
set -x

ANDROID_ARMEABI_TOOLCHAIN_DIR=/home/alex/apps/android-ndk-r9d-arm-linux-androideabi-4.8/
ANDROID_X86_TOOLCHAIN_DIR=/home/alex/apps/android-ndk-r9d-x86-linux-android-4.8/

echo --- android_armeabi_gcc
. creset
cmake ../jni \
    -DSTATICLIB_TOOLCHAIN=android_armeabi_gcc \
    -DANDROID_TOOLCHAIN_DIR=$ANDROID_ARMEABI_TOOLCHAIN_DIR \
    -DCMAKE_BUILD_TYPE=Release
make
make mvninstall_jar

echo --- android_i386_gcc
. creset
cmake ../jni \
    -DSTATICLIB_TOOLCHAIN=android_i386_gcc \
    -DANDROID_TOOLCHAIN_DIR=$ANDROID_X86_TOOLCHAIN_DIR \
    -DCMAKE_BUILD_TYPE=Release
make
make mvninstall_jar

echo --- linux_amd64_gcc
. creset
cmake ../jni \
    -DSTATICLIB_TOOLCHAIN=linux_amd64_gcc \
    -DCMAKE_BUILD_TYPE=Release
make
make mvninstall_so

echo --- FINISH_SUCCESS
