Truecolor{ id="root",
  sRGB=false,
  program="IQ",
  gpu=GPU{ targetSize="globals:windowSize",
           layers={"foo", "board"} }}

ppp = Particles{
  many=2048,
  rate=0.01,
  life=8.00,
  gravity=-0.0001, -- -00.005,
  gl=Group{
    scale=Float(0.3),
    gl=Plane{ material=Material{ program="Amy", texture0=Image('data/texture/among240.png')  } }}}

mc = Metaballs{
  particles=ppp,
  precision=64,
  scale=10.0,
  forkDepth=3 }

Layer{ id="foo",
  camera="uiCamera",
  color=Vec3(0.2,0.3,0.3),
  gl=Group{
    translate=Vec3(0, 5, 0),
    gl={ ppp, mc }}}

--[[      many=8192,
      gravity=-0.1635/300,
      gl=Group{ scale=Float(0.3), gl=Plane{ material=Material{ program="Wireframe" } }}}}}
      ]]--

Layer{ id="board",
  camera="uiCamera",
  gl=Mesh{ name="board10m.obj" }}

