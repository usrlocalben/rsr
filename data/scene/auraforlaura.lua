require "extras"

NOISE_AMP = Float(0)
SCALE = Float(9.0)
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
  gl=Modify{ id="nAmy",
    translate=Vec3(12.5, 4.5, 0.999),
    gl=Modify{
      scale=Vec3(9.0, 9.0, 1.0),
      gl=Plane{ material=Material{
        program="Amy",
        filter=true,
        texture0=Image("data/texture/amy1024.png") }}}}}

Layer{ id="lAuraForLaura",
  camera=Perspective{position=Vec3(0,0,24),
                     h=3.14, v=0, fov=55.0,
                     originX=-0.425},
  gl=Modify{
    gl=AuraForLauraMT{
      material=Material{
        program="Envmap",
        filter=false,
        texture0=Image("data/texture/env-purple.png") },
      freq=T3("{(pow(sin(t*0.5),7))*10+15.0, sin(t*0.25)*15+10, 0}"),
      phase=T3("{t*1, t*5.0, 0}"),
      amp=Float(0.05)},
    translate=ComputedVec3{
      inputs={{ name="amp", type="real", source=NOISE_AMP },
              { name="nx", type="real", source="auraPositionNoiseX" },
              { name="ny", type="real", source="auraPositionNoiseY" },
              { name="nz", type="real", source="auraPositionNoiseZ" },},
      code="{nx*amp, ny*amp, nz*amp}"},
    scale=SCALE,
    rotate=T3('auraRotation', "{0, sin(t*0.5)*0.2+0.2, sin(t*0.222)*0.1}")}}

Noise{ id="auraPositionNoiseX", coord=Vec3(0, T, 0) }
Noise{ id="auraPositionNoiseY", coord=Vec3(0.2348792, T, 0) }
Noise{ id="auraPositionNoiseZ", coord=Vec3(0.4718937, T, 0) }
