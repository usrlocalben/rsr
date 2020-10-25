require "extras"

BW = 0.1  -- border width
RTT_DIM = Vec2(256, 256)
RTT_AA = true

Truecolor{ id="root",
  sRGB=false, program="IQ",
  gpu=GPU{ targetSize="globals:windowSize",
           layers={ "pipLayer" } }}

Layer{ id="pipLayer",
  camera=Orthographic(0, 16, 0, 9),
  color=sRGB(50, 100, 100),
  gl={ "pipBorder", "pipImage" }}


Modify{ id="pipBorder",
  scale=Vec3(7+(BW*2), 7+(BW*2), 1),
  translate=Vec3(7+1, 3.5+1, 0.2),
  gl=Plane{ material="blackEnvmap" }}

Modify{ id="pipImage",
  scale=Vec3(7, 7, 1),
  translate=Vec3(7+1, 3.5+1, 0.1),
  gl=Plane{ material=Material{
    program="Amy", filter=false,
    texture0=RenderToTexture{
      gpu=GPU{
        targetSize=RTT_DIM,
        aspect=Float(1.0),
        aa=RTT_AA,
        layers={ Layer{
          color=Float(0.333),
          camera="uiCamera",
          gl=Modify{
            scale=Float(1.0),
            rotate=T3("{ t*0.1, sin(t*0.41)*0.333, sin(t*0.222)*0.3}"),
            gl=MC{
              material="blackEnvmap",
              frob="globals:wallclock",
              precision=32,
              range=3.2,
              forkDepth=3 }}}, }}}}}}

Material{ id="blackEnvmap",
  program="Envmap", filter=false,
  texture0=Image("data/texture/env-black.png") }
