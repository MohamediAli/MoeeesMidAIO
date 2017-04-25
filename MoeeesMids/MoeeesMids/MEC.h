#pragma once
#define NOMINMAX
#include <iostream>
#include "math.h"
#include <limits>
#include "PluginSDK.h"

struct RectangleF
{
	RectangleF (float x, float y, float w, float h)
	{
		xyzw[0] = x;
		xyzw[1] = y;
		xyzw[2] = w;
		xyzw[3] = h;
	}
	float xyzw[4];
};

/// <summary>
///     Represents a MecCircle
/// </summary>
static struct MecCircle
{
public:
	/// <summary>
	///     The center
	/// </summary>
	Vec2 Center;

	/// <summary>
	///     The radius
	/// </summary>
	float Radius;
	/// <summary>
	///     Initializes a new instance of the <see cref="MecCircle" /> struct.
	/// </summary>
	/// <param name="center">The center.</param>
	/// <param name="radius">The radius.</param>
	MecCircle (Vec2 center, float radius)
	{
		this->Center = center;
		this->Radius = radius;
	}
};

static class MEC
{

public:
	/*static RectangleF g_MinMaxBox;
	static std::vector<Vec2> g_MinMaxCorners;
	static std::vector<Vec2> g_NonCulledPoints;*/

	static void FindMinimalBoundingCircle (std::vector<Vec2> points, Vec2 &center, float &radius)
	{
		// Find the convex hull.
		auto hull = MakeConvexHull (points);
		// The best solution so far.
		auto best_center = points[0];
		float best_radius2 = FLT_MAX;
		// Look at pairs of hull points.
		for (auto i = 0; i < hull.size() - 1; i++)
			{
			for (auto j = i + 1; j < hull.size(); j++)
				{
				// Find the circle through these two points.
				Vec2 test_center = Vec2 ( (hull[i].x + hull[j].x) / 2.0, (hull[i].y + hull[j].y) / 2.0);
				auto dx = test_center.x - hull[i].x;
				auto dy = test_center.y - hull[i].y;
				auto test_radius2 = dx * dx + dy * dy;
				// See if this circle would be an improvement.
				if (test_radius2 < best_radius2)
					{
					// See if this circle encloses all of the points.
					if (CircleEnclosesPoints (test_center, test_radius2, points, i, j, -1))
						{
						// Save this solution.
						best_center = test_center;
						best_radius2 = test_radius2;
						}
					}
				} // for i
			} // for j
		// Look at triples of hull points.
		for (auto i = 0; i < hull.size() - 2; i++)
			{
			for (auto j = i + 1; j < hull.size() - 1; j++)
				{
				for (auto k = j + 1; k < hull.size(); k++)
					{
					// Find the circle through these three points.
					Vec2 test_center;
					float test_radius2 = 0;
					FindCircle (hull[i], hull[j], hull[k], test_center, test_radius2);
					// See if this circle would be an improvement.
					if (test_radius2 < best_radius2)
						{
						// See if this circle encloses all of the points.
						if (CircleEnclosesPoints (test_center, test_radius2, points, i, j, k))
							{
							// Save this solution.
							best_center = test_center;
							best_radius2 = test_radius2;
							}
						}
					} // for k
				} // for i
			} // for j
		center = best_center;
		if (best_radius2 == FLT_MAX)
			{
			radius = 0;
			}
		else
			{
			radius = (float) sqrt (best_radius2);
			}
	}

	static MecCircle GetMec (std::vector<Vec2> points)
	{
		auto center = Vec2();
		float radius;
		auto ConvexHull = MakeConvexHull (points);
		FindMinimalBoundingCircle (ConvexHull, center, radius);
		return MecCircle (center, radius);
	}

