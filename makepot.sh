#!/bin/bash
xgettext --keyword='_' -C -o data/main.pot $(find src -name "*.cpp" -or -name "*.hpp") --add-comments=l10n
find data/ -name "credits.stxt" -print0 | xargs -0 xgettext --keyword='_:1' --language=Lisp --from-code=UTF-8 --sort-by-file --output data/credits.pot --add-comments=l10n
find data/ -name "objects.stoi" -print0 | xargs -0 xgettext --keyword='_:1' --language=Lisp --from-code=UTF-8 --sort-by-file --output data/objects.pot --add-comments=l10n
find data/ -name "*.strf" -print0 | xargs -0 xgettext --keyword='_:1' --language=Lisp --from-code=UTF-8 --sort-by-file --output data/tilesets.pot --add-comments=l10n

msgcat data/main.pot data/objects.pot data/tilesets.pot > data/locale/en.pot
rm -f data/main.pot data/objects.pot data/tilesets.pot 2> /dev/null

# Prepare script files for inclusion in tinygettext
for LEVELSET in $(ls data/levels); do
  SCRIPT_FILES=$(find data/levels/$LEVELSET -name "*.nut")
  for SCRIPT_FILE in $SCRIPT_FILES; do
    name=$(basename ${SCRIPT_FILE})
    name=${name/.nut/}
    python tools/extract_strings.py ${SCRIPT_FILE} data/levels/$LEVELSET/scripts_${name}.txt
  done
done

for LEVELSET in $(ls data/levels); do
  find data/levels/$LEVELSET "(" -name "*.stl" -or -name "*.stwm" -or -name "*.txt" ")" -print0 | xargs -0 xgettext --keyword='_:1' --language=Lisp --sort-by-file --output data/levels/$LEVELSET/en.pot --add-comments=l10n
  sed -i -e 's/\\r\\n/\\n/g' data/levels/$LEVELSET/en.pot
  rm -f data/levels/$LEVELSET/scripts_*.txt 2> /dev/null
done
