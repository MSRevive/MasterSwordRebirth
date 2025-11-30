//==========================================================================
// ASVGUIBindings.cpp - asbind20 Implementation
//
// VGUI (Valve GUI) bindings for AngelScript integration
// CLIENT-SIDE ONLY - UI creation and manipulation
//
// Provides safe wrappers around VGUI panels with reference counting
// and callback support for interactive UI elements
//==========================================================================

#ifndef VALVE_DLL  // VGUI is client-side only

// Include standard library headers first
#include <cstdio>
#include <string>
#include <vector>
#include <map>
#include <angelscript.h>

// Include asbind20 for modern binding syntax
#include <asbind20/asbind.hpp>

// Include MSLogger
#include "mslogger.h"

// Include VGUI headers
#include <VGUI.h>
#include <VGUI_App.h>
#include <VGUI_Panel.h>
#include <VGUI_Frame.h>
#include <VGUI_Button.h>
#include <VGUI_Label.h>
#include <VGUI_TextEntry.h>
#include <VGUI_ImagePanel.h>
#include <VGUI_ScrollPanel.h>
#include <VGUI_Color.h>
#include <VGUI_Scheme.h>
#include <VGUI_Font.h>

// Include client headers
#include "hud.h"
#include "cl_util.h"
#include "ui/vgui_teamfortressviewport.h"

// Include AngelScript headers
#include "ASVGUIBindings.h"
#include "angelscript/ASEngineInterface.h"

namespace ASVGUIBindings
{
    //==========================================================================
    // Panel Registry - Track all panels created by scripts
    //==========================================================================

    static std::map<VGUIPanel*, bool> g_PanelRegistry;
    static bool g_bRegistryInitialized = false;

    void InitializePanelRegistry()
    {
        if (!g_bRegistryInitialized)
        {
            g_PanelRegistry.clear();
            g_bRegistryInitialized = true;
            MS_ANGEL_INFO("[ASVGUIBindings] Panel registry initialized");
        }
    }

    void CleanupPanelRegistry()
    {
        MS_ANGEL_INFO("[ASVGUIBindings] Cleaning up %d registered panels", g_PanelRegistry.size());

        // Delete all registered panels
        for (auto it = g_PanelRegistry.begin(); it != g_PanelRegistry.end(); ++it)
        {
            VGUIPanel* panel = it->first;
            if (panel)
            {
                delete panel;
            }
        }

        g_PanelRegistry.clear();
        g_bRegistryInitialized = false;
        MS_ANGEL_INFO("[ASVGUIBindings] Panel registry cleaned up");
    }

    void RegisterPanel(VGUIPanel* panel)
    {
        if (panel)
        {
            g_PanelRegistry[panel] = true;
            MS_ANGEL_DEBUG("[ASVGUIBindings] Registered panel %p", panel);
        }
    }

    void UnregisterPanel(VGUIPanel* panel)
    {
        if (panel)
        {
            g_PanelRegistry.erase(panel);
            MS_ANGEL_DEBUG("[ASVGUIBindings] Unregistered panel %p", panel);
        }
    }

    //==========================================================================
    // VGUIPanel Implementation
    //==========================================================================

    VGUIPanel::VGUIPanel()
        : m_pPanel(nullptr), m_bOwnPanel(false), m_iRefCount(1)
    {
        MS_ANGEL_DEBUG("VGUIPanel::VGUIPanel() - default constructor");
        RegisterPanel(this);
    }

    VGUIPanel::VGUIPanel(vgui::Panel* pPanel, bool bOwnPanel)
        : m_pPanel(pPanel), m_bOwnPanel(bOwnPanel), m_iRefCount(1)
    {
        MS_ANGEL_DEBUG("VGUIPanel::VGUIPanel(%p, %d)", pPanel, bOwnPanel);
        RegisterPanel(this);
    }

    VGUIPanel::~VGUIPanel()
    {
        MS_ANGEL_DEBUG("VGUIPanel::~VGUIPanel() - panel: %p, own: %d", m_pPanel, m_bOwnPanel);
        UnregisterPanel(this);

        if (m_pPanel && m_bOwnPanel)
        {
            // VGUI panels delete themselves when removed from parent
            // or we can explicitly delete them
            delete m_pPanel;
            m_pPanel = nullptr;
        }
    }

    void VGUIPanel::AddRef()
    {
        m_iRefCount++;
        MS_ANGEL_DEBUG("VGUIPanel::AddRef() - refcount: %d", m_iRefCount);
    }

    void VGUIPanel::Release()
    {
        m_iRefCount--;
        MS_ANGEL_DEBUG("VGUIPanel::Release() - refcount: %d", m_iRefCount);

        if (m_iRefCount <= 0)
        {
            delete this;
        }
    }

    bool VGUIPanel::IsValid() const
    {
        return m_pPanel != nullptr;
    }

