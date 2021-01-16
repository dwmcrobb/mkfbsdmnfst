# mkfbsdmnfst

Emits a FreeBSD package manifest file (+MANIFEST) on stdout
from a template manifest file and/or command-line options.  It may be
used when creating software packages for FreeBSD.

## Build
flex and bison are required.  If you don't have them:
```
pkg install flex
pkg install bison
```
Then it's just:
```
./configure
gmake package
```

## Install
```gmake package``` will create a FreeBSD package which can be install with ```pkg install ...```.  For example,
```
pkg install mkfbsdmnfst-1.0.0.txz
```

### Usage
See the manpage for usage.
