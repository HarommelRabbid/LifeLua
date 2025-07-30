find html/ -type f -name "*.html" -exec sed -i '' 's/\(․\)/./g' {} +
find html/ -type f -name "*.js" -exec sed -i '' 's/\(․\)/./g' {} +
find html/ -type f -name "*.css" -exec sed -i '' 's/\(․\)/./g' {} +
find html/ -type f -name "*.html" -exec sed -i '' 's/\(․\)/./g' {} +
find html/ -type f -name "*.js" -exec sed -i '' 's/\(․\)/./g' {} +
find html/ -type f -name "*.css" -exec sed -i '' 's/\(․\)/./g' {} +

OLD="․"
NEW="."
DIR="html"

for filepath in "$DIR"/*"$OLD"*; do
  [ -e "$filepath" ] || continue
  filename=$(basename "$filepath")
  newname="${filename//$OLD/$NEW}"
  mv -v -- "$filepath" "$DIR/$newname"
done