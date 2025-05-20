local x1, x2 = 10, 10
local score1, score2 = 0, 0
local ballX, ballY, ballspeedX, ballspeedY = 480, 544/2, 4.5, 4.5
while true do
draw.gradientrect(x1, 10, 150, 30, white, white, blue, blue)
draw.gradientrect(x2, 544-30-10, 150, 30, blue, blue, white, white)
draw.circle(ballX, ballY, 20, white)

if ballX <= 10+20 or ballX >= 960-(10+20) then ballspeedX = -ballspeedX end
if (ballY <= 30 or ballY >= 544-30) or (ballY <= 60 and (ballX >= x1 and ballX <= x1+120)) or (ballY <= 544-60 and (ballX >= x2 and ballX <= x2+120)) then ballspeedY = -ballspeedY end
ballX = ballX + ballspeedX
ballY = ballY + ballspeedY

controls.update()
if controls.held(SCE_CTRL_LEFT) then x1 = math.range(x1-5, 10, 960-150-10) end
if controls.held(SCE_CTRL_RIGHT) then x1 = math.range(x1+5, 10, 960-150-10) end
if controls.released(SCE_CTRL_START) then 
if os.message("Are you sure you want to quit?", SCE_MSG_DIALOG_BUTTON_TYPE_YESNO) then
controls.update() break end
end
draw.swapbuffers()
end
