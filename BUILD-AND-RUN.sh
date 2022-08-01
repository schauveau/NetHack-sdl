set -x
set -e
make all install WANT_WIN_SC=1 WANT_WIN_CURSES=1 WANT_DEFAULT=SC
echo "Entering directory '/home/chauveau/git/NetHack/src'"
./src/nethack
