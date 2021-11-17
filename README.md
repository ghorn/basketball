# Why?

There are two purposes to this repository:

1. My implementation of an optimally-shaped basketball backboard that directs all shots into the hoop.
Inspired by (stolen from) https://www.youtube.com/watch?v=vtN4tkvcBMA.
The backboard is modeled as a b-spline with control points. Shots are taken from a grid on the court and
bounce on a grid of points on the backboard. The number of shots that go in the hoop is maximized by
optimizing the b-spline control point locations.

2. Learning how to write modern OpenGL using shader pipelines. This is the real reason I did this.

# Running
The build system is bazel, you might be able to run:

>  bazel run //:vis

Unfortunately it's not hermetic, there are some system dependencies on OpenGL, NLOPT, GLEW and more.
TODO(greg): list of deps

If you don't have bazel installed, you're on Linux, and you are brave enough to run binaries from strangers,
consider trying this command:

>  ./bazelisk-linux-amd64 run //:vis

The bazelisk program downloads and installs bazel and forwards all arguments to it.
A bazelisk binary is committed to this repo.

# Results
It should look something like this:

![alt text](https://github.com/ghorn/basketball/blob/main/readme/screenshot1.png?raw=true)

where the backboard changes shape as the optimization is run.

Try pressing "s", "b", "c", "h", "p", "n", "t", "w" to see more! For example:

![alt text](https://github.com/ghorn/basketball/blob/main/readme/screenshot2.png?raw=true)
