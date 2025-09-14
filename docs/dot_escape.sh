DIR="../../ll_docs"
SEARCHDIR="../../ll_docs/search"

perl -pi -e 's/․/./g' "$DIR"/*.* "$SEARCHDIR"/*.*

OLD="․"
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
patch -p1 -d "$SEARCHDIR" < search.js.patch