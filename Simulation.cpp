#include "Simulation.h"
#include <algorithm>

Simulation::Simulation() {
    presetCircularOrbit(Mode::FIXED_CENTER, 1000.0, 1.0, 100.0, 1.0);
}

void Simulation::init(Mode mode, double m1, double m2, Vector2D r1, Vector2D v1, Vector2D r2, Vector2D v2, double G) {
    mode_ = mode;
    G_ = G;
    m1_init = m1; m2_init = m2;
    r1_init = r1; v1_init = v1;
    r2_init = r2; v2_init = v2;

    bodies_.clear();

    Body b1;
    b1.mass = m1;
    b1.pos = r1;
    b1.vel = v1;
    b1.radius = std::clamp(std::pow(m1, 1.0 / 3.0) * 1.5, 4.0, 25.0);
    b1.color[0] = 1.0f; b1.color[1] = 0.8f; b1.color[2] = 0.2f; // Dorado/Amarillo
    b1.isFixed = (mode == Mode::FIXED_CENTER);
    if (b1.isFixed) {
        b1.pos = {0.0, 0.0};
        b1.vel = {0.0, 0.0};
    }

    Body b2;
    b2.mass = m2;
    b2.pos = r2;
    b2.vel = v2;
    b2.radius = std::clamp(std::pow(m2, 1.0 / 3.0) * 1.5, 3.0, 18.0);
    b2.color[0] = 0.2f; b2.color[1] = 0.6f; b2.color[2] = 1.0f; // Azul cian
    b2.isFixed = false;

    bodies_.push_back(b1);
    bodies_.push_back(b2);
}

void Simulation::presetCircularOrbit(Mode mode, double m1, double m2, double radius, double G) {
    G_ = G;
    if (mode == Mode::FIXED_CENTER) {
        double v = std::sqrt(G_ * m1 / radius);
        init(mode, m1, m2, {0.0, 0.0}, {0.0, 0.0}, {radius, 0.0}, {0.0, v}, G_);
    } else {
        double M = m1 + m2;
        double r1_val = radius * m2 / M;
        double r2_val = radius * m1 / M;
        double v_rel = std::sqrt(G_ * M / radius);
        double v1_val = v_rel * m2 / M;
        double v2_val = v_rel * m1 / M;

        init(mode, m1, m2, {-r1_val, 0.0}, {0.0, -v1_val}, {r2_val, 0.0}, {0.0, v2_val}, G_);
    }
}

void Simulation::presetEllipticalOrbit(Mode mode, double m1, double m2, double a, double e, double G) {
    G_ = G;
    double r_peri = a * (1.0 - e);
    if (mode == Mode::FIXED_CENTER) {
        // v_peri = sqrt(G * M * (1 + e) / r_peri)
        double v_peri = std::sqrt(G_ * m1 * (1.0 + e) / r_peri);
        init(mode, m1, m2, {0.0, 0.0}, {0.0, 0.0}, {r_peri, 0.0}, {0.0, v_peri}, G_);
    } else {
        double M = m1 + m2;
        double r1_val = r_peri * m2 / M;
        double r2_val = r_peri * m1 / M;
        double v_rel = std::sqrt(G_ * M * (1.0 + e) / r_peri);
        double v1_val = v_rel * m2 / M;
        double v2_val = v_rel * m1 / M;

        init(mode, m1, m2, {-r1_val, 0.0}, {0.0, -v1_val}, {r2_val, 0.0}, {0.0, v2_val}, G_);
    }
}

void Simulation::presetParabolicOrbit(Mode mode, double m1, double m2, double r_peri, double G) {
    G_ = G;
    // Excentricidad e = 1. Velocidad de escape en el periapsis: v_esc = sqrt(2 * G * M / r_peri)
    if (mode == Mode::FIXED_CENTER) {
        double v_esc = std::sqrt(2.0 * G_ * m1 / r_peri);
        init(mode, m1, m2, {0.0, 0.0}, {0.0, 0.0}, {r_peri, 0.0}, {0.0, v_esc}, G_);
    } else {
        double M = m1 + m2;
        double r1_val = r_peri * m2 / M;
        double r2_val = r_peri * m1 / M;
        double v_rel = std::sqrt(2.0 * G_ * M / r_peri);
        double v1_val = v_rel * m2 / M;
        double v2_val = v_rel * m1 / M;

        init(mode, m1, m2, {-r1_val, 0.0}, {0.0, -v1_val}, {r2_val, 0.0}, {0.0, v2_val}, G_);
    }
}

