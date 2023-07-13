# raymarcher
## A C and GTK/Cairo port of [raihan142857's scratch ray marcher](https://scratch.mit.edu/projects/403811864)
I thought that project was neat, but scratch was slow in rendering high resolution models, so I made this thing to experiment with GTK and Cairo.

This is an obvious and sloppy port, but it works, with one caveat

### Known Issue
It drives me nuts, but if I set the condition for the brightness to `raylength > len`, it looks awful, which is fixed by setting the condition to `raylength > len - 1`, but that causes the edges of the shadows to be bright (which can be mitigated by making the `res` variable insanely low, but that takes a while to render)
