local db = sqlite3.open("ur0:shell/db/app.db", SQLITE_OPEN_READONLY)
local query = db:query("SELECT * FROM tbl_appinfo_icon WHERE titleId IS NOT NULL")
local scroll = newScroll(query, 7)
local ii = 1
function LifeLuaSystemMessageDialog()
if ii <= #query then
table.insert(icons, image.load(query[ii].iconPath) or image.new(128, 128, white))
ii = ii + 1
else
os.closemessage()
end
end
if #icons <= 0 then
os.systemmessage(SCE_MSG_DIALOG_SYSMSG_TYPE_WAIT)
end
while true do
draw.text(10, 10, os.date("%c"), white)
x = 2
for i=scroll.ini, scroll.lim do
draw.rect(x, (544-128)/2, 128, 128, white)
if i == scroll.sel then
icons[i]:display(x, (544-128)/2)
draw.rect(x, (544-128)/2, 128, 128, white:a(128))
draw.text(10, 544-10-draw.textheight(query[i].title), query[i].title, white)
draw.text(960-10-draw.textwidth(query[i].titleId), 544-10-draw.textheight(query[i].titleId), query[i].titleId, white)
else
icons[i]:display(x, (544-128)/2)
end
draw.rect(x, ((544-128)/2)+128+10, 128, 128, white:a(128))
icons[i]:scaledisplay(x+128, ((544-128)/2)+128+10+128, -1, -1, color.new(0, 0, 0))
icons[i]:scaledisplay(x+128, ((544-128)/2)+128+10+128, -1, -1, white:a(128))
draw.gradientrect(x, ((544-128)/2)+128+10, 128, 128, white:a(0), white:a(0), color.new(0, 0, 0), color.new(0, 0, 0))
x = x + 128+10
end
controls.update()
lx, ly = controls.leftanalog()
if controls.rapidfireheld(SCE_CTRL_LEFT, 8) or lx <= -64 then scroll:up() end
if controls.rapidfireheld(SCE_CTRL_RIGHT, 8) or lx >= 64 then scroll:down() end
if controls.rapidfireheld(SCE_CTRL_LTRIGGER, 8) then for i=1, 6 do scroll:up() end end
if controls.rapidfireheld(SCE_CTRL_RTRIGGER, 8) then for i=1, 6 do scroll:down() end end
if controls.pressed(SCE_CTRL_ACCEPT) then
os.uri("psgm:play?titleid="..query[scroll.sel].titleId, 0x2000000) end
if controls.released(SCE_CTRL_START) then db:close() controls.update() break end
draw.swapbuffers()
end
