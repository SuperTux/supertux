#!/bin/sh
xgettext --keyword='_' -C -o data/locale/messages.pot `find src -name "*.cpp"`
for LEVELSET in "bonus1" "bonus2" "contribs" "misc" "test" "world1" "world2"; do
  xgettext --keyword='_:1' --language=Lisp --sort-by-file -C -o data/levels/$LEVELSET/messages.pot `find data/levels/$LEVELSET -name "*.stl"` `find data/levels/$LEVELSET -name "*.stwm"` `find data/levels/$LEVELSET -name "*.txt"`
done
