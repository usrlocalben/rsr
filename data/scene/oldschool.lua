require "extras"

W=320 H=240 BAR=9 HW=W/2 HH=H/2

local function OrthoCentered4x3On16x9(widthInPx, heightInPx)
  local deviceWidthInPx = heightInPx*16/9
  local unused = deviceWidthInPx - widthInPx
  local offset = unused/2
  local rightEdge = deviceWidthInPx - offset
  return Orthographic(-offset, rightEdge, 0, heightInPx)
end

Truecolor{ id="root",
  sRGB=true, program="Default",
  gpu=GPU{
    targetSize="globals:windowSize",
    layers={ Layer{
      camera=OrthoCentered4x3On16x9(W, H),
      --camera=Orthographic(0, W, 0, H),
      color=sRGB(238, 248, 255),
      gl=Group{ gl={ "background", "rasterBars" } }}}}}

Group{ id="rasterBars", gl=(function ()
  local barScale = Vec2(BAR, H)
  local bars = {}
  for y = 1, H do
    local anim1 = "sin((t*2.19343) + "..y.."*(sin(t*0.25)*0.05) * 3.14159*2)*(sin(t*1.781234)*150)"
    local anim2 = "sin((t)+"..y.."*0.005*3.14159*2)*(sin(t*3.781234)*150)"
    local anim3 = "sin((t*5.666)+"..y.."*0.008*3.14159*2)*(sin(t*1.781234+("..y.."*0.01))*50+100)+sin(("..y.."+(T*60))*0.00898)*100"
    local anim4 = "sin((t*2.45)+"..y.."*0.012*3.14159*2)*(sin(t*1.781234+("..y.."*0.01))*66+33)+sin(("..y.."+(T*60))*0.01111)*100-50"

    local fx = anim4.."+"..HW
    fx = "floor("..fx..")"  -- remove for "HD" look
    local fy = HH - (y-1)
    local fz = -y/H  -- (0, -1]

    -- must allocate unique nodes below here because
    -- the tool can't handle more than 16 backlinks
    table.insert(bars, Group{
      translate=T3("{"..fx..", "..fy..", "..fz.."}"),
      scale=barScale,
      gl=Plane{ material=Material{
        program="Amy", filter=false,
        texture0=Image("data/texture/bar2.png") } }})
  end
  return bars end)() }

Group{ id="background",
  translate=Vec3(W/2, H/2, 0.999), scale=Vec2(W, H),
  gl=Plane{ material=Material{
    program="Amy", filter=false,
    texture0=Image("data/texture/amy.png") }}}
