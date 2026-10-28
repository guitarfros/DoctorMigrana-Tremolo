// SPDX-License-Identifier: AGPL-3.0-only
// Copyright (C) 2026 GuitarFros
#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

// Knobs renderizados (filmstrip): 48 frames en 8 columnas, cada un de 300x340 px.
// Imitan os knobs de baquelita da foto, pero xiran de verdade.
class FilmstripKnobLookAndFeel : public juce::LookAndFeel_V4
{
public:
    static constexpr int numFrames = 48, cols = 8, frameW = 300, frameH = 340;

    void setStrip (const juce::Image& i) { strip = i; }

    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h,
                           float pos, float startAngle, float endAngle, juce::Slider&) override;
    void drawToggleButton (juce::Graphics&, juce::ToggleButton&, bool, bool) override;

private:
    juce::Image strip;
};

class BrownTremEditor : public juce::AudioProcessorEditor
{
public:
    explicit BrownTremEditor (BrownTremProcessor&);
    ~BrownTremEditor() override;
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    BrownTremProcessor& proc;
    FilmstripKnobLookAndFeel laf;
    juce::Image background;
    juce::ComponentBoundsConstrainer constrainer;

    juce::Slider speed, intensity, presence;
    juce::ToggleButton footswitch;

    using SA = juce::AudioProcessorValueTreeState::SliderAttachment;
    using BA = juce::AudioProcessorValueTreeState::ButtonAttachment;
    std::unique_ptr<SA> speedA, intensityA, presenceA;
    std::unique_ptr<BA> bypassA;

    float scale() const { return (float) getWidth() / 1920.0f; }
    void setupKnob (juce::Slider&);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BrownTremEditor)
};
