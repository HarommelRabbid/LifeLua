sfo = io.readsfo("app0:sce_sys/param.sfo")
while true do
local y = 10
for k, v in pairs(sfo) do
draw.text(10, y, tostring(k)..": "..v, white)
y = y + 20
end
draw.swapbuffers()
end
