#!/bin/bash

# 创建构建目录
mkdir -p ../build/cmake
cd ../build/cmake

# 检查参数以决定是否使用交叉编译
if [ "$1" = "cross" ]; then
    echo "执行交叉编译..."
    cmake ../../ -DCMAKE_TOOLCHAIN_FILE=../../config/cmake/toolchain.cmake
else
    echo "执行本地编译..."
    cmake ../../
fi

# 执行构建
make

# 如果构建成功，返回scripts目录
if [ $? -eq 0 ]; then
    echo "构建成功完成！"
else
    echo "构建失败！"
fi

exit 1