	/// <summary>
	///     Makes the convex hull.
	/// </summary>
	/// <param name="points">The points.</param>
	/// <returns>Points that make up a polygon's convex hull..</returns>
	static std::vector<Vec2> MakeConvexHull (std::vector<Vec2> points)
	{
		// Cull.
		points = HullCull (points);
		// Find the remaining point with the smallest Y value.
		// if (there's a tie, take the one with the smaller X value.
		Vec2 best_pt[] = { points[0] };
		for (int i = 0; i < points.size(); i++)
			{
			auto pt = points.at (i);
			if ( (pt.y < best_pt[0].y || pt.y == best_pt[0].y) && pt.x < best_pt[0].x)
				{
				best_pt[0] = pt;
				}
			}
		// Move this point to the convex hull.
		std::vector<Vec2> hull = std::vector<Vec2> { best_pt[0] };
		int index = 0;
		for (int i = 0; i < points.size(); i++)
			{
			if (points.at (i) == best_pt[0])
				{
				index = i;
				break;
				}
			}
		points.erase (points.begin() + index);
		// Start wrapping up the other points.
		float sweep_angle = 0;
		for (;;)
			{
			// If all of the points are on the hull, we're done.
			if (points.size() == 0)
				{
				break;
				}
			// Find the point with smallest AngleValue
			// from the last point.
			auto X = hull[hull.size() - 1].x;
			auto Y = hull[hull.size() - 1].y;
			best_pt[0] = points[0];
			float best_angle = 3600;
			// Search the rest of the points.
			for (int i = 0; i < points.size(); i++)
				{
				auto pt = points[i];
				auto test_angle = AngleValue (X, Y, pt.x, pt.y);
				if ( (test_angle >= sweep_angle) && (best_angle > test_angle))
					{
					best_angle = test_angle;
					best_pt[0] = pt;
					}
				}
			// See if the first point is better.
			// If so, we are done.
			auto first_angle = AngleValue (X, Y, hull[0].x, hull[0].y);
			if ( (first_angle >= sweep_angle) && (best_angle >= first_angle))
				{
				// The first point is better. We're done.
				break;
				}
			for (int i = 0; i < points.size(); i++)
				{
				if (points.at (i) == best_pt[0])
					{
					index = i;
					break;
					}
				}
			// Add the best point to the convex hull.
			hull.push_back (best_pt[0]);
			points.erase (points.begin() + index);
			sweep_angle = best_angle;
			}
		return hull;
	}

	/// <summary>
	///     Return a number that gives the ordering of angles
	///     WRST horizontal from the point(x1, y1) to(x2, y2).
	///     In other words, AngleValue(x1, y1, x2, y2) is not
	///     the angle, but if:
	///     Angle(x1, y1, x2, y2) > Angle(x1, y1, x2, y2)
	///     then
	///     AngleValue(x1, y1, x2, y2) > AngleValue(x1, y1, x2, y2)
	///     this angle is greater than the angle for another set
	///     of points,) this number for
	///     This function is dy / (dy + dx).
	/// </summary>
	/// <param name="x1">The x1.</param>
	/// <param name="y1">The y1.</param>
	/// <param name="x2">The x2.</param>
	/// <param name="y2">The y2.</param>
	/// <returns>A number that gives the ordering of angles</returns>
	static float AngleValue (float x1, float y1, float x2, float y2)
	{
		float t;
		auto dx = x2 - x1;
		auto ax = abs (dx);
		auto dy = y2 - y1;
		auto ay = abs (dy);
		if (ax + ay == 0)
			{
			// if (the two points are the same, return 360.
			t = 360.0 / 9.0;
			}
		else
			{
			t = dy / (ax + ay);
			}
		if (dx < 0)
			{
			t = 2 - t;
			}
		else if (dy < 0)
			{
			t = 4 + t;
			}
		return t * 90;
	}

	/// <summary>
	///     Encloses the points in a circle.
	/// </summary>
	/// <param name="center">The center.</param>
	/// <param name="radius2">The radius2.</param>
	/// <param name="points">The points.</param>
	/// <param name="skip1">The skip1.</param>
	/// <param name="skip2">The skip2.</param>
	/// <param name="skip3">The skip3.</param>
	/// <returns><c>true</c> if the indicated circle encloses all of the points, <c>false</c> otherwise.</returns>
	static bool CircleEnclosesPoints (
	    Vec2 center,
	    float radius2,
	    std::vector<Vec2> points,
	    int skip1,
	    int skip2,
	    int skip3)
	{
		std::vector<Vec2> result;
		for (int i = 0; i < points.size(); i++)
			{
			auto pt = points.at (i);
			auto t = pt.x;
			auto y = pt.y;
			float test_radius = center.x;
			if (test_radius < radius2)
				{
				if ( (i != skip1) && (i != skip2) && (i != skip3))
					{
					auto dx = center.x - pt.x;
					auto dy = center.y - pt.y;
					test_radius = dx * dx + dy * dy;
					}
				}
			else
				{
				return false;
				}
			}
		return true;
	}

