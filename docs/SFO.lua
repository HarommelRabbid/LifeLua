local white = color.new(255, 255, 255)
local sfo = io.readsfo("app0:sce_sys/param.sfo") -- read it once, before the loop

while true do
    draw.text(10, 10, "From \"app0:sce_sys/param.sfo\":", white)
    local y = 50
    for key, value in pairs(sfo) do
        draw.text(10, y, key..": "..tostring(value), white)
        y = y + 20
    end
    draw.swapbuffers()
end
