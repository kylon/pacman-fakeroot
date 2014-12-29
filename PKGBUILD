# vim: set ts=2 sw=2 et:
# $Id$
# Maintainer: Dan McGee <dan@archlinux.org>
# Maintainer: Dave Reisner <dreisner@archlinux.org>

pkgname=pacman
pkgver=4.2.0
pkgrel=5
pkgdesc="A library-based package manager with dependency support"
arch=('i686' 'x86_64')
url="http://www.archlinux.org/pacman/"
license=('GPL')
groups=('base' 'base-devel')
depends=('bash' 'glibc' 'libarchive>=3.1.2' 'curl>=7.39.0'
         'gpgme' 'pacman-mirrorlist' 'archlinux-keyring')
checkdepends=('python2' 'fakechroot')
optdepends=('fakeroot: for makepkg usage as normal user')
provides=('pacman-contrib')
conflicts=('pacman-contrib')
replaces=('pacman-contrib')
backup=(etc/pacman.conf etc/makepkg.conf)
options=('strip' 'debug')

build() {
cd ..
 ./configure --prefix=/usr --sysconfdir=/etc \
    --localstatedir=/var --enable-doc \
    --with-scriptlet-shell=/usr/bin/bash \
    --with-ldconfig=/usr/bin/ldconfig
  make
  make -C contrib
}

check() {
 echo "skipping"
}

package() {
cd ..
  make DESTDIR="$pkgdir" install
  make DESTDIR="$pkgdir" -C contrib install

  # install Arch specific stuff
  install -dm755 "$pkgdir/etc"
  install -m644 "core-config/core-$CARCH/pacman.conf.$CARCH" "$pkgdir/etc/pacman.conf"

  case $CARCH in
    i686)
      mycarch="i686"
      mychost="i686-pc-linux-gnu"
      myflags="-march=i686"
      ;;
    x86_64)
      mycarch="x86_64"
      mychost="x86_64-unknown-linux-gnu"
      myflags="-march=x86-64"
      ;;
  esac

  # set things correctly in the default conf file
  install -m644 "core-config/core-$CARCH/makepkg.conf" "$pkgdir/etc"
  sed -i "$pkgdir/etc/makepkg.conf" \
    -e "s|@CARCH[@]|$mycarch|g" \
    -e "s|@CHOST[@]|$mychost|g" \
    -e "s|@CARCHFLAGS[@]|$myflags|g"

  # put bash_completion in the right location
  install -dm755 "$pkgdir/usr/share/bash-completion/completions"
  mv "$pkgdir/etc/bash_completion.d/pacman" "$pkgdir/usr/share/bash-completion/completions"
  rmdir "$pkgdir/etc/bash_completion.d"

  for f in makepkg pacman-key; do
    ln -s pacman "$pkgdir/usr/share/bash-completion/completions/$f"
  done

  install -Dm644 contrib/PKGBUILD.vim "$pkgdir/usr/share/vim/vimfiles/syntax/PKGBUILD.vim"
}
