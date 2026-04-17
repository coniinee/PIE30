cmake_minimum_required(VERSION 3.22)

project(Voix VERSION 1.2.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# -----------------------------------------------------------------------------
# JUCE location
#
# Preferred layout for GitHub:
#   Voix/
#   ├── CMakeLists.txt
#   ├── JUCE/                <- git submodule or local copy of JUCE
#   └── Source/
#
# Fallback usage:
#   cmake -B build -S . -DJUCE_DIR=/absolute/path/to/JUCE
# -----------------------------------------------------------------------------
set(JUCE_DIR "" CACHE PATH "Path to a JUCE checkout")

if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/JUCE/CMakeLists.txt")
    add_subdirectory(JUCE)
elseif (JUCE_DIR AND EXISTS "${JUCE_DIR}/CMakeLists.txt")
    add_subdirectory("${JUCE_DIR}" JUCE)
else ()
    message(FATAL_ERROR
        "JUCE was not found. Either:\n"
        "  1) add JUCE as a subfolder named 'JUCE', or\n"
        "  2) configure with -DJUCE_DIR=/absolute/path/to/JUCE")
endif ()

juce_add_gui_app(Voix
    PRODUCT_NAME "Voix"
    VERSION ${PROJECT_VERSION}
    COMPANY_NAME "ENSTA"
    BUNDLE_ID "fr.ensta.voix"
    MICROPHONE_PERMISSION_ENABLED TRUE
    MICROPHONE_PERMISSION_TEXT "Cette application utilise le microphone pour détecter les vocalisations dans le cadre d’un entraînement orthophonique."
)

juce_generate_juce_header(Voix)

target_sources(Voix
    PRIVATE
        Source/Main.cpp
        Source/MainComponent.cpp
        Source/AudioAnalyzer.cpp
        Source/SettingsPanel.cpp
        Source/TreeScene.cpp
        Source/WindmillScene.cpp
        Source/ButterflyScene.cpp
        Source/MainComponent.h
        Source/AudioAnalyzer.h
        Source/GameSession.h
        Source/SettingsPanel.h
        Source/SceneChrome.h
        Source/TreeScene.h
        Source/WindmillScene.h
        Source/ButterflyScene.h
)

target_compile_definitions(Voix
    PRIVATE
        JUCE_STRICT_REFCOUNTEDPOINTER=1
)

target_link_libraries(Voix
    PRIVATE
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_data_structures
        juce::juce_dsp
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# Some JUCE versions expose this module separately. Link it when available so the
# CMake setup stays as close as possible to the current .jucer configuration.
if (TARGET juce::juce_audio_processors_headless)
    target_link_libraries(Voix PRIVATE juce::juce_audio_processors_headless)
endif ()
