#!/bin/sh
xgettext --keyword='_' -C -o translations/messages.pot $(find src -name "*.cpp" -or -name "*.hpp") --add-comments=l10n
find data/ "(" -name "credits.stxt" ")" -print0 | xargs -0 xgettext --keyword='_:1' --language=Lisp --from-code=UTF-8 --sort-by-file --output translations/credits.pot --add-comments=l10n

# Prepare script files for inclusion in tinygettext
for LEVELSET in $(ls data/levels); do
  SCRIPT_FILES=$(find data/levels/$LEVELSET -name "*.nut")
  for SCRIPT_FILE in $SCRIPT_FILES; do
    name=$(basename ${SCRIPT_FILE})
    name=${name/.nut/}
    python ./extract_strings.py ${SCRIPT_FILE} data/levels/$LEVELSET/scripts_${name}.txt
  done
done

for LEVELSET in $(ls data/levels); do
  find data/levels/$LEVELSET "(" -name "*.stl" -or -name "*.stwm" -or -name "*.txt" ")" -print0 | xargs -0 xgettext --keyword='_:1' --language=Lisp --sort-by-file --output translations/$LEVELSET.pot --add-comments=l10n
  sed -i -e 's/\\r\\n/\\n/g' translations/$LEVELSET.pot
  rm -f data/levels/$LEVELSET/scripts_*.txt 2> /dev/null
done
