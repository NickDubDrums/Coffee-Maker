cmake_minimum_required(VERSION 3.22)
project(Coffee_Maker VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── ASIO SDK — deve essere settato PRIMA di includere JUCE ───────────────────
set(JUCE_ASIO_SDK_PATH "J:/Dub Studio/Programmi/SDKs/ASIO"
    CACHE PATH "Percorso all'ASIO SDK")

if(EXISTS "${JUCE_ASIO_SDK_PATH}/common/iasiodrv.h")
    message(STATUS "ASIO SDK trovato in: ${JUCE_ASIO_SDK_PATH}")
else()
    message(WARNING "ASIO SDK non trovato in ${JUCE_ASIO_SDK_PATH} — build senza ASIO")
endif()

# ── AAX SDK ───────────────────────────────────────────────────────────────────
set(AAX_SDK_PATH "J:/Dub Studio/Programmi/SDKs/aax-sdk-2-9-0"
    CACHE PATH "Percorso all'AAX SDK")

if(EXISTS "${AAX_SDK_PATH}/Interfaces/AAX.h")
    message(STATUS "AAX SDK trovato in: ${AAX_SDK_PATH}")
else()
    message(WARNING "AAX SDK non trovato in ${AAX_SDK_PATH} — build senza AAX")
endif()

# ── JUCE locale / FetchContent fallback (per CI macOS) ───────────────────────
set(JUCE_PATH "J:/Dub Studio/Programmi/juce-8.0.8/JUCE"
    CACHE PATH "Percorso a JUCE locale")

if(EXISTS "${JUCE_PATH}/CMakeLists.txt")
    message(STATUS "JUCE trovato in: ${JUCE_PATH}")
    add_subdirectory("${JUCE_PATH}" "${CMAKE_BINARY_DIR}/juce")
else()
    message(STATUS "JUCE locale non trovato — uso FetchContent")
    include(FetchContent)
    FetchContent_Declare(JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG        8.0.8
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(JUCE)
endif()

# ── AAX SDK path — deve essere chiamato DOPO aver incluso JUCE ────────────────
if(EXISTS "${AAX_SDK_PATH}/Interfaces/AAX.h")
    juce_set_aax_sdk_path("${AAX_SDK_PATH}")
endif()

# ── Plugin target ─────────────────────────────────────────────────────────────
juce_add_plugin(Coffee_Maker
    COMPANY_NAME             "Coffee Music"
    BUNDLE_ID                "com.CoffeeMusic.CoffeeMaker"
    PLUGIN_MANUFACTURER_CODE Coff
    PLUGIN_CODE              CfMk
    FORMATS                  VST3 AU AAX
    PRODUCT_NAME             "Coffee Maker"
    DESCRIPTION              "Professional Mastering. Guaranteed Louder."
    IS_SYNTH                 FALSE
    NEEDS_MIDI_INPUT         FALSE
    NEEDS_MIDI_OUTPUT        FALSE
    IS_MIDI_EFFECT           FALSE
    VST3_CATEGORIES          "Fx"
    AAX_CATEGORY             AAX_ePlugInCategory_Dynamics
    #PLUGIN_CHANNEL_CONFIGURATIONS "{1,1},{2,2}"
    COPY_PLUGIN_AFTER_BUILD  FALSE
)

juce_add_binary_data(CoffeeMakerAssets
    SOURCES assets/images/BG.png
)

target_sources(Coffee_Maker PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
)

target_compile_features(Coffee_Maker PRIVATE cxx_std_17)

target_compile_definitions(Coffee_Maker PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(Coffee_Maker PRIVATE
    CoffeeMakerAssets
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_gui_basics
    juce::juce_audio_processors
)