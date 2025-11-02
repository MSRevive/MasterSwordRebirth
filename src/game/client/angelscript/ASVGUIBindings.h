//==========================================================================
// ASVGUIBindings.h
//
// VGUI (Valve GUI) bindings for AngelScript integration
// CLIENT-SIDE ONLY - UI creation and manipulation for custom interfaces
//
// Provides wrapper classes and bindings for VGUI panels, allowing scripts
// to create custom user interfaces with buttons, labels, text entry, etc.
//==========================================================================

#pragma once

#ifndef VALVE_DLL  // VGUI is client-side only

class asIScriptEngine;

namespace vgui
{
    // Forward declarations of VGUI classes
    class Panel;
    class Frame;
    class Button;
    class Label;
    class TextEntry;
    class ImagePanel;
    class ScrollPanel;
    class Border;
}

namespace ASVGUIBindings
{
    //==========================================================================
    // Forward Declarations of Wrapper Classes
    //==========================================================================

    class VGUIPanel;
    class VGUIFrame;
    class VGUIButton;
    class VGUILabel;
    class VGUITextEntry;
    class VGUIImagePanel;
    class VGUIScrollPanel;

    //==========================================================================
    // Panel Management
    //==========================================================================

    // Panel registry for tracking script-created panels
    void InitializePanelRegistry();
    void CleanupPanelRegistry();
    void RegisterPanel(VGUIPanel* panel);
    void UnregisterPanel(VGUIPanel* panel);

    //==========================================================================
    // Registration Functions
    //==========================================================================

    /**
     * Register all VGUI bindings with the AngelScript engine
     * This is the main entry point called from ASBindings.cpp
     */
    void RegisterAll(asIScriptEngine* pEngine);

    /**
     * Register individual VGUI components
     */
    void RegisterVGUIPanel(asIScriptEngine* pEngine);
    void RegisterVGUIFrame(asIScriptEngine* pEngine);
    void RegisterVGUIButton(asIScriptEngine* pEngine);
    void RegisterVGUILabel(asIScriptEngine* pEngine);
    void RegisterVGUITextEntry(asIScriptEngine* pEngine);
    void RegisterVGUIImagePanel(asIScriptEngine* pEngine);
    void RegisterFactoryFunctions(asIScriptEngine* pEngine);

    //==========================================================================
    // Wrapper Classes
    // These provide a safe interface between AngelScript and VGUI
    //==========================================================================

    /**
     * VGUIPanel - Base wrapper for vgui::Panel
     * Provides core panel functionality: positioning, sizing, visibility, hierarchy
     */
    class VGUIPanel
    {
    protected:
        vgui::Panel* m_pPanel;
        bool m_bOwnPanel;  // Whether we own the panel and should delete it
        int m_iRefCount;    // Reference count for AngelScript

    public:
        VGUIPanel();
        VGUIPanel(vgui::Panel* pPanel, bool bOwnPanel = true);
        virtual ~VGUIPanel();

        // Reference counting for AngelScript
        void AddRef();
        void Release();

        // Validity checking
        bool IsValid() const;
        vgui::Panel* GetPanel() const { return m_pPanel; }

        // Position and sizing
        void SetPos(int x, int y);
        void GetPos(int& x, int& y) const;
        void SetSize(int wide, int tall);
        void GetSize(int& wide, int& tall) const;
        void SetBounds(int x, int y, int wide, int tall);
        void GetBounds(int& x, int& y, int& wide, int& tall) const;

        // Visibility
        void SetVisible(bool state);
        bool IsVisible() const;

        // Hierarchy
        void SetParent(VGUIPanel* pParent);
        void AddChild(VGUIPanel* pChild);
        void RemoveChild(VGUIPanel* pChild);

        // Appearance
        void SetFgColor(int r, int g, int b, int a);
        void SetBgColor(int r, int g, int b, int a);

        // Rendering
        void Repaint();
        void SetPaintEnabled(bool enabled);

        // Input
        bool IsMouseOver() const;
        void RequestFocus();

        // Cleanup
        void Remove();  // Remove from parent
    };

