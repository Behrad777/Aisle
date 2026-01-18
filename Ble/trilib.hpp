#pragma once
#include <array>
#include <cmath>

//for precise double default or floating point coordinates as well as integers
template<typename T>
struct Point {
    T x;
    T y;
};

template<typename T>
class Trilateration {
public:
    // RSSI-based trilateration for exactly 3 anchors
    // anchors: array of 3 positions
    // dist: array of 3 distances
    // out: computed (x,y). Returns false on degenerate configuration (should never happen in given coordinate system)

    static int solveRSSI(const std::array<Point<T>,3>& anchors, const std::array<T,3>& dist, Point<T>& out) {
        // Build linear system for circles 2 and 3 minus circle 1
        T x1 = anchors[0].x, y1 = anchors[0].y;
        T x2 = anchors[1].x, y2 = anchors[1].y;
        T x3 = anchors[2].x, y3 = anchors[2].y;
        T r1 = dist[0], r2 = dist[1], r3 = dist[2];

        T A11 = 2*(x2 - x1);
        T A12 = 2*(y2 - y1);
        T B1  = (x2*x2 - x1*x1) + (y2*y2 - y1*y1) - (r2*r2 - r1*r1);

        T A21 = 2*(x3 - x1);
        T A22 = 2*(y3 - y1);
        T B2  = (x3*x3 - x1*x1) + (y3*y3 - y1*y1) - (r3*r3 - r1*r1);

        T det = A11 * A22 - A12 * A21;
        if (std::fabs(det) < 1e-6) return -1; // anchors colinear or bad input

        out.x = ( B1 * A22 - A12 * B2) / det;
        out.y = ( A11 * B2 - B1 * A21) / det;
        return 0;
    }
};
