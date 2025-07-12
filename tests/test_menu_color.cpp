#include <cassert>
#include <string>
#include "gui/menu_color.hpp"
#include "util/log.hpp"

// Mock class to simulate clipboard functionality
class MockSDL {
public:
    static void SetClipboardText(const std::string& text) {
        clipboard_text = text;
    }

    static const std::string& GetClipboardText() {
        return clipboard_text;
    }

private:
    static std::string clipboard_text;
};

std::string MockSDL::clipboard_text = "";

// Helper function to simulate SDL_SetClipboardText
void SDL_SetClipboardText(const char* text) {
    MockSDL::SetClipboardText(text);
}

// Helper function to simulate SDL_GetClipboardText
const char* SDL_GetClipboardText() {
    return MockSDL::GetClipboardText().c_str();
}

// Test function for ColorMenu
void test_ColorMenu() {
    // Create a Color object and initialize ColorMenu
    Color color(1.f, 0.f, 0.f);  // Red color
    ColorMenu menu(&color);

    // Test copying to clipboard (RGB format)
    menu.menu_action(menu.get_item_by_id(MNID_COPY_CLIPBOARD_RGB));
    assert(MockSDL::GetClipboardText() == "rgb(1.000000,0.000000,0.000000)");

    // Test copying to clipboard (Hex format)
    menu.menu_action(menu.get_item_by_id(MNID_COPY_CLIPBOARD_HEX));
    assert(MockSDL::GetClipboardText() == "#ff0000");

    // Test pasting from clipboard (valid color)
    MockSDL::SetClipboardText("rgb(0.0,1.0,0.0)");  // Green color
    menu.menu_action(menu.get_item_by_id(MNID_PASTE_CLIPBOARD));
    assert(color == Color(0.f, 1.f, 0.f));  // Verify color is updated to green

    // Test pasting from clipboard (invalid color format)
    MockSDL::SetClipboardText("invalid color format");
    menu.menu_action(menu.get_item_by_id(MNID_PASTE_CLIPBOARD));
    assert(color != Color(1.f, 0.f, 0.f));  // Ensure color hasn't changed

    // Test saturation adjustment
    float old_alpha = color.alpha;
    menu.menu_action(menu.get_item_by_id(MNID_SATURATION));
    assert(color.alpha == old_alpha);  // Ensure alpha hasn't changed
    assert(color != Color(1.f, 0.f, 0.f));  // Ensure color was modified by saturation
}

int main() {
    test_ColorMenu();
    log_info << "All tests passed!" << std::endl;
    return 0;
}

/* EOF */
