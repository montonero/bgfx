#!/bin/bash
SHADERC=./../../../tools/bin/linux/shaderc
BUILD_DIR=../../../.build/shaders
OUT_DIR=../../runtime/shaders

$SHADERC --platform windows -p vs_4_0 -O 3 -i ../../../src/ --type vertex --depends  --varyingdef varying_voxel.sc -o $BUILD_DIR/dx11/vs_voxel.bin -f vs_voxel.sc --disasm
cp $BUILD_DIR/dx11/vs_voxel.bin $OUT_DIR/dx11/vs_voxel.bin

$SHADERC --platform windows -p ps_4_0 -O 3 -i ../../../src/ --type fargment --depends  --varyingdef varying_voxel.sc -o $BUILD_DIR/dx11/fs_voxel.bin -f fs_voxel.sc --disasm
cp $BUILD_DIR/dx11/fs_voxel.bin $OUT_DIR/dx11/fs_voxel.bin

VS_FLAGS="--platform linux -p 120"
FS_FLAGS="--platform linux -p 120"
CS_FLAGS="--platform linux -p 430"
SHADER_PATH=glsl

$SHADERC $VS_FLAGS -O 3 -i ../../../src/ --type vertex --depends  --varyingdef varying_voxel.sc -o $BUILD_DIR/$SHADER_PATH/vs_voxel.bin -f vs_voxel.sc --disasm
cp $BUILD_DIR/$SHADER_PATH/vs_voxel.bin $OUT_DIR/$SHADER_PATH/vs_voxel.bin

$SHADERC $FS_FLAGS -O 3 -i ../../../src/ --type fargment --depends  --varyingdef varying_voxel.sc -o $BUILD_DIR/$SHADER_PATH/fs_voxel.bin -f fs_voxel.sc --disasm
cp $BUILD_DIR/$SHADER_PATH/fs_voxel.bin $OUT_DIR/$SHADER_PATH/fs_voxel.bin