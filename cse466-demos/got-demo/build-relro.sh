#!/bin/bash

gcc -fPIE -Wl,-z,relro -Wl,-z,now -o full_relro $1
