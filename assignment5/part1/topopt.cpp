#define TC_USE_DOUBLE
#define TC_IMAGE_IO
#include "taichi.h"
using namespace taichi;

int window_width = 1080, window_height = (window_width - 200) / 2;
using Vector = VectorND<2, real>;
using Vectori = VectorND<2, int>;
template <typename T>
using Array = ArrayND<2, T>;

// Array is like a 2D C++ array. The element can be either real or 2D Vector
// In this file, we use real = double

class TopologyOptimization {
 public:
  Vectori cell_res, node_res;
  real volume_fraction, minimum_density, penalty;
  real iteration_time, start_time, total_time, objective;
  bool running;
  int iteration;
  int bc_type;
  real filter_radius;
  real change_limit;
  Array<Vector> f, u, last_u;
  Array<real> density;
  CPUCGHexFEMSolver<2> fem;
  std::mutex mut;

  typename HexFEMSolver<2>::BoundaryCondition boundary_condition;

  void initialize_boundary_condition() {
    f.reset(Vector2(0));
    boundary_condition.clear();
    switch (bc_type) {
      case 1:  // MBB Beam
        // Load
        f[0][cell_res[1]][1] = -1;
        // Dirichlet BCs (fixed degrees of freedom)
        // Boundary condition = {(node_x, node_y), axis to
        // fix (0 for x, 1 for y), offset (usually zero)}
        for (int j = 0; j < node_res[1]; j++)
          boundary_condition.push_back({Vector2i(0, j), 0, 0});
        boundary_condition.push_back({Vector2i(cell_res[0], 0), 1, 0});
        break;
      case 2:  // Cantilever Beam (TODO, HW5 part 1.3)
		// ...
        // Load
          f[cell_res[0]][cell_res[1]/2][1] = -1;
          boundary_condition.push_back({ Vector2i(0,0), 0, 0 });
          boundary_condition.push_back({ Vector2i(0,0), 1, 0 });
          boundary_condition.push_back({ Vector2i(0,cell_res[1]), 0, 0});
          boundary_condition.push_back({ Vector2i(0,cell_res[1]), 1, 0});
        break;
      case 3:  // Bridge (TODO, HW5 part 1.3)
		// ...
          //for (int j = 0; j < node_res[0]; j++) f[j][node_res[1]][1] = -1;
          for (int i = 0; i < cell_res[0]; i++) f[i][cell_res[1]][1] = -1;
          f[0][cell_res[1]][1] = -1;
          boundary_condition.push_back({ Vector2i(0,0), 0, 0 });
          boundary_condition.push_back({ Vector2i(0,0), 1, 0 });
          boundary_condition.push_back({ Vector2i(cell_res[0],0), 0, 0});
          boundary_condition.push_back({ Vector2i(cell_res[0],0), 1, 0 });
        break;
    }
  }

  TopologyOptimization() {
    running = false;
    std::thread th([this]() {
      // Main loop in a separate thread
      while (1) {
        if (running) {
          auto change = iterate();
          if (change < 0.005)
            running = false;
        }
        taichi::Time::sleep(0.01);
      }
    });
    th.detach();
  }

  void initialize() {
    start_time = taichi::Time::get_time();
    iteration_time = total_time = iteration = objective = 0;
    minimum_density = 1e-2_f;
    node_res = cell_res + Vector2i(1, 1);
    fem.initialize(node_res, penalty);
    u.initialize(node_res, Vector(0.0f));
    last_u = u;
    density.initialize(cell_res, volume_fraction);
    f.initialize(node_res, Vector(0.0f));
    initialize_boundary_condition();
    fem.set_boundary_condition(boundary_condition);
    running = true;
  }

  // Task 1:
  Array<real> optimality_criteria(const Array<real> &s) {
    Array<real> new_density = density.same_shape();
    // TODO: HW5
    // part 1.1 implement the Optimality Criteria algorithm
    // Note: You should make use of
    //    minimum_density, change_limit, volume_fraction, cell_res, density here
	  // ...
    // you should replace the following line with your own code
    double lambda_lower = 0, lambda_higher = 1e15;
    while ((1 + 1e-15) * lambda_lower < lambda_higher) {
        double lambda = (lambda_lower + lambda_higher) / 2;
        double S = 0.;
        for (int i = 0;i < cell_res[0];i++) 
            for (int j = 0;j < cell_res[1];j++) {
                double v_prime = std::min(
                    std::min(
                        std::max(
                            std::max(
                                density[i][j] - change_limit,
                                std::sqrt(s[i][j] / lambda) * density[i][j]),
                            minimum_density
                        ),
                        density[i][j] + change_limit
                    ), 1.);
                S += v_prime;
                new_density[i][j] = v_prime;
            }
        if (S < volume_fraction * cell_res[0] * cell_res[1]) lambda_higher = lambda;
        else lambda_lower = lambda;
    }
    

    return new_density;
  }