void Simulation::presetHyperbolicOrbit(Mode mode, double m1, double m2, double r_peri, double e, double G) {
    G_ = G;
    // Excentricidad e > 1. Velocidad en el periapsis: v_peri = sqrt(G * M * (1 + e) / r_peri)
    if (mode == Mode::FIXED_CENTER) {
        double v_peri = std::sqrt(G_ * m1 * (1.0 + e) / r_peri);
        init(mode, m1, m2, {0.0, 0.0}, {0.0, 0.0}, {r_peri, 0.0}, {0.0, v_peri}, G_);
    } else {
        double M = m1 + m2;
        double r1_val = r_peri * m2 / M;
        double r2_val = r_peri * m1 / M;
        double v_rel = std::sqrt(G_ * M * (1.0 + e) / r_peri);
        double v1_val = v_rel * m2 / M;
        double v2_val = v_rel * m1 / M;

        init(mode, m1, m2, {-r1_val, 0.0}, {0.0, -v1_val}, {r2_val, 0.0}, {0.0, v2_val}, G_);
    }
}

void Simulation::reset() {
    init(mode_, m1_init, m2_init, r1_init, v1_init, r2_init, v2_init, G_);
}

void Simulation::clearTrails() {
    for (auto& b : bodies_) {
        b.trail.clear();
    }
}

void Simulation::step(double dt) {
    if (bodies_.size() < 2) return;

    auto getAcc = [this](const Vector2D& p1, const Vector2D& p2, double m2, bool fixed) -> Vector2D {
        if (fixed) return {0.0, 0.0};
        Vector2D r = p2 - p1;
        double dist = r.length();
        double softening = 0.5;
        double dist3 = std::pow(dist * dist + softening * softening, 1.5);
        return r * (G_ * m2 / dist3);
    };

    Vector2D p1 = bodies_[0].pos;
    Vector2D v1 = bodies_[0].vel;
    Vector2D p2 = bodies_[1].pos;
    Vector2D v2 = bodies_[1].vel;

    const size_t maxTrailPoints = 3000;
    bodies_[0].trail.push_back(p1);
    if (bodies_[0].trail.size() > maxTrailPoints) bodies_[0].trail.erase(bodies_[0].trail.begin());
    bodies_[1].trail.push_back(p2);
    if (bodies_[1].trail.size() > maxTrailPoints) bodies_[1].trail.erase(bodies_[1].trail.begin());

    // RK4
    Vector2D a1_k1 = getAcc(p1, p2, bodies_[1].mass, bodies_[0].isFixed);
    Vector2D a2_k1 = getAcc(p2, p1, bodies_[0].mass, bodies_[1].isFixed);

    Vector2D p1_k2 = p1 + v1 * (dt * 0.5);
    Vector2D v1_k2 = v1 + a1_k1 * (dt * 0.5);
    Vector2D p2_k2 = p2 + v2 * (dt * 0.5);
    Vector2D v2_k2 = v2 + a2_k1 * (dt * 0.5);

    Vector2D a1_k2 = getAcc(p1_k2, p2_k2, bodies_[1].mass, bodies_[0].isFixed);
    Vector2D a2_k2 = getAcc(p2_k2, p1_k2, bodies_[0].mass, bodies_[1].isFixed);

    Vector2D p1_k3 = p1 + v1_k2 * (dt * 0.5);
    Vector2D v1_k3 = v1 + a1_k2 * (dt * 0.5);
    Vector2D p2_k3 = p2 + v2_k2 * (dt * 0.5);
    Vector2D v2_k3 = v2 + a2_k2 * (dt * 0.5);

    Vector2D a1_k3 = getAcc(p1_k3, p2_k3, bodies_[1].mass, bodies_[0].isFixed);
    Vector2D a2_k3 = getAcc(p2_k3, p1_k3, bodies_[0].mass, bodies_[1].isFixed);

    Vector2D p1_k4 = p1 + v1_k3 * dt;
    Vector2D v1_k4 = v1 + a1_k3 * dt;
    Vector2D p2_k4 = p2 + v2_k3 * dt;
    Vector2D v2_k4 = v2 + a2_k3 * dt;

    Vector2D a1_k4 = getAcc(p1_k4, p2_k4, bodies_[1].mass, bodies_[0].isFixed);
    Vector2D a2_k4 = getAcc(p2_k4, p1_k4, bodies_[0].mass, bodies_[1].isFixed);

    if (!bodies_[0].isFixed) {
        bodies_[0].pos = p1 + (v1 + v1_k2 * 2.0 + v1_k3 * 2.0 + v1_k4) * (dt / 6.0);
        bodies_[0].vel = v1 + (a1_k1 + a1_k2 * 2.0 + a1_k3 * 2.0 + a1_k4) * (dt / 6.0);
    }
    if (!bodies_[1].isFixed) {
        bodies_[1].pos = p2 + (v2 + v2_k2 * 2.0 + v2_k3 * 2.0 + v2_k4) * (dt / 6.0);
        bodies_[1].vel = v2 + (a2_k1 + a2_k2 * 2.0 + a2_k3 * 2.0 + a2_k4) * (dt / 6.0);
    }
}
