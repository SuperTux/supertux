#!/bin/sh
ruby extconf.rb
make
cd ..
ruby installer.rb
cd physfs
ruby install.rb
cd test
ruby test_physfs.rb