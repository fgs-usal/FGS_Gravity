#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Value_Input.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Toggle_Button.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/fl_draw.H>

#include "GravityWindow.h"
#include "Simulation.h"
#include <iostream>
#include <string>

// Instancias globales del GUI
Simulation sim;
GravityWindow* glWin = nullptr;
bool isRunning = true;
double simSpeed = 1.0;

// Widgets de entrada
Fl_Choice* choiceMode = nullptr;
Fl_Choice* choicePreset = nullptr;

Fl_Value_Input* inputMass1 = nullptr;
Fl_Value_Input* inputMass2 = nullptr;

Fl_Value_Input* inputPosX1 = nullptr;
Fl_Value_Input* inputPosY1 = nullptr;
Fl_Value_Input* inputVelX1 = nullptr;
Fl_Value_Input* inputVelY1 = nullptr;

Fl_Value_Input* inputPosX2 = nullptr;
Fl_Value_Input* inputPosY2 = nullptr;
Fl_Value_Input* inputVelX2 = nullptr;
Fl_Value_Input* inputVelY2 = nullptr;

Fl_Value_Input* inputG = nullptr;

Fl_Button* btnPlayPause = nullptr;
Fl_Value_Slider* sliderSpeed = nullptr;

// Función para actualizar el estado del formulario según el modo
void updateControlsState() {
    int modeIdx = choiceMode->value();
    if (modeIdx == 0) { // Objeto fijo en origen
        inputPosX1->value(0.0); inputPosX1->deactivate();
        inputPosY1->value(0.0); inputPosY1->deactivate();
        inputVelX1->value(0.0); inputVelX1->deactivate();
        inputVelY1->value(0.0); inputVelY1->deactivate();
    } else { // Dos objetos móviles
        inputPosX1->activate();
        inputPosY1->activate();
        inputVelX1->activate();
        inputVelY1->activate();
    }
}

// Aplicar parámetros personalizados
void applyCustomInputs() {
    Mode mode = (choiceMode->value() == 0) ? Mode::FIXED_CENTER : Mode::TWO_BODY_MOBILE;
    double m1 = inputMass1->value();
    double m2 = inputMass2->value();

    Vector2D r1{inputPosX1->value(), inputPosY1->value()};
    Vector2D v1{inputVelX1->value(), inputVelY1->value()};

    Vector2D r2{inputPosX2->value(), inputPosY2->value()};
    Vector2D v2{inputVelX2->value(), inputVelY2->value()};

    double G = inputG->value();

    sim.init(mode, m1, m2, r1, v1, r2, v2, G);
    glWin->redraw();
}

// Cargar preset de órbita tradicional
void loadPreset() {
    int presetIdx = choicePreset->value();
    if (presetIdx == 0) return; // Personalizado (Manual)

    Mode mode = (choiceMode->value() == 0) ? Mode::FIXED_CENTER : Mode::TWO_BODY_MOBILE;
    double G = inputG->value();
    double m1 = inputMass1->value();
    double m2 = inputMass2->value();

    switch (presetIdx) {
        case 1: // Órbita Circular (e = 0)
            sim.presetCircularOrbit(mode, m1, m2, 100.0, G);
            break;
        case 2: // Órbita Elíptica Moderada (e = 0.4)
            sim.presetEllipticalOrbit(mode, m1, m2, 120.0, 0.4, G);
            break;
        case 3: // Órbita Muy Elíptica (e = 0.75)
            sim.presetEllipticalOrbit(mode, m1, m2, 130.0, 0.75, G);
            break;
        case 4: // Órbita Parabólica (e = 1.0, Velocidad de Escape)
            sim.presetParabolicOrbit(mode, m1, m2, 60.0, G);
            break;
        case 5: // Órbita Hiperbólica (e = 1.6, Trayectoria abierta)
            sim.presetHyperbolicOrbit(mode, m1, m2, 60.0, 1.6, G);
            break;
    }

    // Actualizar campos numéricos en la interfaz
    const auto& bodies = sim.getBodies();
    if (bodies.size() >= 2) {
        inputMass1->value(bodies[0].mass);
        inputMass2->value(bodies[1].mass);

        inputPosX1->value(bodies[0].pos.x);
        inputPosY1->value(bodies[0].pos.y);
        inputVelX1->value(bodies[0].vel.x);
        inputVelY1->value(bodies[0].vel.y);

        inputPosX2->value(bodies[1].pos.x);
        inputPosY2->value(bodies[1].pos.y);
        inputVelX2->value(bodies[1].vel.x);
        inputVelY2->value(bodies[1].vel.y);
    }
    updateControlsState();
    glWin->redraw();
}

