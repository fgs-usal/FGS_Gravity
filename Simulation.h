#ifndef SIMULATION_H
#define SIMULATION_H

#include <vector>
#include <cmath>

struct Vector2D {
    double x{0.0};
    double y{0.0};

    Vector2D operator+(const Vector2D& o) const { return {x + o.x, y + o.y}; }
    Vector2D operator-(const Vector2D& o) const { return {x - o.x, y - o.y}; }
    Vector2D operator*(double s) const { return {x * s, y * s}; }
    Vector2D operator/(double s) const { return {x / s, y / s}; }
    double length() const { return std::sqrt(x * x + y * y); }
};

struct Body {
    Vector2D pos;
    Vector2D vel;
    double mass{1.0};
    double radius{5.0};
    float color[3]{1.0f, 1.0f, 1.0f};
    bool isFixed{false};
    std::vector<Vector2D> trail;
};

enum class Mode {
    FIXED_CENTER = 0, // 1 cuerpo fijo en origen + 1 cuerpo móvil
    TWO_BODY_MOBILE   // 2 cuerpos móviles (problema de 2 cuerpos general)
};

class Simulation {
public:
    Simulation();

    void init(Mode mode, double m1, double m2, Vector2D r1, Vector2D v1, Vector2D r2, Vector2D v2, double G);
    
    // Presets de órbitas cónicas
    void presetCircularOrbit(Mode mode, double m1, double m2, double radius, double G);
    void presetEllipticalOrbit(Mode mode, double m1, double m2, double a, double e, double G);
    void presetParabolicOrbit(Mode mode, double m1, double m2, double r_peri, double G);
    void presetHyperbolicOrbit(Mode mode, double m1, double m2, double r_peri, double e, double G);

    void step(double dt);
    void reset();

    Mode getMode() const { return mode_; }
    const std::vector<Body>& getBodies() const { return bodies_; }
    double getG() const { return G_; }
    void setG(double g) { G_ = g; }
    void clearTrails();

private:
    Mode mode_{Mode::FIXED_CENTER};
    std::vector<Body> bodies_;
    double G_{1.0};

    // Parámetros guardados para reinicio
    double m1_init{1000.0}, m2_init{1.0};
    Vector2D r1_init, v1_init, r2_init, v2_init;
};

#endif // SIMULATION_H
