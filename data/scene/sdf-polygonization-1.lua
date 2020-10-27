require "extras"

Truecolor{ id="root",
  sRGB=true, program="Default",
  gpu=GPU{ targetSize="globals:windowSize",
           layers={ "foo", } }}

Layer{ id="foo",
  color=Float(0.333),
  camera="uiCamera",
  gl=Group{
    scale=Float(1.0),
    rotate=T3("{t*0.1, sin(t*0.41)*0.333, sin(t*0.222)*0.3}"),
    gl=MC{
      material=Material{
        program="Envmap", filter=false,
        texture0=Image("data/texture/env-black.png")},
      frob="globals:wallclock",
      precision=32,
      range=3.2,
      forkDepth=3 }}}
