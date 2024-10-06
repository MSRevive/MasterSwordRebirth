
#ifndef VGUI_INT_H
#define VGUI_INT_H

namespace VGUISystem {
    void Startup();
    void Shutdown();

    //Only safe to call from inside subclass of Panel::paintBackground
    void ViewportPaintBackground(int extents[4]);
}

#endif