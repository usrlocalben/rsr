require "extras"

Truecolor{ id="root",
  sRGB=true, program="Default",
  gpu=GPU{ targetSize="globals:windowSize",
           layers={"lEffects"}}}

Layer{ id="lEffects",
  camera=Perspective{ position=Vec3(88, 80, 93),
                      h=3.72, v=-0.35,
                      fov=45.0 },
  color=sRGB(128,128,128),
  gl=Mesh{ --material=Material{ program="Wireframe" },
           name="colortest.obj" }}
