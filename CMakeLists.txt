cmake_minimum_required(VERSION 3.22)

# macOS: binario universal (Intel + Apple Silicon). Debe ir ANTES de project().
if(APPLE)
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "Build architectures for macOS")
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum macOS version")
endif()

project(DoctorMigranaTremolo VERSION 0.1.0)

# Copia o plugin ao cartafol do sistema despois de compilar (desactivado no CI)
option(MIGRANA_COPY_AFTER_BUILD "Copy plugin to the system plugin folder after build" ON)

include(FetchContent)
# 8.0.11 ou posterior: inclúe o SDK de VST3 3.8 (licenza MIT)
FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.11)
FetchContent_MakeAvailable(JUCE)

# PLUGIN_NAME  = nome visible no host (co guión longo)
# PRODUCT_NAME = nome dos ficheiros (ASCII)
juce_add_plugin(DoctorMigranaTremolo
    COMPANY_NAME "GuitarFros"
    PLUGIN_MANUFACTURER_CODE Gfrs
    PLUGIN_CODE Mtrm
    FORMATS VST3 Standalone
    PLUGIN_NAME "Doctor Migrana – Tremolo"
    PRODUCT_NAME "Doctor Migrana - Tremolo"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    COPY_PLUGIN_AFTER_BUILD ${MIGRANA_COPY_AFTER_BUILD})

juce_add_binary_data(DoctorMigranaTremoloAssets SOURCES Assets/pedal.png Assets/knob_strip.png)

juce_generate_juce_header(DoctorMigranaTremolo)

target_sources(DoctorMigranaTremolo PRIVATE
    PluginProcessor.cpp PluginProcessor.h
    PluginEditor.cpp PluginEditor.h)

target_compile_definitions(DoctorMigranaTremolo PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

# Necesario en MSVC para que o nome con caracteres non ASCII compile ben
target_compile_options(DoctorMigranaTremolo PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/utf-8>)

target_link_libraries(DoctorMigranaTremolo
    PRIVATE DoctorMigranaTremoloAssets
            juce::juce_audio_utils
    PUBLIC juce::juce_recommended_config_flags
           juce::juce_recommended_lto_flags
           juce::juce_recommended_warning_flags)