    void VGUIPanel::SetPos(int x, int y)
    {
        if (!IsValid()) return;
        m_pPanel->setPos(x, y);
        MS_ANGEL_DEBUG("VGUIPanel::SetPos(%d, %d)", x, y);
    }

    void VGUIPanel::GetPos(int& x, int& y) const
    {
        if (!IsValid())
        {
            x = 0;
            y = 0;
            return;
        }
        m_pPanel->getPos(x, y);
    }

    void VGUIPanel::SetSize(int wide, int tall)
    {
        if (!IsValid()) return;
        m_pPanel->setSize(wide, tall);
        MS_ANGEL_DEBUG("VGUIPanel::SetSize(%d, %d)", wide, tall);
    }

    void VGUIPanel::GetSize(int& wide, int& tall) const
    {
        if (!IsValid())
        {
            wide = 0;
            tall = 0;
            return;
        }
        m_pPanel->getSize(wide, tall);
    }

    void VGUIPanel::SetBounds(int x, int y, int wide, int tall)
    {
        if (!IsValid()) return;
        m_pPanel->setBounds(x, y, wide, tall);
        MS_ANGEL_DEBUG("VGUIPanel::SetBounds(%d, %d, %d, %d)", x, y, wide, tall);
    }

    void VGUIPanel::GetBounds(int& x, int& y, int& wide, int& tall) const
    {
        if (!IsValid())
        {
            x = 0;
            y = 0;
            wide = 0;
            tall = 0;
            return;
        }
        m_pPanel->getBounds(x, y, wide, tall);
    }

    void VGUIPanel::SetVisible(bool state)
    {
        if (!IsValid()) return;
        m_pPanel->setVisible(state);
        MS_ANGEL_DEBUG("VGUIPanel::SetVisible(%d)", state);
    }

    bool VGUIPanel::IsVisible() const
    {
        if (!IsValid()) return false;
        return m_pPanel->isVisible();
    }

    void VGUIPanel::SetParent(VGUIPanel* pParent)
    {
        if (!IsValid()) return;
        if (!pParent || !pParent->IsValid())
        {
            MS_ANGEL_ERROR("VGUIPanel::SetParent() - invalid parent panel");
            return;
        }
        m_pPanel->setParent(pParent->GetPanel());

        // CRITICAL FIX: Add a permanent reference to keep this panel alive
        // while it's in the VGUI hierarchy. This prevents premature deletion
        // when AngelScript local handles go out of scope.
        this->AddRef();

        MS_ANGEL_DEBUG("VGUIPanel::SetParent(%p) - AddRef called, refcount now %d", pParent, m_iRefCount);
    }

    void VGUIPanel::AddChild(VGUIPanel* pChild)
    {
        if (!IsValid()) return;
        if (!pChild || !pChild->IsValid())
        {
            MS_ANGEL_ERROR("VGUIPanel::AddChild() - invalid child panel");
            return;
        }
        pChild->GetPanel()->setParent(m_pPanel);

        // CRITICAL FIX: Add a permanent reference to keep child alive
        // while it's in the VGUI hierarchy. This prevents premature deletion
        // when AngelScript local handles go out of scope.
        pChild->AddRef();

        MS_ANGEL_DEBUG("VGUIPanel::AddChild(%p) - AddRef called, child refcount now %d", pChild, pChild->m_iRefCount);
    }

    void VGUIPanel::RemoveChild(VGUIPanel* pChild)
    {
        if (!IsValid()) return;
        if (!pChild || !pChild->IsValid())
        {
            MS_ANGEL_ERROR("VGUIPanel::RemoveChild() - invalid child panel");
            return;
        }
        pChild->GetPanel()->setParent(nullptr);

        // Release the reference that was added when the child was parented
        // This allows the child to be deleted if no other references exist
        pChild->Release();

        MS_ANGEL_DEBUG("VGUIPanel::RemoveChild(%p) - Release called", pChild);
    }

    void VGUIPanel::SetFgColor(int r, int g, int b, int a)
    {
        if (!IsValid()) return;
        m_pPanel->setFgColor(r, g, b, a);
        MS_ANGEL_DEBUG("VGUIPanel::SetFgColor(%d, %d, %d, %d)", r, g, b, a);
    }

    void VGUIPanel::SetBgColor(int r, int g, int b, int a)
    {
        if (!IsValid()) return;
        m_pPanel->setBgColor(r, g, b, a);
        MS_ANGEL_DEBUG("VGUIPanel::SetBgColor(%d, %d, %d, %d)", r, g, b, a);
    }

    void VGUIPanel::Repaint()
    {
        if (!IsValid()) return;
        m_pPanel->repaint();
    }

    void VGUIPanel::SetPaintEnabled(bool enabled)
    {
        if (!IsValid()) return;
        m_pPanel->setPaintEnabled(enabled);
        MS_ANGEL_DEBUG("VGUIPanel::SetPaintEnabled(%d)", enabled);
    }

