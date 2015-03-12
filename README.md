3D Slicer repository for modules implemented by the CSIM research group.

Assuming the source code of your module is located in folder <code>MyModule</code>, externally building modules could be achieved doing:
<pre>
$ mkdir MyModule-build
$ cd MyModule-build
$ cmake -DSlicer_DIR:PATH=/path/to/Slicer-Superbuild/Slicer-build ../MyModule
$ make
</pre>
