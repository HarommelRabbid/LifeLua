--colors & fonts
local white = Color.new(255, 255, 255)
local blue = Color.new(0, 0, 255)

--positions & everything else
local y1, y2, ballspeedx, ballspeedy = 10, 10, 0.5, 0.5
local circlex, circley = 960/2, 544/2
local pointsP1, pointsP2 = 0,0

function math.range(num, min, max)
return math.min(math.max(num, min), max)
end

while true do
--drawing the ball & paddles
Graphics.initBlend() 
Screen.clear()
Graphics.fillRect(10, 30, y1, y1+120, blue) --first paddle
Graphics.fillRect(960-10, 960-30, y2, y2+120, white) --second paddle
Graphics.fillCircle(circlex, circley, 12, white) --ball
--score display
Graphics.debugPrint(960/2-string.len(tostring(pointsP1))*5-10, 10, pointsP1, white)
Graphics.debugPrint(960/2+string.len(tostring(pointsP1))*5+10, 10, pointsP2, white)
Graphics.termBlend() 
Screen.flip()

--ball movement
if circley <= 12 or circley >= 544-12 then ballspeedy = -ballspeedy end
if (circlex <= 12 or circlex >= 960-12) or (circlex == 40 and (circley >= y1 and circley <= y1+120)) or (circlex == 960-40 and (circley >= y2 and circley <= y2+120)) then ballspeedx = -ballspeedx end
if circlex >= 960-12 then pointsP1 = pointsP1 + 1 end
if circlex <= 12 then pointsP2 = pointsP2 + 1 end
circley = circley + ballspeedy
circlex = circlex + ballspeedx

--opponent AI paddle movement
if circley <= 544-130 and circley >= 10 then
if math.random(1, 100) >= 30 then
if y2 < circley then
y2 = math.range(y2+2, 10, 544-130)
elseif y2 > circley then
y2 = math.range(y2-2, 10, 544-130)
end
end
end

--controls
pad = Controls.read()
if Controls.check(pad, SCE_CTRL_DOWN) and Controls.check(oldpad, SCE_CTRL_DOWN) and y1 <= 544-130 then y1 = y1+2 end
if Controls.check(pad, SCE_CTRL_UP) and Controls.check(oldpad, SCE_CTRL_UP) and y1 >= 10 then y1 = y1-2 end
if not Controls.check(pad, SCE_CTRL_START) and Controls.check(oldpad, SCE_CTRL_START) then break end
oldpad = pad

end