    bool VGUIPanel::IsMouseOver() const
    {
        if (!IsValid()) return false;

        // Get mouse position and check if it's within panel bounds
        int mx, my;
        vgui::App::getInstance()->getCursorPos(mx, my);

        int x, y, wide, tall;
        m_pPanel->getAbsExtents(x, y, wide, tall);

        return (mx >= x && mx <= wide && my >= y && my <= tall);
    }

    void VGUIPanel::RequestFocus()
    {
        if (!IsValid()) return;
        m_pPanel->requestFocus();
        MS_ANGEL_DEBUG("VGUIPanel::RequestFocus()");
    }

    void VGUIPanel::Remove()
    {
        if (!IsValid()) return;
        m_pPanel->setParent(nullptr);
        MS_ANGEL_DEBUG("VGUIPanel::Remove()");
    }

    //==========================================================================
    // VGUIFrame Implementation
    //==========================================================================

    VGUIFrame::VGUIFrame()
        : VGUIPanel()
    {
        m_pPanel = new vgui::Frame(0, 0, 320, 240);
        m_bOwnPanel = true;
        MS_ANGEL_DEBUG("VGUIFrame::VGUIFrame() - created frame: %p", m_pPanel);
    }

    VGUIFrame::VGUIFrame(int x, int y, int wide, int tall, const char* title)
        : VGUIPanel()
    {
        vgui::Frame* pFrame = new vgui::Frame(x, y, wide, tall);
        if (title && title[0])
        {
            pFrame->setTitle(title);
        }
        m_pPanel = pFrame;
        m_bOwnPanel = true;
        MS_ANGEL_DEBUG("VGUIFrame::VGUIFrame(%d, %d, %d, %d, \"%s\")", x, y, wide, tall, title ? title : "");
    }

    VGUIFrame::~VGUIFrame()
    {
        MS_ANGEL_DEBUG("VGUIFrame::~VGUIFrame()");
    }

    void VGUIFrame::SetTitle(const std::string& title)
    {
        if (!IsValid()) return;
        vgui::Frame* pFrame = dynamic_cast<vgui::Frame*>(m_pPanel);
        if (pFrame)
        {
            pFrame->setTitle(title.c_str());
            MS_ANGEL_DEBUG("VGUIFrame::SetTitle(\"%s\")", title.c_str());
        }
    }

    void VGUIFrame::SetMoveable(bool state)
    {
        if (!IsValid()) return;
        vgui::Frame* pFrame = dynamic_cast<vgui::Frame*>(m_pPanel);
        if (pFrame)
        {
            pFrame->setMoveable(state);
            MS_ANGEL_DEBUG("VGUIFrame::SetMoveable(%d)", state);
        }
    }

    void VGUIFrame::SetSizeable(bool state)
    {
        if (!IsValid()) return;
        vgui::Frame* pFrame = dynamic_cast<vgui::Frame*>(m_pPanel);
        if (pFrame)
        {
            pFrame->setSizeable(state);
            MS_ANGEL_DEBUG("VGUIFrame::SetSizeable(%d)", state);
        }
    }

    void VGUIFrame::SetCloseButton(bool state)
    {
        if (!IsValid()) return;
        vgui::Frame* pFrame = dynamic_cast<vgui::Frame*>(m_pPanel);
        if (pFrame)
        {
            pFrame->setCloseButtonVisible(state);
            MS_ANGEL_DEBUG("VGUIFrame::SetCloseButton(%d)", state);
        }
    }

    void VGUIFrame::SetMinimizeButton(bool state)
    {
        if (!IsValid()) return;
        vgui::Frame* pFrame = dynamic_cast<vgui::Frame*>(m_pPanel);
        if (pFrame)
        {
            pFrame->setMinimizeButtonVisible(state);
            MS_ANGEL_DEBUG("VGUIFrame::SetMinimizeButton(%d)", state);
        }
    }

    void VGUIFrame::Center()
    {
        if (!IsValid()) return;

        // TODO: vgui::App::getSize not available in this VGUI version
        // Using default screen size for now
        int screenWide = 640, screenTall = 480;

        int wide, tall;
        m_pPanel->getSize(wide, tall);

        int x = (screenWide - wide) / 2;
        int y = (screenTall - tall) / 2;

        m_pPanel->setPos(x, y);
        MS_ANGEL_DEBUG("VGUIFrame::Center() - positioned at (%d, %d)", x, y);
    }

    void VGUIFrame::Activate()
    {
        if (!IsValid()) return;

        // Make the frame visible
        m_pPanel->setVisible(true);

        // Request keyboard focus
        m_pPanel->requestFocus();

        // Force a repaint to ensure visibility
        m_pPanel->repaint();

        MS_ANGEL_INFO("VGUIFrame::Activate() - frame activated and visible");
    }

    //==========================================================================
    // VGUIButton Implementation
    //==========================================================================

    // Custom button class to handle callbacks
    class ASButton : public vgui::Button
    {
    private:
        VGUIButton* m_pWrapper;

    public:
        ASButton(const char* text, int x, int y, int wide, int tall)
            : Button(text, x, y, wide, tall), m_pWrapper(nullptr)
        {
        }