// Callbacks
void cb_mode(Fl_Widget*, void*) {
    updateControlsState();
    if (choicePreset->value() != 0) {
        loadPreset();
    } else {
        applyCustomInputs();
    }
}

void cb_preset(Fl_Widget*, void*) {
    if (choicePreset->value() != 0) {
        loadPreset();
    }
}

void cb_apply(Fl_Widget*, void*) {
    choicePreset->value(0); // Cambiar a manual
    applyCustomInputs();
}

void cb_reset(Fl_Widget*, void*) {
    sim.reset();
    glWin->resetView();
}

void cb_play_pause(Fl_Widget*, void*) {
    isRunning = !isRunning;
    if (isRunning) {
        btnPlayPause->label("Pausa");
        btnPlayPause->color(FL_YELLOW);
    } else {
        btnPlayPause->label("Iniciar");
        btnPlayPause->color(FL_GREEN);
    }
}

void cb_trails(Fl_Widget* w, void*) {
    Fl_Toggle_Button* btn = (Fl_Toggle_Button*)w;
    glWin->setShowTrails(btn->value() != 0);
}

void cb_grid(Fl_Widget* w, void*) {
    Fl_Toggle_Button* btn = (Fl_Toggle_Button*)w;
    glWin->setShowGrid(btn->value() != 0);
}

void cb_clear_trails(Fl_Widget*, void*) {
    sim.clearTrails();
    glWin->redraw();
}

void timer_cb(void*) {
    if (isRunning) {
        double dt = 0.05 * simSpeed;
        sim.step(dt);
        glWin->redraw();
    }
    Fl::repeat_timeout(1.0 / 60.0, timer_cb);
}

