sfo = io.readsfo("app0:sce_sys/param.sfo")
while true do
    draw.text(10, 10, "From \"app0:sce_sys/param.sfo\": ", white)
    local y = 50
    for k, v in pairs(sfo) do
        draw.text(10, y, tostring(k)..": "..v, white)
        y = y + 20
    end
    draw.swapbuffers()
end