        void SetWrapper(VGUIButton* wrapper)
        {
            m_pWrapper = wrapper;
        }

        virtual void fireActionSignal()
        {
            // Call the wrapper's callback handler
            if (m_pWrapper)
            {
                m_pWrapper->OnButtonPressed();
            }
        }
    };

    VGUIButton::VGUIButton()
        : VGUIPanel(), m_CallbackFunction("")
    {
        ASButton* pButton = new ASButton("", 0, 0, 64, 24);
        pButton->SetWrapper(this);
        m_pPanel = pButton;
        m_bOwnPanel = true;
        MS_ANGEL_DEBUG("VGUIButton::VGUIButton() - created button: %p", m_pPanel);
    }

    VGUIButton::VGUIButton(const char* text, int x, int y, int wide, int tall)
        : VGUIPanel(), m_CallbackFunction("")
    {
        ASButton* pButton = new ASButton(text, x, y, wide, tall);
        pButton->SetWrapper(this);
        m_pPanel = pButton;
        m_bOwnPanel = true;
        MS_ANGEL_DEBUG("VGUIButton::VGUIButton(\"%s\", %d, %d, %d, %d)", text, x, y, wide, tall);
    }

    VGUIButton::~VGUIButton()
    {
        MS_ANGEL_DEBUG("VGUIButton::~VGUIButton()");
    }

    void VGUIButton::SetText(const std::string& text)
    {
        if (!IsValid()) return;
        vgui::Button* pButton = dynamic_cast<vgui::Button*>(m_pPanel);
        if (pButton)
        {
            pButton->setText(text.c_str());
            MS_ANGEL_DEBUG("VGUIButton::SetText(\"%s\")", text.c_str());
        }
    }

    void VGUIButton::SetCallback(const std::string& functionName)
    {
        m_CallbackFunction = functionName;
        MS_ANGEL_DEBUG("VGUIButton::SetCallback(\"%s\")", functionName.c_str());
    }

    void VGUIButton::SetEnabled(bool state)
    {
        if (!IsValid()) return;
        vgui::Button* pButton = dynamic_cast<vgui::Button*>(m_pPanel);
        if (pButton)
        {
            pButton->setEnabled(state);
            MS_ANGEL_DEBUG("VGUIButton::SetEnabled(%d)", state);
        }
    }

    void VGUIButton::SetArmedColor(int r, int g, int b, int a)
    {
        if (!IsValid()) return;
        // Note: setArmedColor is not available in base VGUI
        // Using setFgColor as a workaround for hover effects
        vgui::Button* pButton = dynamic_cast<vgui::Button*>(m_pPanel);
        if (pButton)
        {
            pButton->setFgColor(r, g, b, a);
            MS_ANGEL_DEBUG("VGUIButton::SetArmedColor(%d, %d, %d, %d) - using setFgColor", r, g, b, a);
        }
    }

    void VGUIButton::SetDepressedColor(int r, int g, int b, int a)
    {
        if (!IsValid()) return;
        // Note: setDepressedColor is not available in base VGUI
        // Using setBgColor as a workaround for pressed effects
        vgui::Button* pButton = dynamic_cast<vgui::Button*>(m_pPanel);
        if (pButton)
        {
            pButton->setBgColor(r, g, b, a);
            MS_ANGEL_DEBUG("VGUIButton::SetDepressedColor(%d, %d, %d, %d) - using setBgColor", r, g, b, a);
        }
    }

    void VGUIButton::OnButtonPressed()
    {
        if (m_CallbackFunction.empty())
        {
            MS_ANGEL_DEBUG("VGUIButton::OnButtonPressed() - no callback set");
            return;
        }

        MS_ANGEL_INFO("VGUIButton::OnButtonPressed() - calling: %s", m_CallbackFunction.c_str());

        // Get the active script context
        asIScriptContext* ctx = asGetActiveContext();
        if (!ctx)
        {
            MS_ANGEL_ERROR("VGUIButton::OnButtonPressed() - no active context");
            return;
        }

        // Get the engine from the context
        asIScriptEngine* engine = ctx->GetEngine();

        // Search all modules for the callback function
        asIScriptFunction* func = nullptr;
        asUINT moduleCount = engine->GetModuleCount();

        for (asUINT i = 0; i < moduleCount; i++)
        {
            asIScriptModule* module = engine->GetModuleByIndex(i);
            if (!module)
                continue;

            func = module->GetFunctionByName(m_CallbackFunction.c_str());
            if (func)
            {
                MS_ANGEL_DEBUG("VGUIButton::OnButtonPressed() - found '%s' in module '%s'",
                              m_CallbackFunction.c_str(), module->GetName());
                break;
            }
        }

        if (!func)
        {
            MS_ANGEL_ERROR("VGUIButton::OnButtonPressed() - function '%s' not found in any module",
                          m_CallbackFunction.c_str());
            return;
        }

        // Create a new context for the callback
        asIScriptContext* callbackCtx = engine->CreateContext();
        if (!callbackCtx)
        {
            MS_ANGEL_ERROR("VGUIButton::OnButtonPressed() - failed to create context");
            return;
        }

        // Prepare and execute the function
        int r = callbackCtx->Prepare(func);
        if (r < 0)
        {
            MS_ANGEL_ERROR("VGUIButton::OnButtonPressed() - failed to prepare function");
            callbackCtx->Release();
            return;
        }

        r = callbackCtx->Execute();
        if (r != asEXECUTION_FINISHED)
        {
            if (r == asEXECUTION_EXCEPTION)
            {
                MS_ANGEL_ERROR("VGUIButton::OnButtonPressed() - exception: %s", callbackCtx->GetExceptionString());
            }
            else
            {
                MS_ANGEL_ERROR("VGUIButton::OnButtonPressed() - execution failed: %d", r);
            }
        }

        callbackCtx->Release();
    }

