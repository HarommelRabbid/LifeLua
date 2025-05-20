local tmr = timer.new()
local inc = 0
tmr:start()
function LifeLuaProgressMessageDialog()
controls.update()
if controls.pressed(SCE_CTRL_CROSS) then os.progressmessagetext("Changed") end
if tmr:elapsed() >= 0.1 and inc <= 100 then inc = inc + 1 os.incprogressmessage(1) tmr:stop() tmr:start() end
if inc == 100 then tmr:stop() os.closemessage() end
if controls.released(SCE_CTRL_START) then os.abortmessage() end
end
os.progressmessage("test")
