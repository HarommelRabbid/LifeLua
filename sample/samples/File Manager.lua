dofile("scroll.lua")
local files = io.list("ux0:")
local filescroll = newScroll(files, 26)
local longestint, longest = 0, ""
while true do
for i=1, #files do
if #files[i].name > longestint then
longestint = #files[i].name
longest = files[i].name
end
end
local y = 10
for i = filescroll.ini, filescroll.lim do
if files[i].isafolder then
if i == filescroll.sel then
draw.text(10, y, files[i].name.."/", color.new(0, 0, 255))
draw.text(10+draw.textwidth(longest.."/")+20, y, files[i].created, color.new(0, 0, 255))
else
draw.text(10, y, files[i].name.."/", color.new(255, 255, 255))
draw.text(10+draw.textwidth(longest.."/")+20, y, files[i].created, color.new(255, 255, 255))
end
else
if i == filescroll.sel then
draw.text(10, y, files[i].name, color.new(0, 0, 255))
draw.text(10+draw.textwidth(longest.."/")+20, y, files[i].size.." B", color.new(0, 0, 255))
else
draw.text(10, y, files[i].name, color.new(255, 255, 255))
draw.text(10+draw.textwidth(longest.."/")+20, y, files[i].size.." B", color.new(255, 255, 255))
end
end
y = y+20
end
controls.update()
if controls.pressed(SCE_CTRL_UP) then filescroll:up() end
if controls.pressed(SCE_CTRL_DOWN) then filescroll:down() end
if controls.released(SCE_CTRL_CROSS) then 
if files[filescroll.sel].isafolder then
if #io.list(files[filescroll.sel].path) >= 1 then
files = io.list(files[filescroll.sel].path)
filescroll:set(files, 26) 
end
else
os.keyboard("Enter text", assert(io.open(files[filescroll.sel].path, "r"):read("*all")), SCE_IME_TYPE_DEFAULT, SCE_IME_DIALOG_TEXTBOX_MODE_DEFAULT, SCE_IME_OPTION_MULTILINE)
end
end
if controls.released(SCE_CTRL_CIRCLE) then
if io.filestrip(files[1].path):gsub("/", "") == "ux0:" then
controls.update() break
else
files = io.list(io.filestrip(io.filestrip(files[filescroll.sel].path)))
filescroll:set(files, 26) 
end
end
if controls.released(SCE_CTRL_START) then controls.update() break end
draw.swapbuffers()
end