    //==========================================================================
    // VGUILabel Implementation
    //==========================================================================

    VGUILabel::VGUILabel()
        : VGUIPanel()
    {
        m_pPanel = new vgui::Label("");
        m_bOwnPanel = true;
        MS_ANGEL_DEBUG("VGUILabel::VGUILabel() - created label: %p", m_pPanel);
    }

    VGUILabel::VGUILabel(const char* text, int x, int y, int wide, int tall)
        : VGUIPanel()
    {
        vgui::Label* pLabel = new vgui::Label(text);
        pLabel->setBounds(x, y, wide, tall);
        m_pPanel = pLabel;
        m_bOwnPanel = true;
        MS_ANGEL_DEBUG("VGUILabel::VGUILabel(\"%s\", %d, %d, %d, %d)", text, x, y, wide, tall);
    }

    VGUILabel::~VGUILabel()
    {
        MS_ANGEL_DEBUG("VGUILabel::~VGUILabel()");
    }

    void VGUILabel::SetText(const std::string& text)
    {
        if (!IsValid()) return;
        vgui::Label* pLabel = dynamic_cast<vgui::Label*>(m_pPanel);
        if (pLabel)
        {
            pLabel->setText(text.c_str());
            MS_ANGEL_DEBUG("VGUILabel::SetText(\"%s\")", text.c_str());
        }
    }

    void VGUILabel::SetTextColor(int r, int g, int b, int a)
    {
        if (!IsValid()) return;
        vgui::Label* pLabel = dynamic_cast<vgui::Label*>(m_pPanel);
        if (pLabel)
        {
            pLabel->setFgColor(r, g, b, a);
            MS_ANGEL_DEBUG("VGUILabel::SetTextColor(%d, %d, %d, %d)", r, g, b, a);
        }
    }

    void VGUILabel::SetContentAlignment(int alignment)
    {
        if (!IsValid()) return;
        vgui::Label* pLabel = dynamic_cast<vgui::Label*>(m_pPanel);
        if (pLabel)
        {
            vgui::Label::Alignment vguiAlign;
            switch (alignment)
            {
                case 0: vguiAlign = vgui::Label::a_west; break;   // Left
                case 1: vguiAlign = vgui::Label::a_center; break; // Center
                case 2: vguiAlign = vgui::Label::a_east; break;   // Right
                default: vguiAlign = vgui::Label::a_west; break;
            }
            pLabel->setContentAlignment(vguiAlign);
            MS_ANGEL_DEBUG("VGUILabel::SetContentAlignment(%d)", alignment);
        }
    }

    //==========================================================================
    // VGUITextEntry Implementation
    //==========================================================================

    VGUITextEntry::VGUITextEntry()
        : VGUIPanel()
    {
        m_pPanel = new vgui::TextEntry("", 0, 0, 64, 24);
        m_bOwnPanel = true;
        MS_ANGEL_DEBUG("VGUITextEntry::VGUITextEntry() - created text entry: %p", m_pPanel);
    }

    VGUITextEntry::VGUITextEntry(int x, int y, int wide, int tall)
        : VGUIPanel()
    {
        m_pPanel = new vgui::TextEntry("", x, y, wide, tall);
        m_bOwnPanel = true;
        MS_ANGEL_DEBUG("VGUITextEntry::VGUITextEntry(%d, %d, %d, %d)", x, y, wide, tall);
    }

    VGUITextEntry::~VGUITextEntry()
    {
        MS_ANGEL_DEBUG("VGUITextEntry::~VGUITextEntry()");
    }

    void VGUITextEntry::SetText(const std::string& text)
    {
        if (!IsValid()) return;
        vgui::TextEntry* pEntry = dynamic_cast<vgui::TextEntry*>(m_pPanel);
        if (pEntry)
        {
            pEntry->setText(text.c_str(), text.length());
            MS_ANGEL_DEBUG("VGUITextEntry::SetText(\"%s\")", text.c_str());
        }
    }

