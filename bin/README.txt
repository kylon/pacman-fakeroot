How to build:

If you are using official pacman >=4.2.0 you CAN'T build this pkg with root privileges, use a standard user instead.

./autogen.sh -ONLY FIRST TIME-
make clean -AFTER FIRST TIME, IF YOU WANT TO MAKE A CLEAN BUILD-
makepkg

How to install:
pacman -U packegName.pkg.tar.xz