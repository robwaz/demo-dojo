gcc -g -O0 -fno-omit-frame-pointer -fno-stack-protector -fcf-protection=none -no-pie -z noexecstack -Wno-stringop-overflow demo.c -o demo