    std::string VGUITextEntry::GetText() const
    {
        if (!IsValid()) return "";
        vgui::TextEntry* pEntry = dynamic_cast<vgui::TextEntry*>(m_pPanel);
        if (pEntry)
        {
            char buffer[256];
            pEntry->getText(0, buffer, sizeof(buffer));
            return std::string(buffer);
        }
        return "";
    }

    void VGUITextEntry::SetEditable(bool state)
    {
        if (!IsValid()) return;
        // TODO: setEditable not available in this VGUI version
        // TextEntry is always editable by default
        MS_ANGEL_DEBUG("VGUITextEntry::SetEditable(%d) - not implemented", state);
    }

    void VGUITextEntry::SetMaxLength(int maxLen)
    {
        // Note: VGUI TextEntry doesn't have a direct max length method
        // This would need custom implementation
        MS_ANGEL_DEBUG("VGUITextEntry::SetMaxLength(%d) - not implemented in base VGUI", maxLen);
    }

    void VGUITextEntry::SetMultiline(bool state)
    {
        // Note: For multiline, you'd typically use TextPanel instead
        MS_ANGEL_DEBUG("VGUITextEntry::SetMultiline(%d) - not supported, use TextPanel", state);
    }

    //==========================================================================
    // VGUIImagePanel Implementation
    //==========================================================================

    VGUIImagePanel::VGUIImagePanel()
        : VGUIPanel()
    {
        m_pPanel = new vgui::ImagePanel();
        m_bOwnPanel = true;
        MS_ANGEL_DEBUG("VGUIImagePanel::VGUIImagePanel() - created image panel: %p", m_pPanel);
    }

    VGUIImagePanel::VGUIImagePanel(const char* imagePath, int x, int y, int wide, int tall)
        : VGUIPanel()
    {
        vgui::ImagePanel* pImagePanel = new vgui::ImagePanel();
        pImagePanel->setBounds(x, y, wide, tall);
        // Note: Would need to load image here - depends on your image loading system
        m_pPanel = pImagePanel;
        m_bOwnPanel = true;
        MS_ANGEL_DEBUG("VGUIImagePanel::VGUIImagePanel(\"%s\", %d, %d, %d, %d)", imagePath, x, y, wide, tall);
    }

    VGUIImagePanel::~VGUIImagePanel()
    {
        MS_ANGEL_DEBUG("VGUIImagePanel::~VGUIImagePanel()");
    }

    void VGUIImagePanel::SetImage(const std::string& imagePath)
    {
        if (!IsValid()) return;
        // Image loading would depend on your specific implementation
        MS_ANGEL_DEBUG("VGUIImagePanel::SetImage(\"%s\") - needs image loading implementation", imagePath.c_str());
    }

    void VGUIImagePanel::SetScaleImage(bool state)
    {
        if (!IsValid()) return;
        // TODO: setScaleImage not available in this VGUI version
        MS_ANGEL_DEBUG("VGUIImagePanel::SetScaleImage(%d) - not implemented", state);
    }

    //==========================================================================
    // Factory Functions
    //==========================================================================

    VGUIFrame* CreateFrame(int x, int y, int wide, int tall, const std::string& title)
    {
        VGUIFrame* frame = new VGUIFrame(x, y, wide, tall, title.c_str());
        MS_ANGEL_INFO("CreateFrame(%d, %d, %d, %d, \"%s\") = %p", x, y, wide, tall, title.c_str(), frame);
        return frame;
    }

    VGUIButton* CreateButton(const std::string& text, int x, int y, int wide, int tall)
    {
        VGUIButton* button = new VGUIButton(text.c_str(), x, y, wide, tall);
        MS_ANGEL_INFO("CreateButton(\"%s\", %d, %d, %d, %d) = %p", text.c_str(), x, y, wide, tall, button);
        return button;
    }

    VGUILabel* CreateLabel(const std::string& text, int x, int y, int wide, int tall)
    {
        VGUILabel* label = new VGUILabel(text.c_str(), x, y, wide, tall);
        MS_ANGEL_INFO("CreateLabel(\"%s\", %d, %d, %d, %d) = %p", text.c_str(), x, y, wide, tall, label);
        return label;
    }

    VGUITextEntry* CreateTextEntry(int x, int y, int wide, int tall)
    {
        VGUITextEntry* entry = new VGUITextEntry(x, y, wide, tall);
        MS_ANGEL_INFO("CreateTextEntry(%d, %d, %d, %d) = %p", x, y, wide, tall, entry);
        return entry;
    }

    VGUIImagePanel* CreateImagePanel(const std::string& imagePath, int x, int y, int wide, int tall)
    {
        VGUIImagePanel* panel = new VGUIImagePanel(imagePath.c_str(), x, y, wide, tall);
        MS_ANGEL_INFO("CreateImagePanel(\"%s\", %d, %d, %d, %d) = %p", imagePath.c_str(), x, y, wide, tall, panel);
        return panel;
    }

