local sha = io.sha1("app0:main.lua")
local sha0 = io.crc32("test")
while true do
draw.text(10, 10, "SHA1 hash of app0:main.lua: "..sha, color.new(255, 255, 255))
draw.text(10, 30, "CRC32 of \"test\": "..string.format("%u (0x%08X)", sha0, sha0), color.new(255, 255, 255))
controls.update()
if controls.released(SCE_CTRL_START) then controls.update() break end
draw.swapbuffers()
end
