gcc -g -O0 -fno-omit-frame-pointer -fno-stack-protector -fcf-protection=none \
    -fPIE -pie -z noexecstack stackpivot.c -o stackpivot -ldl