    VGUIPanel* CreatePanel(int x, int y, int wide, int tall)
    {
        vgui::Panel* pPanel = new vgui::Panel(x, y, wide, tall);
        VGUIPanel* panel = new VGUIPanel(pPanel, true);
        MS_ANGEL_INFO("CreatePanel(%d, %d, %d, %d) = %p", x, y, wide, tall, panel);
        return panel;
    }

    //==========================================================================
    // Utility Functions
    //==========================================================================

    void GetScreenSize(int& wide, int& tall)
    {
        // TODO: vgui::App::getSize not available in this VGUI version
        wide = 640;
        tall = 480;
        MS_ANGEL_DEBUG("GetScreenSize() = %d x %d", wide, tall);
    }

    bool IsPanelValid(VGUIPanel* panel)
    {
        return panel && panel->IsValid();
    }

    VGUIPanel* GetViewportPanel()
    {
        // Static cached wrapper - created once, lives forever
        // This prevents multiple wrappers for the same viewport and fixes crashes
        static VGUIPanel* s_viewportWrapper = nullptr;

        // gViewPort is declared in hud.h and defined in vgui_teamfortressviewport.cpp
        if (!gViewPort)
        {
            MS_ANGEL_ERROR("GetViewportPanel() - gViewPort is NULL");
            return nullptr;
        }

        // Create wrapper only on first call
        if (s_viewportWrapper == nullptr)
        {
            // TeamFortressViewport inherits from vgui::Panel, so we can safely pass it
            // Wrap the viewport in a VGUIPanel (don't own it - it's managed by the engine)
            s_viewportWrapper = new VGUIPanel(gViewPort, false);
            s_viewportWrapper->AddRef();  // Keep alive permanently (never deleted)
            MS_ANGEL_INFO("GetViewportPanel() - created singleton wrapper %p for %p",
                          s_viewportWrapper, gViewPort);
        }

        return s_viewportWrapper;
    }

    //==========================================================================
    // AngelScript Registration
    //==========================================================================

    void RegisterVGUIPanel(asIScriptEngine* pEngine)
    {
        MS_ANGEL_INFO("[ASVGUIBindings] Registering VGUIPanel...");

        asbind20::ref_class<VGUIPanel>(pEngine, "VGUIPanel")
            .addref(&VGUIPanel::AddRef)
            .release(&VGUIPanel::Release)
            .method("bool IsValid()", &VGUIPanel::IsValid)
            .method("void SetPos(int, int)", &VGUIPanel::SetPos)
            .method("void SetSize(int, int)", &VGUIPanel::SetSize)
            .method("void SetBounds(int, int, int, int)", &VGUIPanel::SetBounds)
            .method("void SetVisible(bool)", &VGUIPanel::SetVisible)
            .method("bool IsVisible()", &VGUIPanel::IsVisible)
            .method("void SetParent(VGUIPanel@)", &VGUIPanel::SetParent)
            .method("void AddChild(VGUIPanel@)", &VGUIPanel::AddChild)
            .method("void RemoveChild(VGUIPanel@)", &VGUIPanel::RemoveChild)
            .method("void SetFgColor(int, int, int, int)", &VGUIPanel::SetFgColor)
            .method("void SetBgColor(int, int, int, int)", &VGUIPanel::SetBgColor)
            .method("void Repaint()", &VGUIPanel::Repaint)
            .method("void SetPaintEnabled(bool)", &VGUIPanel::SetPaintEnabled)
            .method("bool IsMouseOver()", &VGUIPanel::IsMouseOver)
            .method("void RequestFocus()", &VGUIPanel::RequestFocus)
            .method("void Remove()", &VGUIPanel::Remove);

        MS_ANGEL_INFO("VGUIPanel registration complete");
    }

    void RegisterVGUIFrame(asIScriptEngine* pEngine)
    {
        MS_ANGEL_INFO("[ASVGUIBindings] Registering VGUIFrame...");

        asbind20::ref_class<VGUIFrame>(pEngine, "VGUIFrame")
            .addref(&VGUIFrame::AddRef)
            .release(&VGUIFrame::Release)
            .method("void SetTitle(const string& in)", &VGUIFrame::SetTitle)
            .method("void SetMoveable(bool)", &VGUIFrame::SetMoveable)
            .method("void SetSizeable(bool)", &VGUIFrame::SetSizeable)
            .method("void SetCloseButton(bool)", &VGUIFrame::SetCloseButton)
            .method("void SetMinimizeButton(bool)", &VGUIFrame::SetMinimizeButton)
            .method("void Center()", &VGUIFrame::Center)
            .method("void Activate()", &VGUIFrame::Activate)
            .base<VGUIPanel>();

        MS_ANGEL_INFO("VGUIFrame registration complete");
    }

