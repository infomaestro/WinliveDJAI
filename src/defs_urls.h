#pragma once
#include "version.h"

// Two-level macro to stringize the numeric version definitions to a version
// string. See https://gcc.gnu.org/onlinedocs/cpp/Stringizing.html for details.
#define TO_STR(x) #x
#define TO_VERSION_STR(major, minor) \
    TO_STR(major)                    \
    "." TO_STR(minor)

// Icons for the main window and dialogs
#define MIXXX_ICON_PATH ":/images/icons/scalable/apps/wldjai.svg"
#define MIXXX_LOGO_PATH ":/images/wldjai_logo_black.svg"

#define MIXXX_WEBSITE_URL       "https://www.promusicsoftware.com"
#define MIXXX_WEBSITE_SHORT_URL "https://www.promusicsoftware.com"
#define MIXXX_SUPPORT_URL       "https://www.promusicsoftware.com/news.php?news=250"
#define MIXXX_TRANSLATION_URL   "https://www.promusicsoftware.com"
#define MIXXX_DONATE_URL "https://www.promusicsoftware.com"
#define MIXXX_ADDONS_URL "https://www.promusicsoftware.com/news.php?news=249"  

#define MIXXX_CONTROLLER_FORUMS_URL \
    "https://www.promusicsoftware.com"

#define MIXXX_WIKI_URL "https://www.promusicsoftware.com/news.php?news=249"
#define MIXXX_WIKI_TROUBLESHOOTING_SOUND_URL \
    MIXXX_WIKI_URL
#define MIXXX_WIKI_HARDWARE_COMPATIBILITY_URL \
    MIXXX_WIKI_URL
#define MIXXX_WIKI_AUDIO_LATENCY_URL \
    MIXXX_WIKI_URL
#define MIXXX_WIKI_CONTROLLER_MAPPING_FORMAT_URL \
    MIXXX_WIKI_URL
#define MIXXX_WIKI_MIDI_SCRIPTING_URL \
    MIXXX_WIKI_URL

#define MIXXX_MANUAL_URL "https://www.promusicsoftware.com/news.php?news=249"                       \

#define MIXXX_MANUAL_SHORTCUTS_URL \
    MIXXX_MANUAL_URL 
#define MIXXX_MANUAL_COMMANDLINEOPTIONS_URL \
    MIXXX_MANUAL_URL
#define MIXXX_MANUAL_CONTROLLERS_URL \
    MIXXX_MANUAL_URL
#define MIXXX_MANUAL_CONTROLLERMANUAL_PREFIX \
    MIXXX_MANUAL_URL 
#define MIXXX_MANUAL_CONTROLLERMANUAL_SUFFIX ".html"
#define MIXXX_MANUAL_CONTROLS_URL \
    MIXXX_MANUAL_URL 
#define MIXXX_MANUAL_SOUND_URL \
    MIXXX_MANUAL_URL
#define MIXXX_MANUAL_LIBRARY_URL \
    MIXXX_MANUAL_URL 
#define MIXXX_MANUAL_CUE_MODES_URL \
    MIXXX_MANUAL_URL 
#define MIXXX_MANUAL_SYNC_MODES_URL \
    MIXXX_MANUAL_URL 
#define MIXXX_MANUAL_TRACK_SEARCH_URL \
    MIXXX_MANUAL_URL 
#define MIXXX_MANUAL_BEATS_URL \
    MIXXX_MANUAL_URL 
#define MIXXX_MANUAL_KEY_URL \
    MIXXX_MANUAL_URL
#define MIXXX_MANUAL_EQ_URL \
    MIXXX_MANUAL_URL 
#define MIXXX_MANUAL_BROADCAST_URL \
    MIXXX_MANUAL_URL 
#define MIXXX_MANUAL_VINYL_URL \
    MIXXX_MANUAL_URL 
#define MIXXX_MANUAL_VINYL_TROUBLESHOOTING_URL \
    MIXXX_MANUAL_URL 
#define MIXXX_MANUAL_SETTINGS_DIRECTORY_URL \
    MIXXX_MANUAL_URL 
#define MIXXX_MANUAL_SOUND_API_URL \
    MIXXX_MANUAL_URL
#define MIXXX_MANUAL_OUTPUT_AND_INPUT_DEVICES \
    MIXXX_MANUAL_URL
#define MIXXX_MANUAL_MIC_MONITOR_MODES_URL \
    MIXXX_MANUAL_URL
#define MIXXX_MANUAL_MIC_LATENCY_URL \
    MIXXX_MANUAL_URL


#define MIXXX_MANUAL_FILENAME   "WinliveDjAi-Manual.pdf"
#define MIXXX_KBD_SHORTCUTS_FILENAME "WinliveDjAi-Keyboard-Shortcuts.pdf"



#define WDJ_WINLIVEAI_URL "https://www.winlive.ai/"
