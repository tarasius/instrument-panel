/*
 * FLIGHT SIMULATOR INSTRUMENT PANEL
 * 
 * Scott Vincent
 * August 2020
 * 
 * This program was heavily inspired by Dave Ault and contains original artwork by him.
 * 
 *   http://82.7.215.98/Learjet45Chimera/index.html
 *   https://hangar45.net/hangar-45-forum/topic/standby-gauge-software-by-dave-ault
 * 
 * It has been completely rewritten and updated to use Allegro5. Hopefully,
 * Allegro5 will support hardware acceleration on the Raspberry Pi 4 soon!
 *
 * NOTE: Allegro5 must be built on RasPi4 as a standard Linux build, not the
 * specific Raspberry Pi build, i.e.:
 *
 *     mkdir build
 *     cd allegro5/build
 *     cmake .. -DCMAKE_BUILD_TYPE=Release
 *     make && sudo make install
 * 
 * KEYS
 * 
 * p ........ Adjust position and size of individual instruments.
 * v ........ Adjust FlightSim variables. Simulates changes even if no FlightSim connected.
 * m ........ Move the display to the next monitor if multiple monitors are connected.
 * s ........ Enable/disable shadows on instruments. Shadows give a more realistic 3D look.
 * Esc ...... Quit the program.
 * 
 * To make adjustments use the arrow keys. Up/down arrows select the previous or next
 * setting and left/right arrows change the value. You can also use numpad left/right
 * arrows to make larger adjustments.
 
 * NOTES
 * 
 * You can determine which instruments are included in the panel by editing the following
 * file and setting the enabled flags:
 * 
 *   settings/instrument-panel.json
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
#include <Windows.h>
#endif
#include <list>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include "globals.h"
#include "simvars.h"

// Instruments
#include "airspeedIndicator.h"
#include "attitudeIndicator.h"
#include "altimeter.h"

const double FPS = 30.0;

struct globalVars globals;

ALLEGRO_TIMER* timer = NULL;
ALLEGRO_EVENT_QUEUE* eventQueue = NULL;
bool finish = false;
std::list<instrument*> instruments;


/// <summary>
/// Display an error message and terminate
/// </summary>
void fatalError(const char* msg)
{
    char outMsg[256];

    strcpy(outMsg, msg);
    strcat(outMsg, "\n");

    printf(outMsg);

#ifdef _WIN32
    // Show message in IDE output window
    OutputDebugStringA(msg);
#endif

    printf("Exiting\n");
    exit(1);
}

/// <summary>
/// Initialise Allegro etc.
/// </summary>
void init()
{
    if (!al_init()) {
        fatalError("Failed to initialise Allegro");
    }

    if (!al_init_font_addon()) {
        fatalError("Failed to initialise font\n");
    }

    if (!al_init_image_addon()) {
        fatalError("Failed to initialise image");
    }

    if (!al_install_keyboard()) {
        fatalError("Failed to initialise keyboard");
    }

    if (!(timer = al_create_timer(1.0 / FPS))) {
        fatalError("Failed to create timer");
    }

    if (!(eventQueue = al_create_event_queue())) {
        fatalError("Failed to create event queue");
    }

    if (!(globals.font = al_create_builtin_font())) {
        fatalError("Failed to create font");
    }

    al_set_new_window_title("Instrument Panel");

    // Use existing desktop resolution/refresh rate and force OpenGL ES 3
    // for Raspberry Pi 4 hardware acceleration compatibility.
    al_set_new_display_flags(ALLEGRO_FULLSCREEN_WINDOW | ALLEGRO_FRAMELESS | ALLEGRO_OPENGL_3_0 | ALLEGRO_OPENGL_ES_PROFILE);

#ifdef _WIN32
    // Turn on vsync (fails on Raspberry Pi)
    al_set_new_display_option(ALLEGRO_VSYNC, 1, ALLEGRO_REQUIRE);
#endif

    // Resolution is ignored for fullscreen window (uses existing desktop resolution)
    // but fails on Rasberry Pi if set to 0!
    if ((globals.display = al_create_display(500, 500)) == NULL) {
            fatalError("Failed to create display");
    }

    globals.displayHeight = al_get_display_height(globals.display);
    globals.displayWidth = al_get_display_width(globals.display);

    al_hide_mouse_cursor(globals.display);
    al_inhibit_screensaver(true);

    al_register_event_source(eventQueue, al_get_keyboard_event_source());
    al_register_event_source(eventQueue, al_get_timer_event_source(timer));
    al_register_event_source(eventQueue, al_get_display_event_source(globals.display));

    globals.simVars = new simvars();
}

/// <summary>
/// Cleanup Allegro etc.
/// </summary>
void cleanup()
{
    // Destroy all instruments
    while (!instruments.empty()) {
        delete instruments.front();
        instruments.pop_front();
    }

    if (timer) {
        al_destroy_timer(timer);
    }

    if (eventQueue) {
        al_destroy_event_queue(eventQueue);
    }

    if (globals.font) {
        al_destroy_font(globals.font);
    }

    if (globals.display) {
        al_destroy_display(globals.display);
    }

    al_inhibit_screensaver(false);
}

/// <summary>
/// These variables are used by multiple instruments
/// </summary>
void addCommon()
{
    globals.simVars->addVar("Common", "Electrics", 0x0B6A, true, 1, 0);
    globals.simVars->addVar("Common", "External Controls", 0x73E0, true, 1, 1);
}

/// <summary>
/// Fetch latest values of common variables
/// </summary>
bool updateCommon()
{
    bool success = true;
    DWORD dwResult;

    // Electrics check
    unsigned char charVal;
    if (!globals.simVars->FSUIPC_Read(0x0B6A, 1, &charVal, &dwResult))
    {
        globals.electrics = false;
        success = false;
    }
    else
    {
        globals.electrics = (charVal != 1);
    }

    // APU_Status check
    if (!globals.simVars->FSUIPC_Read(0x0B52, 1, &charVal, &dwResult))
    {
        success = false;
    }
    else if (charVal == 1)
    {
        globals.electrics = true;
    }

    // Check if FS or external controls
    unsigned short fs_ext;
    if (!globals.simVars->FSUIPC_Read(0x73E0, 2, &fs_ext, &dwResult))
    {
        globals.externalControls = true;
        success = false;
    }
    else
    {
        globals.externalControls = (fs_ext == 1);
    }

    if (!globals.simVars->FSUIPC_Process(&dwResult))
    {
        success = false;
    }

    return success;
}

/// <summary>
/// Update everything before the next frame
/// </summary>
void doUpdate()
{
    // Update variables common to all instruments
    globals.connected = updateCommon();

    // Update all instruments
    for (auto const& instrument : instruments) {
        instrument->update();
    }
}

/// <summary>
/// Render the next frame
/// </summary>
void doRender()
{
    // Clear background
    al_clear_to_color(al_map_rgb(0, 0, 0));

    // Draw all instruments
    for (auto const& instrument : instruments) {
        instrument->render();
    }

    // Display variable if required
    if (globals.arranging || globals.simulating) {
        globals.simVars->view();
    }

    // Display any error message
    if (globals.error[0] != '\0') {
        al_draw_text(globals.font, al_map_rgb(0x50, 0x50, 0x50), 10, globals.displayHeight - 30, 0, globals.error);
    }
}

/// <summary>
/// Switches the display to the next monitor if multiple monitors are connected
/// </summary>
void switchMonitor()
{
    // Find all monitor positions
    int monCount = al_get_num_video_adapters();

    if (monCount <= 1) {
        return;
    }

    int monX[16];
    int monY[16];

    int monNum = 0;
    for (int i = 0; i < monCount; i++) {
        ALLEGRO_MONITOR_INFO monitorInfo;
        al_get_monitor_info(i, &monitorInfo);

        monX[i] = monitorInfo.x1;
        monY[i] = monitorInfo.y1;

        if (monX[i] == globals.displayX && monY[i] == globals.displayY) {
            monNum = i;
        }
    }

    // Move to next monitor
    monNum++;
    if (monNum == monCount) {
        monNum = 0;
    }

    globals.displayX = monX[monNum];
    globals.displayY = monY[monNum];

    al_set_window_position(globals.display, globals.displayX, globals.displayY);
}

/// <summary>
/// Handle keypress
/// </summary>
void doKeypress(ALLEGRO_EVENT *event)
{
    switch (event->keyboard.keycode) {

    case ALLEGRO_KEY_P:
        // Position and size instruments
        globals.arranging = !globals.arranging;
        if (globals.arranging) {
            globals.simulating = false;
        }
        else {
            globals.simVars->viewClear();
        }
        break;

    case ALLEGRO_KEY_V:
        // Simulate FlightSim variables
        globals.simulating = !globals.simulating;
        if (globals.simulating) {
            globals.arranging = false;
        }
        else {
            globals.simVars->viewClear();
        }
        break;

    case ALLEGRO_KEY_M:
        // Display on a different monitor
        switchMonitor();
        break;

    case ALLEGRO_KEY_S:
        // Enable/disable shadows on instruments
        globals.enableShadows = !globals.enableShadows;
        break;

    case ALLEGRO_KEY_T:
        // Used for debugging
        globals.tweak = !globals.tweak;
        break;

    case ALLEGRO_KEY_ESCAPE:
        // Quit program
        finish = true;
        break;
    }

    if (globals.arranging || globals.simulating) {
        globals.simVars->doKeypress(event->keyboard.keycode);
    }
}

/// <summary>
/// Add instruments to panel
/// </summary>
void addInstruments()
{
    // Add instruments
    instruments.push_back(new airspeedIndicator(100, 100, 350));
    instruments.push_back(new attitudeIndicator(500, 100, 350));
    instruments.push_back(new altimeter(900, 100, 350));
}

///
/// main
///
int main()
{
    init();

    addCommon();
    addInstruments();

    // Use simulated values for initial defaults so that
    // instruments look normal if we can't connect yet.
    globals.simulating = true;
    doUpdate();
    globals.simulating = false;

    bool redraw = true;
    ALLEGRO_EVENT event;

    al_start_timer(timer);
    while (true)
    {
        al_wait_for_event(eventQueue, &event);

        switch (event.type)
        {
        case ALLEGRO_EVENT_TIMER:
            doUpdate();
            redraw = true;
            break;

        case ALLEGRO_EVENT_KEY_CHAR:
            doKeypress(&event);
            break;

        case ALLEGRO_EVENT_DISPLAY_CLOSE:
            finish = true;
            break;
        }

        if (finish)
            break;

        if (redraw && al_is_event_queue_empty(eventQueue))
        {
            doRender();
            al_flip_display();
            redraw = false;
        }
    }

    // Settings get saved when simVars are destructed
    delete globals.simVars;

    cleanup();
    return 0;
}