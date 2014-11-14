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

The animation control buttons allow you to toggle whether or not the animation is active,
and allow you to pause the animation.

The dropdown for cylinder shape allows you to choose which shape is drawn along the frenet frames.
Note: Custom cross section drawing is available under the Cylinder Shape Dropdown. 
When selecting "Custom" a new window is opened. 
The controls in this window are:
	Left Click and drag to move points.
	Right click to place a point with a line to the previous point.
	Middle click to place a point without a line to the previous point. This is useful when drawing multiple shapes.


Data Structures used:
The only significant data structure used in this assignment are QVectors of QVector3Ds.
Specifically, the QVector containing my cross-section co-ordinates is interesting because
it contains QVector3Ds but refers to 2D points in the cross-section. The z-value on these
points refers to whether or not there is a line drawn between the previous point or not. 
This allows for complex shapes.

The algorithms of note are the catmull rom algorithm, the ray picking used to select points,
and the mapping from window coordinates to world coordinates, which is performed mostly through OpenGL functions.