#/bin/sh -x
set -o
cd $(dirname "$0")
cd sys/unix 
sh -x setup.sh hints/linux.370
cd ../..
make fetch-Lua
#make all WANT_WIN_SGL=1

