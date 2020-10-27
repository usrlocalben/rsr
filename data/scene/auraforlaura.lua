require "extras"

SHAKE_AMP = Float(0)
SHAKE_SPEED = Float(1)
SCALE = Float(9.0)
SPIKINESS = Float(0.05)
T = "globals:wallclock"

local function Linear(s)
  return math.pow(s/255.0, 2.2333)
end

local function sRGB(r, g, b)
  return Vec3(Linear(r), Linear(g), Linear(b))
end

Truecolor{ id='root',
  sRGB=true,
  program="Default",
  gpu=GPU{
    targetSize="globals:windowSize",
    layers={"lAmy", "lAuraForLaura"}}}

Layer{ id="lAmy",
  camera=Orthographic(0, 16, 0, 9),
  color=sRGB(238, 248, 255),
  gl=Group{ id="nAmy",
    translate=Vec3(12.5, 4.5, 0.999),
    scale=Vec3(9.0, 9.0, 1.0),
    gl=Plane{ material=Material{
      program="Amy",
      filter=true,
      texture0=Image("data/texture/amy1024.png") }}}}

ft = Mul_Float_Float(T, SHAKE_SPEED)
shake = Mul_Vec3_Float(
  Vec3{ x=Noise(Vec2(0, ft)),
        y=Noise(Vec2(0.2348792, ft)),
	z=Noise(Vec2(0.4718937, ft)) },
  SHAKE_AMP)

Layer{ id="lAuraForLaura",
  camera=Perspective{position=Vec3(0,0,24),
                     h=3.14, v=0, fov=55.0,
                     originX=-0.425},
  gl=Group{
    gl=AuraForLauraMT{
      material=Material{
        program="Envmap", filter=false,
        texture0=Image("data/texture/env-purple.png") },
      freq=T3("{(pow(sin(t*0.5),7))*10+15.0, sin(t*0.25)*15+10, 0}"),
      phase=T3("{t*1, t*5.0, 0}"),
      amp=SPIKINESS},
    translate=shake,
    scale=SCALE,
    rotate=T3("{0, sin(t*0.5)*0.2+0.2, sin(t*0.222)*0.1}")}}
