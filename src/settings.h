#pragma once
#include <string>

class Settings {
public:
    Settings();
    ~Settings();
    
    // Load settings from file
    bool loadFromFile(const std::string& filename = "settings.txt");
    
    // Save settings to file
    bool saveToFile(const std::string& filename = "settings.txt");
    
    // Settings properties
    bool isFullscreen() const { return m_fullscreen; }
    void setFullscreen(bool fullscreen) { m_fullscreen = fullscreen; }
    
    // Reset to defaults
    void resetToDefaults();
    
private:
    bool m_fullscreen = false;
    std::string m_filename;
    
    // Helper functions
    std::string trim(const std::string& str);
    bool parseBool(const std::string& value);
};
