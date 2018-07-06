#!/bin/sh

package_name="SuperTux"
package_version="$(git describe --tags --match "?[0-9]*.[0-9]*.[0-9]*")"

xgettext --keyword='_' --keyword='__:1,2' -C -o data/locale/main.pot \
  $(find src -name "*.cpp" -or -name "*.hpp") \
  --add-comments=l10n \
  --package-name="${package_name}" --package-version="${package_version}" \
  --msgid-bugs-address=https://github.com/SuperTux/supertux/issues

find data/ -name "credits.stxt" -print0 | xargs -0 xgettext --keyword='_:1' \
  --language=Lisp --from-code=UTF-8 --sort-by-file \
  --output data/locale/credits.pot --add-comments=l10n \
  --package-name="${package_name}" --package-version="${package_version}" \
  --msgid-bugs-address=https://github.com/SuperTux/supertux/issues

find data/ -name "objects.stoi" -print0 | xargs -0 xgettext --keyword='_:1' \
  --language=Lisp --from-code=UTF-8 --sort-by-file \
  --output data/locale/objects.pot --add-comments=l10n \
  --package-name="${package_name}" --package-version="${package_version}" \
  --msgid-bugs-address=https://github.com/SuperTux/supertux/issues

find data/ -name "*.strf" -print0 | xargs -0 xgettext --keyword='_:1' \
  --language=Lisp --from-code=UTF-8 --sort-by-file \
  --output data/locale/tilesets.pot --add-comments=l10n \
  --package-name="${package_name}" --package-version="${package_version}" \
  --msgid-bugs-address=https://github.com/SuperTux/supertux/issues

msgcat data/locale/main.pot data/locale/credits.pot data/locale/objects.pot data/locale/tilesets.pot > data/locale/messages.pot
rm -f data/locale/main.pot data/locale/credits.pot data/locale/objects.pot data/locale/tilesets.pot

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
  find "data/levels/${LEVELSET}" "(" -name "*.stl" -or -name "*.stl.in" -or -name "*.stwm" -or -name "*.txt" ")" -print0 | xargs -0 xgettext --keyword='_:1' \
    --language=Lisp --from-code=UTF-8 --sort-by-file \
    --output "data/levels/${LEVELSET}/messages.pot" --add-comments=l10n \
    --package-name="${package_name}" --package-version="${package_version}" \
    --msgid-bugs-address=https://github.com/SuperTux/supertux/issues
  sed -i -e 's/\\r\\n/\\n/g' "data/levels/${LEVELSET}/messages.pot"
  rm -f data/levels/$LEVELSET/scripts_*.txt
done
