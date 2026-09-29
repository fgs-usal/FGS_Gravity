#include "GravityWindow.h"
#include <FL/gl.h>
#include <GL/glu.h>
#include <FL/Fl.H>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

GravityWindow::GravityWindow(int X, int Y, int W, int H, const char* L)
    : Fl_Gl_Window(X, Y, W, H, L) {
    mode(FL_RGB | FL_DOUBLE | FL_DEPTH | FL_MULTISAMPLE);
}

void GravityWindow::resetView() {
    zoom_ = 1.0;
    panX_ = 0.0;
    panY_ = 0.0;
    redraw();
}

void GravityWindow::drawCircle(double cx, double cy, double r, int numSegments) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2d(cx, cy);
    for (int i = 0; i <= numSegments; ++i) {
        double theta = 2.0 * M_PI * i / numSegments;
        double x = r * std::cos(theta);
        double y = r * std::sin(theta);
        glVertex2d(cx + x, cy + y);
    }
    glEnd();
}

void GravityWindow::drawGrid() {
    glColor4f(0.2f, 0.25f, 0.35f, 0.4f);
    glLineWidth(1.0f);

    double gridSize = 50.0;
    int numLines = 40;

    glBegin(GL_LINES);
    for (int i = -numLines; i <= numLines; ++i) {
        // Líneas verticales
        glVertex2d(i * gridSize, -numLines * gridSize);
        glVertex2d(i * gridSize, numLines * gridSize);
        // Líneas horizontales
        glVertex2d(-numLines * gridSize, i * gridSize);
        glVertex2d(numLines * gridSize, i * gridSize);
    }
    glEnd();

    // Ejes X e Y resaltados
    glLineWidth(1.5f);
    glBegin(GL_LINES);
    glColor3f(0.8f, 0.3f, 0.3f); // Eje X rojo suave
    glVertex2d(-numLines * gridSize, 0.0);
    glVertex2d(numLines * gridSize, 0.0);

    glColor3f(0.3f, 0.8f, 0.3f); // Eje Y verde suave
    glVertex2d(0.0, -numLines * gridSize);
    glVertex2d(0.0, numLines * gridSize);
    glEnd();
}

void GravityWindow::draw() {
    if (!valid()) {
        glViewport(0, 0, w(), h());
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_LINE_SMOOTH);
        glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    }

    glClearColor(0.05f, 0.07f, 0.12f, 1.0f); // Fondo azul marino oscuro profundo
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    double aspect = (double)w() / (double)h();
    double viewWidth = 400.0 / zoom_;
    double viewHeight = viewWidth / aspect;

    glOrtho(-viewWidth / 2.0 + panX_, viewWidth / 2.0 + panX_,
             -viewHeight / 2.0 + panY_, viewHeight / 2.0 + panY_,
             -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if (showGrid_) {
        drawGrid();
    }

    if (!sim_) return;

    const auto& bodies = sim_->getBodies();

    // Dibujar estelas
    if (showTrails_) {
        glLineWidth(2.0f);
        for (const auto& b : bodies) {
            if (b.trail.size() < 2) continue;
            glBegin(GL_LINE_STRIP);
            for (size_t i = 0; i < b.trail.size(); ++i) {
                float alpha = (float)i / (float)b.trail.size();
                glColor4f(b.color[0], b.color[1], b.color[2], alpha * 0.7f);
                glVertex2d(b.trail[i].x, b.trail[i].y);
            }
            glEnd();
        }
    }

    // Dibujar cuerpos
    for (const auto& b : bodies) {
        // Halo de resplandor externo
        glColor4f(b.color[0], b.color[1], b.color[2], 0.25f);
        drawCircle(b.pos.x, b.pos.y, b.radius * 1.5, 32);

        // Núcleo del objeto
        glColor3f(b.color[0], b.color[1], b.color[2]);
        drawCircle(b.pos.x, b.pos.y, b.radius, 32);

        // Centro brillante
        glColor3f(1.0f, 1.0f, 1.0f);
        drawCircle(b.pos.x, b.pos.y, b.radius * 0.3, 16);
    }

    // Si es modo cuerpo fijo, dibujar símbolo de origen fijo
    if (sim_->getMode() == Mode::FIXED_CENTER && !bodies.empty()) {
        glColor3f(1.0f, 0.4f, 0.4f);
        glLineWidth(2.0f);
        glBegin(GL_LINES);
        glVertex2d(-8.0, 0.0); glVertex2d(8.0, 0.0);
        glVertex2d(0.0, -8.0); glVertex2d(0.0, 8.0);
        glEnd();
    }
}

int GravityWindow::handle(int event) {
    switch (event) {
        case FL_PUSH:
            if (Fl::event_button() == FL_LEFT_MOUSE) {
                lastMouseX_ = Fl::event_x();
                lastMouseY_ = Fl::event_y();
                dragging_ = true;
                return 1;
            }
            break;

        case FL_RELEASE:
            if (Fl::event_button() == FL_LEFT_MOUSE) {
                dragging_ = false;
                return 1;
            }
            break;

        case FL_DRAG:
            if (dragging_) {
                int dx = Fl::event_x() - lastMouseX_;
                int dy = Fl::event_y() - lastMouseY_;

                double aspect = (double)w() / (double)h();
                double viewWidth = 400.0 / zoom_;
                double viewHeight = viewWidth / aspect;

                panX_ -= dx * (viewWidth / w());
                panY_ += dy * (viewHeight / h());

                lastMouseX_ = Fl::event_x();
                lastMouseY_ = Fl::event_y();
                redraw();
                return 1;
            }
            break;

        case FL_MOUSEWHEEL:
            int dy = Fl::event_dy();
            if (dy < 0) {
                zoom_ *= 1.15;
            } else if (dy > 0) {
                zoom_ /= 1.15;
                if (zoom_ < 0.05) zoom_ = 0.05;
            }
            redraw();
            return 1;
    }
    return Fl_Gl_Window::handle(event);
}
