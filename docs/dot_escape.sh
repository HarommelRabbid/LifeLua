DIR="../../ll_docs"

perl -pi -e 's/․/./g' "$DIR"/*.{html,js,css}

OLD="․"
NEW="."

for filepath in "$DIR"/*"$OLD"*; do
  [ -e "$filepath" ] || continue
  filename=$(basename "$filepath")
  newname="${filename//$OLD/$NEW}"
  mv -v -- "$filepath" "$DIR/$newname"
done