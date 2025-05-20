local ftpinit = false
while true do
if not ftpinit then
draw.text(10, 10, "Press Cross to enable FTP", white)
else
draw.text(10, 10, "FTP enabled at: "..ftp.ip..":"..ftp.port.."\nPress Cross to disable FTP", white)
end
controls.update()
if controls.pressed(SCE_CTRL_CROSS) then 
if not ftpinit then 
os.lock(true, SCE_SHELL_UTIL_LOCK_TYPE_PS_BTN, SCE_SHELL_UTIL_LOCK_TYPE_QUICK_MENU)
ftp = network.ftp(true) 
network.ftpadddevice("ux0:") 
ftpinit = true 
else 
os.lock(false, SCE_SHELL_UTIL_LOCK_TYPE_PS_BTN, SCE_SHELL_UTIL_LOCK_TYPE_QUICK_MENU)
network.ftp(false) 
ftpinit = false 
end 
end
if controls.released(SCE_CTRL_START) and not ftpinit then controls.update() break end
draw.swapbuffers()
end
