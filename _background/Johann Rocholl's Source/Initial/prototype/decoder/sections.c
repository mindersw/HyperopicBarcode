#include <math.h>
#include <stdio.h>

#include "sections.h"


double p(double distance, double angle, double u)
{
	double cos_angle = cos(angle);
	double sin_angle = sin(angle);
	return (distance * u * cos_angle) / (distance + u * sin_angle);
}


double q(double distance, double angle, double u)
{
	double cos_angle = cos(angle);
	double sin_angle = sin(angle);
	double p_u = (distance * u * cos_angle) / (distance + u * sin_angle);
	double p_left = (distance * -1 * cos_angle) / (distance - sin_angle);
	double p_right = (distance * 1 * cos_angle) / (distance + sin_angle);
	return (p_u - p_left) / (p_right - p_left);
}


void calculate_sections(int count, int *sections,
						int start, int stop, double angle, double distance)
{
	double cos_angle = cos(angle);
	double sin_angle = sin(angle);
	double p_left = (distance * -1 * cos_angle) / (distance - sin_angle);
	double p_right = (distance * 1 * cos_angle) / (distance + sin_angle);
	double p_range = p_right - p_left;
	double inside = (double) (stop - start);
	for (int i = 0; i < count; i++) {
		double u = -1.0 + 2.0 * i / (count - 1);
		double p_u =
			(distance * u * cos_angle) / (distance + u * sin_angle);
		double q_u = (p_u - p_left) / p_range;
		sections[i] = start + cvRound(q_u * inside);
		// printf("u=%.6f p_u=%.6f q_u=%.6f sections[%d]=%d\n",
		//        u, p_u, q_u, i, sections[i]);
	}
}


void draw_sections(int count, int *sections, int width,
				   IplImage *debug, CvScalar color, int thickness)
{
	for (int i = 0; i <= count; i++) {
		if (i > 3 && i < 45 && (i - 3) % 7)
			continue;
		if (i > 50 && i < 92 && (i - 50) % 7)
			continue;
		double px = (double) sections[i] / width;
		int x = cvRound(debug->width * px);
		CvPoint pt1 = cvPoint(x, 0);
		CvPoint pt2 = cvPoint(x, debug->height - 1);
		cvLine(debug, pt1, pt2, color, thickness, 8, 0);
	}
}
