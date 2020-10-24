require "host"
require "extras"

Glow{ id="root",
  sRGB=true,
  image="main:color",
  blur=Kawase{ intensity=2, taskSize=16, input="main:half" }}

Buffers{ id="main",
  color=true,
  half=true,
  gpu=GPU{
    targetSize="globals:windowSize",
    layers={Layer{
      color=Vec3(0.222, 0.222, 0.333),
      camera=Perspective{ position=Vec3(-18.5, 8.5, 44.0),
                          h=3.0, v=-0.4, fov=45.0, originX=0 },
      gl=Modify{
        rotate=T3("{pow(sin(t*0.25), 100)*0.05, t*0.005, 0}"),
        gl=Many{ material=Material{program="Many"},
                 mesh="mycube.obj" }}},}}}
