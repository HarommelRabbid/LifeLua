function LifeLuaNetworkDownload(read, wrote, size, speed)
draw.text(10, 10, read, white)
draw.text(10, 30, math.floor((wrote*100)/size).."%", white)
draw.text(10, 50, size.." B", white)
draw.gradientrect(0, 544-39, (wrote*960)/size, 39, white, blue, white, blue)
draw.swapbuffers()
end
while true do
draw.text(10, 10, "WIFI enabled: "..tostring(network.wifi()), white)
draw.text(10, 30, "IP address: "..network.ip().." (no reason to censor, it's local)", white)
draw.text(10, 50, "MAC address: "..network.mac(), white)
draw.text(10, 70, "Press Cross to download Rabbid MultiTool 0.12pre2 to \"ux0:rbmt.vpk\"", white)
controls.update()
if controls.released(SCE_CTRL_START) then controls.update() break end
if controls.released(SCE_CTRL_CROSS) then network.download("https://github.com/HarommelRabbid/RabbidMultiToolVita/releases/download/0.12pre2/Rabbid.MultiTool.0.12pre2.vpk", "ux0:rbmt.vpk") end
draw.swapbuffers()
end