	/// <summary>
	///     Finds the circle through the three points.
	/// </summary>
	/// <param name="a">a.</param>
	/// <param name="b">The b.</param>
	/// <param name="c">The c.</param>
	/// <param name="center">The center.</param>
	/// <param name="radius2">The radius2.</param>
	static void FindCircle (Vec2 a, Vec2 b, Vec2 c, Vec2 center, float radius2)
	{
		// Get the perpendicular bisector of (x1, y1) and (x2, y2).
		auto x1 = (b.x + a.x) / 2;
		auto y1 = (b.y + a.y) / 2;
		auto dy1 = b.x - a.x;
		auto dx1 = - (b.y - a.y);
		// Get the perpendicular bisector of (x2, y2) and (x3, y3).
		auto x2 = (c.x + b.x) / 2;
		auto y2 = (c.y + b.y) / 2;
		auto dy2 = c.x - b.x;
		auto dx2 = - (c.y - b.y);
		// See where the lines intersect.
		auto cx = (y1 * dx1 * dx2 + x2 * dx1 * dy2 - x1 * dy1 * dx2 - y2 * dx1 * dx2) / (dx1 * dy2 - dy1 * dx2);
		auto cy = (cx - x1) * dy1 / dx1 + y1;
		center = Vec2 (cx, cy);
		auto dx = cx - a.x;
		auto dy = cy - a.y;
		radius2 = dx * dx + dy * dy;
	}

	// Find a box that fits inside the MinMax quadrilateral.
	/// <summary>
	///     Gets the minimum maximum box.
	/// </summary>
	/// <param name="points">The points.</param>
	/// <returns>RectangleF.</returns>
	static RectangleF GetMinMaxBox (std::vector<Vec2> points)
	{
		// Find the MinMax quadrilateral.
		Vec2 ul = Vec2 (0, 0), ur = ul, ll = ul, lr = ul;
		GetMinMaxCorners (points, ul, ur, ll, lr);
		// Get the coordinates of a box that lies inside this quadrilateral.
		auto xmin = ul.x;
		auto ymin = ul.y;
		auto xmax = ur.x;
		if (ymin < ur.y)
			{
			ymin = ur.y;
			}
		if (xmax > lr.x)
			{
			xmax = lr.x;
			}
		auto ymax = lr.y;
		if (xmin < ll.x)
			{
			xmin = ll.x;
			}
		if (ymax > ll.y)
			{
			ymax = ll.y;
			}
		auto result = RectangleF (xmin, ymin, xmax - xmin, ymax - ymin);
		//g_MinMaxBox = result; // For debugging.
		return result;
	}

	// Find the points nearest the upper left, upper right,
	// lower left, and lower right corners.
	/// <summary>
	///     Gets the minimum maximum corners.
	/// </summary>
	/// <param name="points">The points.</param>
	/// <param name="ul">The ul.</param>
	/// <param name="ur">The ur.</param>
	/// <param name="ll">The ll.</param>
	/// <param name="lr">The lr.</param>
	static void GetMinMaxCorners (std::vector<Vec2> points, Vec2 ul,
	                              Vec2 ur1, Vec2 ll, Vec2 lr)
	{
		// Start with the first point as the solution.
		ul = points[0];
		ur1 = ul;
		ll = ul;
		lr = ul;
		// Search the other points.
		for (int i = 0; i < points.size(); i++)
			{
			auto  pt = points[i];
			if (-pt.x - pt.y > -ul.x - ul.y)
				{
				ul = pt;
				}
			if (pt.x - pt.y > ur1.x - ur1.y)
				{
				ur1 = pt;
				}
			if (-pt.x + pt.y > -ll.x + ll.y)
				{
				ll = pt;
				}
			if (pt.x + pt.y > lr.x + lr.y)
				{
				lr = pt;
				}
			}
		//for debugging
		/*g_MinMaxCorners[0] = ul;
		g_MinMaxCorners[1] = ur1;
		g_MinMaxCorners[2] = lr;
		g_MinMaxCorners[3] = ll;*/
	}


	/// <summary>
	///     Culls points out of the convex hull that lie inside the trapezoid defined by the vertices with smallest and largest
	///     X and Y coordinates.
	/// </summary>
	/// <param name="points">The points.</param>
	/// <returns>Points that are not culled.</returns>
	static std::vector<Vec2> HullCull (std::vector<Vec2> points)
	{
		// Find a culling box.
		auto culling_box = GetMinMaxBox (points);
		// Cull the points.
		std::vector<Vec2> results;
		for (int i = 0; i < points.size(); i++)
			{
			auto pt = points.at (i);
			if (pt.x <= culling_box.xyzw[0] || pt.x >= culling_box.xyzw[1]
			        || pt.y <= culling_box.xyzw[2]
			        || pt.y >= culling_box.xyzw[3])
				{ results.push_back (pt); }
			}
		// For debugging.
		/*g_NonCulledPoints[results.size()];
		for (int i = 0; i < results.size(); i++)
		{
		g_NonCulledPoints[i] = results.at(i);
		}*/
		return results;
	}
};