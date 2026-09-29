#ifndef GRAVITYWINDOW_H
#define GRAVITYWINDOW_H

#include <FL/Fl_Gl_Window.H>
#include "Simulation.h"

class GravityWindow : public Fl_Gl_Window {
public:
    GravityWindow(int X, int Y, int W, int H, const char* L = nullptr);

    void setSimulation(Simulation* sim) { sim_ = sim; }
    void resetView();

    void setZoom(double z) { zoom_ = z; redraw(); }
    double getZoom() const { return zoom_; }

    void setShowTrails(bool b) { showTrails_ = b; redraw(); }
    bool getShowTrails() const { return showTrails_; }

    void setShowGrid(bool b) { showGrid_ = b; redraw(); }
    bool getShowGrid() const { return showGrid_; }

protected:
    void draw() override;
    int handle(int event) override;

private:
    Simulation* sim_{nullptr};
    double zoom_{1.0};
    double panX_{0.0};
    double panY_{0.0};
    bool showTrails_{true};
    bool showGrid_{true};

    int lastMouseX_{0};
    int lastMouseY_{0};
    bool dragging_{false};

    void drawGrid();
    void drawCircle(double cx, double cy, double r, int numSegments = 32);
};

#endif // GRAVITYWINDOW_H
