Note: Control information is available in the app under the help button, but will be detailed here as well.

Dylan Gedig - A2 Three Dimensional Catmull Rom Splines

I eschewed a multi-window view in favour of a more intuitive single window approach. This allows you to create points
and then later move them around. The points are given a predetermined depth from the camera when first placed,
as there is no easy way to define that. When selecting or deleting points later on, ray picking is used to affect the appropriate
point.

Controls: 

Clicking and holding the left mouse button allows you to rotate the camera.
Clicking a point with the left mouse button selects it, allowing you to move it.

Clicking and holding the right mouse button allows you to zoom.
Clicking a point with the right mouse button deletes the point.

The middle mouse button is used to create points.


Data Structures used:
The only significant data structure used in this assignment is a QVector of QVector3Ds.
The algorithms of note are the catmull rom algorithm, the ray picking used to select points,
and the mapping from window coordinates to world coordinates, which is performed mostly through OpenGL functions.