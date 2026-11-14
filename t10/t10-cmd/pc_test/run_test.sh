#!/usr/bin/env bash
# T10 白盒测试：编译 + 运行。在 Git Bash 里跑，不用板子。
#   cd /d/stm32_code/t10/t10-cmd
#   ./pc_test/run_test.sh
set -e
cd "$(dirname "$0")"

GCC="${GCC:-D:/w64devkit/bin/gcc.exe}"

if [ ! -x "$GCC" ] && ! command -v "$GCC" >/dev/null 2>&1; then
    echo "找不到 gcc：$GCC"
    echo "如果 w64devkit 装在别处，用 GCC=/你的路径/gcc.exe ./pc_test/run_test.sh"
    exit 1
fi

"$GCC" -Wall -Wextra -I../src test_cmd.c ../src/cmd.c -o test_cmd.exe
./test_cmd.exe