    /**
     * VGUIFrame - Wrapper for vgui::Frame
     * Provides windowed panels with title bars, borders, and draggability
     */
    class VGUIFrame : public VGUIPanel
    {
    public:
        VGUIFrame();
        VGUIFrame(int x, int y, int wide, int tall, const char* title);
        virtual ~VGUIFrame();

        // Frame-specific methods
        void SetTitle(const std::string& title);
        void SetMoveable(bool state);
        void SetSizeable(bool state);
        void SetCloseButton(bool state);
        void SetMinimizeButton(bool state);
        void Center();  // Center on screen
        void Activate();  // Make visible, request focus, and bring to front
    };

    /**
     * VGUIButton - Wrapper for vgui::Button
     * Provides clickable buttons with callback support
     */
    class VGUIButton : public VGUIPanel
    {
    private:
        std::string m_CallbackFunction;  // AngelScript callback function name

    public:
        VGUIButton();
        VGUIButton(const char* text, int x, int y, int wide, int tall);
        virtual ~VGUIButton();

        // Button-specific methods
        void SetText(const std::string& text);
        void SetCallback(const std::string& functionName);
        void SetEnabled(bool state);
        void SetArmedColor(int r, int g, int b, int a);
        void SetDepressedColor(int r, int g, int b, int a);

        // Internal - called when button is clicked
        void OnButtonPressed();
        const std::string& GetCallback() const { return m_CallbackFunction; }
    };

    /**
     * VGUILabel - Wrapper for vgui::Label
     * Provides text display with formatting options
     */
    class VGUILabel : public VGUIPanel
    {
    public:
        VGUILabel();
        VGUILabel(const char* text, int x, int y, int wide, int tall);
        virtual ~VGUILabel();

        // Label-specific methods
        void SetText(const std::string& text);
        void SetTextColor(int r, int g, int b, int a);
        void SetContentAlignment(int alignment);  // 0=left, 1=center, 2=right
    };

    /**
     * VGUITextEntry - Wrapper for vgui::TextEntry
     * Provides text input fields
     */
    class VGUITextEntry : public VGUIPanel
    {
    public:
        VGUITextEntry();
        VGUITextEntry(int x, int y, int wide, int tall);
        virtual ~VGUITextEntry();

        // TextEntry-specific methods
        void SetText(const std::string& text);
        std::string GetText() const;
        void SetEditable(bool state);
        void SetMaxLength(int maxLen);
        void SetMultiline(bool state);
    };

    /**
     * VGUIImagePanel - Wrapper for vgui::ImagePanel
     * Provides image display
     */
    class VGUIImagePanel : public VGUIPanel
    {
    public:
        VGUIImagePanel();
        VGUIImagePanel(const char* imagePath, int x, int y, int wide, int tall);
        virtual ~VGUIImagePanel();

        // ImagePanel-specific methods
        void SetImage(const std::string& imagePath);
        void SetScaleImage(bool state);
    };

    //==========================================================================
    // Factory Functions (exposed to AngelScript)
    //==========================================================================

    VGUIFrame* CreateFrame(int x, int y, int wide, int tall, const std::string& title);
    VGUIButton* CreateButton(const std::string& text, int x, int y, int wide, int tall);
    VGUILabel* CreateLabel(const std::string& text, int x, int y, int wide, int tall);
    VGUITextEntry* CreateTextEntry(int x, int y, int wide, int tall);
    VGUIImagePanel* CreateImagePanel(const std::string& imagePath, int x, int y, int wide, int tall);
    VGUIPanel* CreatePanel(int x, int y, int wide, int tall);

    //==========================================================================
    // Utility Functions
    //==========================================================================

    /**
     * Get screen dimensions
     */
    void GetScreenSize(int& wide, int& tall);

    /**
     * Get the root viewport panel for parenting top-level UI elements
     * Returns the main VGUI viewport that all visible panels must be parented to
     */
    VGUIPanel* GetViewportPanel();

    /**
     * Check if a panel is valid (not null and still exists)
     */
    bool IsPanelValid(VGUIPanel* panel);
}

#endif // CLIENT_DLL
