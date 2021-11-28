#pragma once
#include <Eigen/Dense>
#include <vector>
#include <set>
#include <iostream>
#include <functional>

namespace geometry {
    template <typename T>
    using Vector3 = Eigen::Matrix<T, 3, 1>;
    template <typename T>
    using Vector2 = Eigen::Matrix<T, 2, 1>;
    template <typename T>
    using VectorX = Eigen::Matrix<T, Eigen::Dynamic, 1>;

    template <typename T>
    class VectorCompare {
    public:
        bool operator () (const Vector3<T> &v1, const Vector3<T> &v2) { return v1(1) < v2(1); }
    };

    template <typename T>
    bool comp(Vector2<T> a, Vector2<T> b) {
        return a[0] < b[0];
    }
    template <typename T>
    bool compy(Vector2<T> a, Vector2<T> b) {
        return a[1] < b[1];
    }


    // TODO: HW5
    // part 2.1 implement nlog(n) method for 2d Pareto front
    // The input is a vector of 2d points
    // The output should be the 2d points on the Pareto front
    // sort your return vector for comparing with 
    // the reference result
    template <typename T>
    std::vector<Vector2<T>> ParetoFront2D(const std::vector<Vector2<T>> &points) {
        std::vector<Vector2<T>> ps;
        for (auto& p : points)
            ps.push_back(Vector2<T> {p[0], p[1]});
        std::sort(ps.begin(), ps.end(), comp<T>);
        std::vector<Vector2<T>> pFront;

        T y = 1e10;

        for (auto p = ps.begin(); p != ps.end(); p++) {
            if ((*p)[1] < y) {
                if (!pFront.empty() && ((*p)[0] == pFront.back()[0])) { pFront.back()[1] = (*p)[1]; }
                else pFront.push_back(Vector2<T> {(*p)[0], (*p)[1]});
                y = (*p)[1];
            }
        }

        return pFront;
    }
}