  Array<real> sensitivity_filtering(const Array<real> &s) const {
    Array<real> s_filtered = s.same_shape();
    if (filter_radius == 0.0) {
      s_filtered = s;  // no filtering. Directly copy.
    } else {
      for (int i = 0; i < cell_res[0]; i++) {
        for (int j = 0; j < cell_res[1]; j++) {
          // TODO: HW5
          // part 1.2 Sensitivity filtering
          // TODO: replace the follow line with filtering.
           double numerator = 0., denominator = 0.;
           for (int ii = std::max(0., i - filter_radius); ii < std::min(i + filter_radius, (double)cell_res[0]); ii++) {
               for (int jj = std::max(0., j - filter_radius); jj < std::min(j + filter_radius, (double)cell_res[1]); jj++) {
                   double dist = std::sqrt(std::pow((ii - i), 2) + std::pow((jj - j), 2));
                   numerator += (s[ii][jj] * density[ii][jj] * std::max(0., filter_radius - dist));
                   denominator += (std::max(0., filter_radius - dist) * density[ii][jj]);
               }
           }
           if (denominator <= 0.)
               s_filtered[i][j] = 0.;
           else s_filtered[i][j] = numerator / denominator;
          // NOTE: density and s have size of ``cell_res''.
          //       Be careful not to access the undefined region outside the
          //       range.
          //...
        }
      }
    }
    return s_filtered;
  }

  // Optimization iteration.
  // returns: max change
  real iterate() {
    std::lock_guard<std::mutex> _(mut);
    real start_t = taichi::Time::get_time();
    Array<real> s = density.same_shape(0.0f);  // sensitivity
    auto u = fem.solve(density, f, last_u, s, objective);
    last_u = u;
    s = sensitivity_filtering(s);
    auto new_density = optimality_criteria(s);
    auto diff = new_density - density;
    real change = diff.abs_max();
    density = new_density;
    iteration += 1;
    iteration_time = 1000 * (taichi::Time::get_time() - start_t);
    total_time = taichi::Time::get_time() - start_time;
    return change;
  }

  void render(Canvas &canvas) {
    real scale_x = density.get_res()[0] / real(window_width - 200);
    real scale_y = density.get_res()[1] / real(window_height);
    for (auto &ind : Region2D(Vector2i(0, 0),
                              Vector2i(window_width - 200, window_height))) {
      Vectori coord(int32(ind.get_ipos()[0] * scale_x),
                    int32((ind.get_ipos()[1]) * scale_y));
      if (density.inside(coord))
        canvas.img[ind] = Vector4(1 - density[coord]);
    }
  }
};

TopologyOptimization opt;

int main() {
  GUI gui("Topology Optimization", Vector2i(window_width, window_height),
          false);
  int res = 50, penalty = 3, bc_type = 1;
  real volume_fraction = 0.5, filter_radius = 1.5, change_limit = 0.2;
  auto start = [&]() {
    opt.running = false;
    std::lock_guard<std::mutex> _(opt.mut);
    opt.cell_res = Vector2i(res, res / 2);
    opt.volume_fraction = volume_fraction;
    opt.penalty = penalty;
    opt.filter_radius = filter_radius;
    opt.change_limit = change_limit;
    opt.bc_type = bc_type;
    opt.initialize();
  };
  start();
  gui.button("(Re)start", start)
      .slider("Resolution", res, 10, 200)
      .slider("BC Type", bc_type, 1, 3)
      .slider("Volume Fraction (c)", volume_fraction, 0.1, 0.99)
      .slider("OC Change Limit (M)", change_limit, 0.01, 1.0)
      .slider("SIMP Penalty", penalty, 1, 4)
      .slider("Filter Radius", filter_radius, 0.0, 5.0)
      .label("Iteration", opt.iteration)
      .label("Iter. Time (ms)", opt.iteration_time)
      .label("Total Time (s)", opt.total_time)
      .label("Objective", opt.objective)
      .button("Save Image", [&] { gui.screenshot(); });
  while (1) {
    gui.canvas->clear(Vector4(1));
    opt.render(gui.get_canvas());
    gui.update();
  }
  return 0;
};

