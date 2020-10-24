Truecolor{ id="root",
  sRGB=false,
  program="IQ",
  gpu=GPU{ targetSize="globals:windowSize",
           layers={"writerTest",} }}

Layer{ id="writerTest",
  camera=Orthographic{l=0, r=16.0, b=-9, t=0},
  --camera="uiCamera",
  color=Vec3(0.2,0.3,0.3),
  gl=Modify{
    translate=Vec3(4, -1, 0),
    gl=Writer{
      material=Material{
        program="Text",
        filter=true,
        alpha=true,
        texture0=Image("data/font/cmu_serif_roman_48.png")},
      leading=0.70,
      --tracking=0.70,
      width=8.0,
      color=Vec3(1.00, 1.0, 1.0),
      font="data/font/cmu_serif_roman_48.lua",
      text=String("We are lucky to live in a glorious age that "..
                  "gives us everything we could ask for as a "..
                  "human race. What more could you need when you "..
                  "have meat covered in cheese nestled between "..
                  "bread as a complete meal.")}}}
