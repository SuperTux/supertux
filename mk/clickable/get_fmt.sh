#!/usr/bin/env sh

# Get fmt version 9 for Ubuntu 20 and older.

wget archive.ubuntu.com/ubuntu/pool/universe/f/fmtlib/libfmt9_9.1.0+ds1-2_${ARCH}.deb
sudo dpkg -i libfmt9_9.1.0+ds1-2_${ARCH}.deb

wget archive.ubuntu.com/ubuntu/pool/universe/f/fmtlib/libfmt-dev_9.1.0+ds1-2_${ARCH}.deb
sudo dpkg -i libfmt-dev_9.1.0+ds1-2_${ARCH}.deb