    void RegisterVGUIButton(asIScriptEngine* pEngine)
    {
        MS_ANGEL_INFO("[ASVGUIBindings] Registering VGUIButton...");

        asbind20::ref_class<VGUIButton>(pEngine, "VGUIButton")
            .addref(&VGUIButton::AddRef)
            .release(&VGUIButton::Release)
            .method("void SetText(const string& in)", &VGUIButton::SetText)
            .method("void SetCallback(const string& in)", &VGUIButton::SetCallback)
            .method("void SetEnabled(bool)", &VGUIButton::SetEnabled)
            .method("void SetArmedColor(int, int, int, int)", &VGUIButton::SetArmedColor)
            .method("void SetDepressedColor(int, int, int, int)", &VGUIButton::SetDepressedColor)
            .base<VGUIPanel>();

        MS_ANGEL_INFO("VGUIButton registration complete");
    }

    void RegisterVGUILabel(asIScriptEngine* pEngine)
    {
        MS_ANGEL_INFO("[ASVGUIBindings] Registering VGUILabel...");

        asbind20::ref_class<VGUILabel>(pEngine, "VGUILabel")
            .addref(&VGUILabel::AddRef)
            .release(&VGUILabel::Release)
            .method("void SetText(const string& in)", &VGUILabel::SetText)
            .method("void SetTextColor(int, int, int, int)", &VGUILabel::SetTextColor)
            .method("void SetContentAlignment(int)", &VGUILabel::SetContentAlignment)
            .base<VGUIPanel>();

        MS_ANGEL_INFO("VGUILabel registration complete");
    }

    void RegisterVGUITextEntry(asIScriptEngine* pEngine)
    {
        MS_ANGEL_INFO("[ASVGUIBindings] Registering VGUITextEntry...");

        asbind20::ref_class<VGUITextEntry>(pEngine, "VGUITextEntry")
            .addref(&VGUITextEntry::AddRef)
            .release(&VGUITextEntry::Release)
            .method("void SetText(const string& in)", &VGUITextEntry::SetText)
            .method("string GetText()", &VGUITextEntry::GetText)
            .method("void SetEditable(bool)", &VGUITextEntry::SetEditable)
            .method("void SetMaxLength(int)", &VGUITextEntry::SetMaxLength)
            .method("void SetMultiline(bool)", &VGUITextEntry::SetMultiline)
            .base<VGUIPanel>();

        MS_ANGEL_INFO("VGUITextEntry registration complete");
    }

    void RegisterVGUIImagePanel(asIScriptEngine* pEngine)
    {
        MS_ANGEL_INFO("[ASVGUIBindings] Registering VGUIImagePanel...");

        asbind20::ref_class<VGUIImagePanel>(pEngine, "VGUIImagePanel")
            .addref(&VGUIImagePanel::AddRef)
            .release(&VGUIImagePanel::Release)
            .method("void SetImage(const string& in)", &VGUIImagePanel::SetImage)
            .method("void SetScaleImage(bool)", &VGUIImagePanel::SetScaleImage)
            .base<VGUIPanel>();

        MS_ANGEL_INFO("VGUIImagePanel registration complete");
    }

    void RegisterFactoryFunctions(asIScriptEngine* pEngine)
    {
        MS_ANGEL_INFO("[ASVGUIBindings] Registering factory functions...");

        asbind20::global(pEngine)
            .function("VGUIFrame@ CreateFrame(int, int, int, int, const string& in)", CreateFrame)
            .function("VGUIButton@ CreateButton(const string& in, int, int, int, int)", CreateButton)
            .function("VGUILabel@ CreateLabel(const string& in, int, int, int, int)", CreateLabel)
            .function("VGUITextEntry@ CreateTextEntry(int, int, int, int)", CreateTextEntry)
            .function("VGUIImagePanel@ CreateImagePanel(const string& in, int, int, int, int)", CreateImagePanel)
            .function("VGUIPanel@ CreatePanel(int, int, int, int)", CreatePanel)
            .function("void GetScreenSize(int& out, int& out)", GetScreenSize)
            .function("VGUIPanel@ GetViewportPanel()", GetViewportPanel)
            .function("bool IsPanelValid(VGUIPanel@)", IsPanelValid);

        MS_ANGEL_INFO("Factory functions registration complete");
    }

    void RegisterAll(asIScriptEngine* pEngine)
    {
        if (!pEngine)
        {
            MS_ANGEL_ERROR("[ASVGUIBindings] NULL engine pointer");
            return;
        }

        MS_ANGEL_INFO("[ASVGUIBindings] Starting VGUI bindings registration...");

        // Initialize panel registry
        InitializePanelRegistry();

        // Register types in order (base classes first)
        RegisterVGUIPanel(pEngine);
        RegisterVGUIFrame(pEngine);
        RegisterVGUIButton(pEngine);
        RegisterVGUILabel(pEngine);
        RegisterVGUITextEntry(pEngine);
        RegisterVGUIImagePanel(pEngine);

        // Register factory functions
        RegisterFactoryFunctions(pEngine);

        MS_ANGEL_INFO("[ASVGUIBindings] VGUI bindings registration complete");
    }
}

#endif // CLIENT_DLL
