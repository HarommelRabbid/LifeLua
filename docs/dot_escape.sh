DIR="../ll_docs"

find "${DIR}/" -type f -name "*.html" -exec sed -i '' 's/\(․\)/./g' {} +
find "${DIR}/" -type f -name "*.js" -exec sed -i '' 's/\(․\)/./g' {} +
find "${DIR}/" -type f -name "*.css" -exec sed -i '' 's/\(․\)/./g' {} +

OLD="․"
NEW="."

for filepath in "$DIR"/*"$OLD"*; do
  [ -e "$filepath" ] || continue
  filename=$(basename "$filepath")
  newname="${filename//$OLD/$NEW}"
  mv -v -- "$filepath" "$DIR/$newname"
done