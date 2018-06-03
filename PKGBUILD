# vim: set ts=2 sw=2 et:
# $Id$
# Maintainer: Dan McGee <dan@archlinux.org>
# Maintainer: Dave Reisner <dreisner@archlinux.org>

pkgname=pacman
pkgver=5.1.0
pkgrel=2
pkgdesc="A library-based package manager with dependency support"
arch=('x86_64')
url="http://www.archlinux.org/pacman/"
license=('GPL')
groups=('base' 'base-devel')
depends=('bash' 'glibc' 'libarchive' 'curl'
         'gpgme' 'pacman-mirrorlist' 'archlinux-keyring')
makedepends=('asciidoc')   # roundup patch alters docs
checkdepends=('python2' 'fakechroot')
optdepends=('fakeroot: for makepkg usage as normal user perl-locale-gettext: translation support in makepkg-template')
backup=(etc/pacman.conf etc/makepkg.conf)
options=('strip' 'debug')

prepare() {
  cd ..

  # Fix install_packages failure exit code, required by makechrootpkg
  patch -Np1 -i pkg-config/0001-makepkg-Clear-ERR-trap-before-trying-to-restore-it.patch
  patch -Np1 -i pkg-config/0002-makepkg-Don-t-use-parameterless-return.patch
}

build() {
  cd ..

  ./configure --prefix=/usr --sysconfdir=/etc \
    --localstatedir=/var --enable-doc \
    --with-scriptlet-shell=/usr/bin/bash \
    --with-ldconfig=/usr/bin/ldconfig
  make V=1
}

check() {
 echo "skipping"
}

package() {
  cd ..

  make DESTDIR="$pkgdir" install

  # install Arch specific stuff
  install -dm755 "$pkgdir/etc"

  # set things correctly in the default conf file
  install -m644 "pkg-config/pacman.conf" "$pkgdir/etc"
  install -m644 "pkg-config/makepkg.conf" "$pkgdir/etc"

  # put bash_completion in the right location
  install -dm755 "$pkgdir/usr/share/bash-completion/completions"
  mv "$pkgdir/etc/bash_completion.d/pacman" "$pkgdir/usr/share/bash-completion/completions"
  rmdir "$pkgdir/etc/bash_completion.d"

  for f in makepkg pacman-key; do
    ln -s pacman "$pkgdir/usr/share/bash-completion/completions/$f"
  done
}

