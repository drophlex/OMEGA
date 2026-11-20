cmake_minimum_required(VERSION 3.22)
project(OMEGA VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "")

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/JUCE/CMakeLists.txt")
  include(FetchContent)
  FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.4 GIT_SHALLOW TRUE)
  FetchContent_MakeAvailable(JUCE)
else()
  add_subdirectory(JUCE)
endif()

juce_add_plugin(Omega
  COMPANY_NAME "Polaris Sound Lab"
  COMPANY_WEBSITE "https://github.com/drophlex/OMEGA"
  BUNDLE_ID "com.polarissoundlab.omega"
  PLUGIN_MANUFACTURER_CODE Plrs
  PLUGIN_CODE Omga
  FORMATS VST3 AU
  PRODUCT_NAME "Omega"
  IS_SYNTH FALSE
  NEEDS_MIDI_INPUT FALSE
  NEEDS_MIDI_OUTPUT FALSE
  IS_MIDI_EFFECT FALSE
  EDITOR_WANTS_KEYBOARD_FOCUS FALSE
  COPY_PLUGIN_AFTER_BUILD FALSE
  VST3_CATEGORIES Fx Mastering Dynamics
  AU_MAIN_TYPE kAudioUnitType_Effect
  DESCRIPTION "Mix-bus and mastering processor with honest metering")

target_sources(Omega PRIVATE Source/PluginProcessor.cpp Source/PluginEditor.cpp)
target_include_directories(Omega PRIVATE Source)
target_compile_definitions(Omega PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0 JUCE_DISPLAY_SPLASH_SCREEN=0)
target_compile_options(Omega PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/utf-8>)
target_link_libraries(Omega
  PRIVATE juce::juce_audio_utils juce::juce_dsp juce::juce_cryptography
  PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_lto_flags juce::juce_recommended_warning_flags)

option(OM_BUILD_TESTS "Build offline test" OFF)
if(OM_BUILD_TESTS)
  juce_add_console_app(OmegaTest PRODUCT_NAME "OmegaTest")
  target_sources(OmegaTest PRIVATE Tests/OmegaTest.cpp Source/PluginProcessor.cpp Source/PluginEditor.cpp)
  target_include_directories(OmegaTest PRIVATE Source)
  target_compile_definitions(OmegaTest PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_MODAL_LOOPS_PERMITTED=1)
  target_link_libraries(OmegaTest PRIVATE juce::juce_audio_utils juce::juce_dsp juce::juce_cryptography juce::juce_recommended_config_flags)
endif()