int main(int argc, char** argv) {
    Fl::scheme("gtk+");

    Fl_Double_Window* window = new Fl_Double_Window(1150, 720, "FGS_Gravity - Simulación de Interacción Gravitatoria");
    window->color(fl_rgb_color(30, 34, 42));

    // Área OpenGL (Izquierda)
    glWin = new GravityWindow(10, 10, 780, 700, "Pantalla OpenGL");
    glWin->setSimulation(&sim);

    // Panel de control (Derecha)
    Fl_Group* panel = new Fl_Group(800, 10, 340, 700, "Panel de Control");
    panel->box(FL_ROUNDED_BOX);
    panel->color(fl_rgb_color(40, 44, 52));
    panel->labelcolor(FL_WHITE);

    int y = 30;
    Fl_Box* lblTitle = new Fl_Box(810, y, 320, 30, "CONFIGURACIÓN SIMULACIÓN");
    lblTitle->labelsize(14);
    lblTitle->labelfont(FL_BOLD);
    lblTitle->labelcolor(fl_rgb_color(100, 180, 255));
    y += 35;

    // 1. Selector de Modo
    Fl_Box* lblMode = new Fl_Box(810, y, 320, 20, "Modo de Sistema:");
    lblMode->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    lblMode->labelcolor(FL_WHITE);
    y += 22;

    choiceMode = new Fl_Choice(810, y, 320, 28);
    choiceMode->add("1. Objeto Fijo en el Origen");
    choiceMode->add("2. Dos Objetos Móviles (Centro Masas)");
    choiceMode->value(0);
    choiceMode->callback(cb_mode);
    y += 35;

    // 2. Selector de Órbitas Tradicionales (Presets)
    Fl_Box* lblPreset = new Fl_Box(810, y, 320, 20, "Órbita Tradicional (Preset):");
    lblPreset->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    lblPreset->labelcolor(FL_WHITE);
    y += 22;

    choicePreset = new Fl_Choice(810, y, 320, 28);
    choicePreset->add("Personalizada (Manual)");
    choicePreset->add("Órbita Circular (e = 0)");
    choicePreset->add("Órbita Elíptica (e = 0.4)");
    choicePreset->add("Órbita Muy Elíptica (e = 0.75)");
    choicePreset->add("Órbita Parabólica (e = 1.0, Escape)");
    choicePreset->add("Órbita Hiperbólica (e = 1.6, Abierta)");
    choicePreset->value(1); // Empezar en circular
    choicePreset->callback(cb_preset);
    y += 40;

    // 3. Condiciones Iniciales
    Fl_Box* lblCond = new Fl_Box(810, y, 320, 25, "Condiciones Iniciales:");
    lblCond->labelfont(FL_BOLD);
    lblCond->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    lblCond->labelcolor(fl_rgb_color(255, 200, 100));
    y += 25;

    // Constante G
    inputG = new Fl_Value_Input(960, y, 170, 24, "Constante G:");
    inputG->labelcolor(FL_WHITE);
    inputG->value(1.0);
    y += 30;

    // Cuerpo 1
    Fl_Box* lblB1 = new Fl_Box(810, y, 320, 20, "-- Cuerpo 1 (Dorado/Centro) --");
    lblB1->labelcolor(fl_rgb_color(255, 215, 0));
    y += 22;

    inputMass1 = new Fl_Value_Input(960, y, 170, 24, "Masa 1:");
    inputMass1->labelcolor(FL_WHITE);
    inputMass1->value(1000.0);
    y += 26;

    inputPosX1 = new Fl_Value_Input(860, y, 100, 24, "Pos X1:");
    inputPosX1->labelcolor(FL_WHITE);
    inputPosY1 = new Fl_Value_Input(1030, y, 100, 24, "Y1:");
    inputPosY1->labelcolor(FL_WHITE);
    y += 26;

    inputVelX1 = new Fl_Value_Input(860, y, 100, 24, "Vel X1:");
    inputVelX1->labelcolor(FL_WHITE);
    inputVelY1 = new Fl_Value_Input(1030, y, 100, 24, "Y1:");
    inputVelY1->labelcolor(FL_WHITE);
    y += 32;

    // Cuerpo 2
    Fl_Box* lblB2 = new Fl_Box(810, y, 320, 20, "-- Cuerpo 2 (Azul/Orbital) --");
    lblB2->labelcolor(fl_rgb_color(100, 200, 255));
    y += 22;

    inputMass2 = new Fl_Value_Input(960, y, 170, 24, "Masa 2:");
    inputMass2->labelcolor(FL_WHITE);
    inputMass2->value(1.0);
    y += 26;

    inputPosX2 = new Fl_Value_Input(860, y, 100, 24, "Pos X2:");
    inputPosX2->labelcolor(FL_WHITE);
    inputPosY2 = new Fl_Value_Input(1030, y, 100, 24, "Y2:");
    inputPosY2->labelcolor(FL_WHITE);
    y += 26;

    inputVelX2 = new Fl_Value_Input(860, y, 100, 24, "Vel X2:");
    inputVelX2->labelcolor(FL_WHITE);
    inputVelY2 = new Fl_Value_Input(1030, y, 100, 24, "Y2:");
    inputVelY2->labelcolor(FL_WHITE);
    y += 35;

    // Botones de Aplicar y Reiniciar
    Fl_Button* btnApply = new Fl_Button(810, y, 155, 30, "Aplicar Manual");
    btnApply->color(fl_rgb_color(60, 120, 180));
    btnApply->labelcolor(FL_WHITE);
    btnApply->callback(cb_apply);

    Fl_Button* btnReset = new Fl_Button(975, y, 155, 30, "Reiniciar Vista");
    btnReset->color(fl_rgb_color(140, 60, 60));
    btnReset->labelcolor(FL_WHITE);
    btnReset->callback(cb_reset);
    y += 45;

    // 4. Controles de Ejecución y Visualización
    Fl_Box* lblCtrl = new Fl_Box(810, y, 320, 20, "-- Simulación y Visualización --");
    lblCtrl->labelcolor(FL_WHITE);
    y += 25;

    btnPlayPause = new Fl_Button(810, y, 155, 32, "Pausa");
    btnPlayPause->color(FL_YELLOW);
    btnPlayPause->callback(cb_play_pause);

    Fl_Button* btnClear = new Fl_Button(975, y, 155, 32, "Borrar Estelas");
    btnClear->color(fl_rgb_color(80, 80, 90));
    btnClear->labelcolor(FL_WHITE);
    btnClear->callback(cb_clear_trails);
    y += 40;

    Fl_Toggle_Button* btnTrails = new Fl_Toggle_Button(810, y, 155, 28, "Estelas");
    btnTrails->value(1);
    btnTrails->callback(cb_trails);

    Fl_Toggle_Button* btnGrid = new Fl_Toggle_Button(975, y, 155, 28, "Rejilla Ejes");
    btnGrid->value(1);
    btnGrid->callback(cb_grid);
    y += 40;

    sliderSpeed = new Fl_Value_Slider(810, y, 320, 25, "Velocidad de Simulación");
    sliderSpeed->type(FL_HOR_SLIDER);
    sliderSpeed->bounds(0.1, 5.0);
    sliderSpeed->value(1.0);
    sliderSpeed->align(FL_ALIGN_TOP);
    sliderSpeed->labelcolor(FL_WHITE);
    sliderSpeed->callback([](Fl_Widget* w, void*) {
        simSpeed = ((Fl_Value_Slider*)w)->value();
    });

    panel->end();

    window->end();
    window->resizable(glWin);
    window->show(argc, argv);

    loadPreset();

    Fl::add_timeout(1.0 / 60.0, timer_cb);

    return Fl::run();
}
