db = sqlite3.open("ur0:shell/db/app.db", SQLITE_OPEN_READONLY)
query = db:query("SELECT * FROM tbl_appinfo_icon WHERE titleId IS NOT NULL")
queryscroll = newScroll(query, 26)
while true do
local y = 10
for i=queryscroll.ini, queryscroll.lim do
if i == queryscroll.sel then
draw.text(10, y, query[i].titleId.." - "..query[i].title or "", blue)
else
draw.text(10, y, query[i].titleId.." - "..query[i].title or "", white)
end
y = y + 20
end
controls.update()
lx, ly = controls.leftanalog()
if controls.pressed(SCE_CTRL_UP) or ly <= -64 then queryscroll:up() end
if controls.pressed(SCE_CTRL_DOWN) or ly >= 64 then queryscroll:down() end
if controls.pressed(SCE_CTRL_ACCEPT) then
os.uri("psgm:play?titleid="..query[queryscroll.sel].titleId, 0x2000000)
end
draw.swapbuffers()
end
