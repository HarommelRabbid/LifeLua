# Doxygen doesn't allow "." in function names, so the docs headers write
# Lua names like image.load as "imageꓸload", using "ꓸ" (U+A4F8 LISU LETTER TONE MYA TI)
# as a stand-in dot. After Doxygen runs, this script turns every "ꓸ" in the
# generated HTML (and in the file names) back into a real ".".

DIR="../../ll_docs"
SEARCHDIR="../../ll_docs/search"

perl -pi -e 's/ꓸ/./g' "$DIR"/*.* "$SEARCHDIR"/*.*

OLD="ꓸ"
NEW="."

for filepath in "$DIR"/*"$OLD"*; do
  [ -e "$filepath" ] || continue
  filename=$(basename "$filepath")
  newname="${filename//$OLD/$NEW}"
  mv -v -- "$filepath" "$DIR/$newname"
done
for filepath in "$SEARCHDIR"/*"$OLD"*; do
  [ -e "$filepath" ] || continue
  filename=$(basename "$filepath")
  newname="${filename//$OLD/$NEW}"
  mv -v -- "$filepath" "$SEARCHDIR/$newname"
done
patch -p0 -d "$SEARCHDIR" < search.js.patch
