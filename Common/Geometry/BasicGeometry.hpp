#pragma once
#include <Eigen/Dense>
#include <vector>

namespace geometry {
    template <typename T>
    using Vector3 = Eigen::Matrix<T, 3, 1>;

    // Norm of a vector
    template <typename T>
    T norm(T a, T b, T c) {
        return sqrt(pow(a, 2) + pow(b, 2) + pow(c, 2));
    }

    // the plane is represented by (x - _p) /dot _normal = 0
    template <typename T>
    class Plane {
    public:
        Plane(Vector3<T> p, Vector3<T> normal) {
            _p = p;
            _normal = normal;
            _normal.normalize();
        }

        Vector3<T>& p() { return _p; }
        Vector3<T>& normal() { return _normal; }
        
        // return if the point is on plane
        // also fill parameter dist as the signed distance from point to plane
        bool onPlane(Vector3<T> point, T& dist) {
            dist = (point - _p).dot(_normal);
            if (std::fabs(dist) < 1e-6) {
                return true;
            } else {
                return false;
            }
        }

    private:
        Vector3<T> _p;
        Vector3<T> _normal;
    };

    

    template <typename T>
    class Triangle {
    public:
        Triangle(Vector3<T> v0, Vector3<T> v1, Vector3<T> v2) {
            _vertices[0] = v0;
            _vertices[1] = v1;
            _vertices[2] = v2;
        }

        Vector3<T>* vertices() { return _vertices; }
        Vector3<T>& vertices(int idx) { return _vertices[idx]; }

        
        // TODO: HW1
        // part 2.1
        // Implement the function to do intersection between triangle and plane p
        // Input: plane p
        // Output: intersections points with three edges
        // Hint:
        //      - enumerate three edges of the triangle and do intersection individually
        //      - consider the case that no intersection
        //      - consider how to avoid repeated intersection points in returned list
        std::vector<Vector3<T>> IntersectPlane(Plane<T> p) {
            std::vector<Vector3<T>> intersections;
            intersections.clear();
            bool op0, op1, op2;
            T d0, d1, d2;
            if (op0 = p.onPlane(vertices(0), d0)) {
                intersections.push_back(vertices(0));
            }
            if (op1 = p.onPlane(vertices(1), d1)) {
                intersections.push_back(vertices(1));
            }
            if (op2 = p.onPlane(vertices(2), d2)) {
                intersections.push_back(vertices(2));
            }
            if (d0 * d1 < 0 && !op0 && !op1) {
                T t = d0 / (d0 - d1);
                intersections.push_back(vertices(0) + t * (vertices(1) - vertices(0)));
            }
            if (d0 * d2 < 0 && !op0 && !op2) {
                T t = d0 / (d0 - d2);
                intersections.push_back(vertices(0) + t * (vertices(2) - vertices(0)));
            }
            if (d2 * d1 < 0 && !op2 && !op1) {
                T t = d2 / (d2 - d1);
                intersections.push_back(vertices(2) + t * (vertices(1) - vertices(2)));
            }
            return intersections;
        }

        // TODO: HW2
        // part 1.1
        // Implement the function to do intersection between triangle and a ray
        // Input: a ray, the ray is represented by an origin position and a direction vector
        // Output: return a real number t, the intersection is origin + dir * t, t = -1 means no intersection
        const T IntersectRay(const Vector3<T>& origin, const Vector3<T>& dir) const {
            const T epsilon = 1e-6;
            Vector3<T> e1 = _vertices[1] - _vertices[0];
            Vector3<T> e2 = _vertices[2] - _vertices[0];
            Vector3<T> h = dir.cross(e2);
            T a = h.dot(e1);
            if (a > -epsilon && a < epsilon) return -1; // ray parallel to triangle;

            T f = 1 / a;
            Vector3<T> s = origin - _vertices[0];
            T u = f * h.dot(s);
            
            if (u < 0 || u > 1) return -1;

            Vector3<T> q = s.cross(e1);
            T v = f * dir.dot(q);
            if (v < 0 || u + v > 1) return -1;

            // At this point, there is an intersection so find t
            T t = f * e2.dot(q);
            return t;
        }

    private:
        Vector3<T> _vertices[3];
    };
}