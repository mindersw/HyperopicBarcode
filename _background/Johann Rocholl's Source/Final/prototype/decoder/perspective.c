#include <math.h>
#include <stdio.h>

#include "perspective.h"
#include "points.h"


double perspective_distance(double angle_of_view, CvSize size,
							CvPoint pt1, CvPoint pt2)
{
	double view_size = point_distance(0, 0, size.width, size.height);
	double barcode_size = point_distance(pt1.x, pt1.y, pt2.x, pt2.y);
	double angle_of_barcode = angle_of_view / view_size * barcode_size;
	return 1.0 / tan(angle_of_barcode / 2.0);
}


double perspective_angle(CvPoint tl, CvPoint tr, CvPoint bl, CvPoint br,
						 double distance)
{
	double lx = (tl.x + bl.x) / 2.0;
	double ly = (tl.y + bl.y) / 2.0;
	double l = (point_line_distance(tl.x, tl.y, tr.x, tr.y, lx, ly) +
				point_line_distance(bl.x, bl.y, br.x, br.y, lx, ly));
	if (l < 5.0)
		return 0.0;
	double rx = (tr.x + br.x) / 2.0;
	double ry = (tr.y + br.y) / 2.0;
	double r = (point_line_distance(tl.x, tl.y, tr.x, tr.y, rx, ry) +
				point_line_distance(bl.x, bl.y, br.x, br.y, rx, ry));
	if (r < 5.0)
		return 0.0;
	double ratio = l / r;
	double loga = 1.2 * log(ratio);
	double angle = atan(loga);
	printf("distance=%.2f l=%.2f r=%.2f ratio=%.5f loga=%.5f angle=%.2f\n",
		   distance, l, r, ratio, loga, angle * 180 / CV_PI);
	return angle;
}
