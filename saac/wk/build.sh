#!/bin/bash
# acwk (saac worker) 编译脚本
# 用法: bash build.sh
# 注意: 必须 -O0 (否则 Log 输出被优化掉); GCC 14 将隐式声明视为 error, 需补全系统头
cd "$(dirname "$0")"
gcc -o acwk -O0 -g -w -fcommon -D_FIX_WORKS -D_FIX_MESSAGE \
    -include stdio.h -include stdlib.h -include string.h \
    -include ctype.h -include time.h -include errno.h \
    -include arpa/inet.h \
    -include error.h -include TCPIP.h -include tools.h -include version.h \
    wk.c error.c TCPIP.c tools.c
ls -la acwk && echo "acwk 编译成功"
