#include "points.h"


int square_distance(CvPoint pt1, CvPoint pt2)
{
	int dx = pt2.x - pt1.x;
	int dy = pt2.y - pt1.y;
	return dx * dx + dy * dy;
}


CvPoint point_on_line(CvPoint pt1, CvPoint pt2, double u)
{
	int dx = pt2.x - pt1.x;
	int dy = pt2.y - pt1.y;
	CvPoint result;
	result.x = pt1.x + cvRound(dx * u);
	result.y = pt1.y + cvRound(dy * u);
	return result;
}


bool point_in_image(IplImage *image, CvPoint point)
{
	return (point.x >= 0 && point.x < image->width &&
			point.y >= 0 && point.y < image->height);
}


double point_distance(double x1, double y1, double x2, double y2)
{
	double xd = x2 - x1;
	double yd = y2 - y1;
	return sqrt(xd * xd + yd * yd);
}


double point_line_distance(double x1, double y1,
						   double x2, double y2, double x3, double y3)
{
	double x12 = x2 - x1;
	double y12 = y2 - y1;
	double x13 = x3 - x1;
	double y13 = y3 - y1;
	double u1 = x13 * x12 + y13 * y12;
	double u2 = x12 * x12 + y12 * y12;
	double u = u1 / u2;
	double xu = x1 + u * x12;
	double yu = y1 + u * y12;
	double xd = xu - x3;
	double yd = yu - y3;
	double d = sqrt(xd * xd + yd * yd);
	// printf("p1=%.1f,%.1f p2=%.1f,%.1f p3=%.1f,%.1f ", x1, y1, x2, y2, x3, y3);
	// printf("u=%.3f pu=%.1f,%.1f d=%.4f\n", u, xu, yu, d);
	return d;
}
