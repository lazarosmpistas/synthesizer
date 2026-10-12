cmake_minimum_required(VERSION 3.25)
project(Synthesizer VERSION 0.1.0 LANGUAGES C CXX)

if(NOT WIN32 OR NOT MSVC OR NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "This project currently requires Windows x64 and MSVC.")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
include(CTest)

add_executable(ToolchainSmoke tests/ToolchainSmoke.cpp)
target_compile_definitions(ToolchainSmoke PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
target_compile_options(ToolchainSmoke PRIVATE /W4 /WX /permissive-)
if(BUILD_TESTING)
    add_test(NAME environment.toolchain COMMAND ToolchainSmoke)
    set_tests_properties(environment.toolchain PROPERTIES TIMEOUT 15)
endif()

option(SYNTH_ENABLE_JUCE_SMOKE "Build the pinned JUCE module integration check" OFF)
option(SYNTH_BUILD_PLUGIN "Build the LAZ-SYNTH VST3 instrument and scaffold check" OFF)
if(SYNTH_ENABLE_JUCE_SMOKE OR SYNTH_BUILD_PLUGIN)
    file(READ "${CMAKE_CURRENT_SOURCE_DIR}/dependencies.json" dependency_lock)
    string(JSON juce_directory GET "${dependency_lock}" juce directory)
    string(JSON juce_version GET "${dependency_lock}" juce version)
    string(JSON juce_revision GET "${dependency_lock}" juce revision)
    set(juce_source "${CMAKE_CURRENT_SOURCE_DIR}/${juce_directory}")
    if(NOT EXISTS "${juce_source}/CMakeLists.txt" OR NOT EXISTS "${juce_source}/.git")
        message(FATAL_ERROR "Pinned JUCE is missing. Run scripts/Setup-Dependencies.ps1 first.")
    endif()
    find_package(Git REQUIRED)
    execute_process(COMMAND "${GIT_EXECUTABLE}" -c "safe.directory=${juce_source}" -C "${juce_source}" rev-parse HEAD
        RESULT_VARIABLE revision_result OUTPUT_VARIABLE actual_revision
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(NOT revision_result EQUAL 0 OR NOT actual_revision STREQUAL juce_revision)
        message(FATAL_ERROR "JUCE checkout does not match dependencies.json; existing files were preserved.")
    endif()
    execute_process(COMMAND "${GIT_EXECUTABLE}" -c "safe.directory=${juce_source}" -C "${juce_source}" status --porcelain
        RESULT_VARIABLE status_result OUTPUT_VARIABLE juce_changes
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(NOT status_result EQUAL 0 OR NOT juce_changes STREQUAL "")
        message(FATAL_ERROR "JUCE has local changes; inspect them before validating the pinned dependency.")
    endif()
    set(JUCE_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(JUCE_BUILD_EXTRAS OFF CACHE BOOL "" FORCE)
    add_subdirectory("${juce_source}" "${CMAKE_CURRENT_BINARY_DIR}/JUCE" EXCLUDE_FROM_ALL)
    get_directory_property(actual_juce_version DIRECTORY "${juce_source}" DEFINITION JUCE_VERSION)
    if(NOT actual_juce_version STREQUAL juce_version)
        message(FATAL_ERROR "Unexpected JUCE version: ${actual_juce_version}")
    endif()
endif()

if(SYNTH_ENABLE_JUCE_SMOKE)
    juce_add_console_app(JuceModuleSmoke PRODUCT_NAME "Synthesizer module check")
    target_sources(JuceModuleSmoke PRIVATE tests/JuceModuleSmoke.cpp)
    target_compile_definitions(JuceModuleSmoke PRIVATE
        JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0)
    target_link_libraries(JuceModuleSmoke PRIVATE
        juce::juce_audio_basics
        juce::juce_audio_processors
        juce::juce_dsp
        juce::juce_gui_basics
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags)
    if(BUILD_TESTING)
        add_test(NAME environment.juce-modules COMMAND JuceModuleSmoke)
        set_tests_properties(environment.juce-modules PROPERTIES TIMEOUT 30)
    endif()
endif()

if(SYNTH_BUILD_PLUGIN)
    # Keep these identity codes stable so saved host projects find the same plugin.
    juce_add_plugin(Synthesizer
        COMPANY_NAME "LAZ"
        BUNDLE_ID "com.lazaraudio.synthesizer"
        PLUGIN_MANUFACTURER_CODE Laza
        PLUGIN_CODE Syba
        PRODUCT_NAME "LAZ-SYNTH"
        PLUGIN_NAME "LAZ-SYNTH"
        DESCRIPTION "LAZ-SYNTH instrument foundation"
        FORMATS VST3
        IS_SYNTH TRUE
        NEEDS_MIDI_INPUT TRUE
        NEEDS_MIDI_OUTPUT FALSE
        IS_MIDI_EFFECT FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS FALSE
        VST3_CATEGORIES Instrument Synth
        COPY_PLUGIN_AFTER_BUILD FALSE)
    target_sources(Synthesizer PRIVATE
        Source/Plugin/PluginProcessor.cpp
        Source/Plugin/PluginProcessor.h
        Source/DSP/SawOscillator.h
        Source/DSP/SawVoiceEngine.cpp
        Source/DSP/SawVoiceEngine.h
        Source/UI/PluginEditor.cpp
        Source/UI/PluginEditor.h)
    target_include_directories(Synthesizer PUBLIC Source "${juce_source}/modules")
    target_compile_definitions(Synthesizer PUBLIC
        JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1
        JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0)
    target_link_libraries(Synthesizer PRIVATE juce::juce_audio_processors
        PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)

    if(BUILD_TESTING)
        juce_add_console_app(PluginScaffoldCheck PRODUCT_NAME "Synthesizer scaffold check")
        target_sources(PluginScaffoldCheck PRIVATE tests/PluginScaffoldCheck.cpp)
        target_link_libraries(PluginScaffoldCheck PRIVATE Synthesizer)
        add_test(NAME plugin.scaffold COMMAND PluginScaffoldCheck)
        set_tests_properties(plugin.scaffold PROPERTIES TIMEOUT 30)
    endif()
endif()
