cmake_minimum_required(VERSION 3.22)
project(DarkFunkDurcalSynt VERSION 2.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "macOS minimum")

# JUCE 8 (Starter licence: closed source allowed up to the revenue limit; see LICENSE-NOTES.md)
if(DEFINED JUCE_DIR)
    add_subdirectory(${JUCE_DIR} JUCE)
else()
    include(FetchContent)
    FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.10 GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(JUCE)
endif()

if(NOT DEFINED DFD_FORMATS)
    if(APPLE)
        set(DFD_FORMATS AU VST3 Standalone)
    else()
        if(WIN32)
            set(DFD_FORMATS VST3 Standalone)
        else()
            set(DFD_FORMATS VST3)
        endif()
    endif()
endif()

juce_add_plugin(DarkFunkDurcalSynt
    COMPANY_NAME "Polaris Sound Lab"
    COMPANY_WEBSITE "https://www.instagram.com/polaris.sound.lab/"
    BUNDLE_ID com.polarissoundlab.darkfunkdurcalsynt
    PRODUCT_NAME "Dark Funk Durcal Synt"
    PLUGIN_MANUFACTURER_CODE Plsl
    PLUGIN_CODE Dfds
    FORMATS ${DFD_FORMATS}
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_CATEGORIES Instrument Synth Drum
    AU_MAIN_TYPE kAudioUnitType_MusicDevice
    DESCRIPTION "Acid, breaks, rituals and warped funk")

juce_generate_juce_header(DarkFunkDurcalSynt)

juce_add_binary_data(DfdData SOURCES
    Resources/fonts/Cinzel.ttf
    Resources/fonts/BarlowCondensed-SemiBold.ttf
    Resources/fonts/BarlowCondensed-Bold.ttf)

target_sources(DarkFunkDurcalSynt PRIVATE Source/PluginProcessor.cpp Source/PluginEditor.cpp)
target_include_directories(DarkFunkDurcalSynt PRIVATE Source)
target_compile_definitions(DarkFunkDurcalSynt PUBLIC
    JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0 JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_ALSA=$<IF:$<PLATFORM_ID:Linux>,0,1> JUCE_JACK=0)
target_link_libraries(DarkFunkDurcalSynt PRIVATE DfdData
    juce::juce_audio_utils juce::juce_audio_processors juce::juce_gui_extra juce::juce_cryptography
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_lto_flags juce::juce_recommended_warning_flags)

# ---- host test (Linux / CI): the real processor and editor in a fake host
option(DFD_TESTS "build the host test" OFF)
if(DFD_TESTS)
    juce_add_console_app(DfdHostTest PRODUCT_NAME "DfdHostTest")
    juce_generate_juce_header(DfdHostTest)
    target_sources(DfdHostTest PRIVATE tests/host_test.cpp Source/PluginProcessor.cpp Source/PluginEditor.cpp)
    target_include_directories(DfdHostTest PRIVATE Source)
    target_compile_definitions(DfdHostTest PRIVATE JUCE_MODAL_LOOPS_PERMITTED=1 JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_ALSA=0 JUCE_JACK=0
        JucePlugin_Name="Dark Funk Durcal Synt" JucePlugin_IsSynth=1 JucePlugin_WantsMidiInput=1 JucePlugin_ProducesMidiOutput=0)
    target_link_libraries(DfdHostTest PRIVATE DfdData juce::juce_audio_utils juce::juce_audio_processors juce::juce_gui_extra juce::juce_cryptography juce::juce_recommended_config_flags)
endif()
