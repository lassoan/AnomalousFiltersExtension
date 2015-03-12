3D Slicer repository for modules implemented by the CSIM research group.

= Directions to build the modules =

Assuming the source code of your module is located in folder <code>CSIMModule</code>, externally building modules could be achieved doing:
<pre>
$ mkdir CSIMModule-build
$ cd CSIMModule-build
$ cmake -DSlicer_DIR:PATH=/path/to/Slicer-Superbuild/Slicer-build ../CSIMModule
$ make
</pre>
