// SPDX-License-Identifier: AGPL-3.0-only
// Copyright (C) 2026 GuitarFros
#include "PluginEditor.h"
#include <BinaryData.h>

// Coordenadas sobre a imaxe orixinal (1920 x 1280)
namespace Layout
{
    // esquina superior esquerda de cada knob (sprite de 300 x 340)
    const juce::Point<float> speedO     { 432.0f,  42.0f };
    const juce::Point<float> intensityO { 797.0f,  47.0f };
    const juce::Point<float> presenceO  { 1165.0f, 47.0f };
    constexpr float knobW = 300.0f, knobH = 340.0f;

    const juce::Point<float> ledC       { 950.0f,  918.0f };
    constexpr float ledR = 20.0f;
    const juce::Point<float> footC      { 940.0f,  1070.0f };
    constexpr float footR = 95.0f;
}

void FilmstripKnobLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h,
                                                 float pos, float, float, juce::Slider&)
{
    if (! strip.isValid())
        return;

    const int idx = juce::jlimit (0, numFrames - 1, juce::roundToInt (pos * (float) (numFrames - 1)));
    const int sx = (idx % cols) * frameW;
    const int sy = (idx / cols) * frameH;

    g.setImageResamplingQuality (juce::Graphics::highResamplingQuality);
    g.drawImage (strip, x, y, w, h, sx, sy, frameW, frameH);
}

void FilmstripKnobLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& btn, bool, bool down)
{
    if (down)   // lixeira sombra ao pisar o footswitch
    {
        g.setColour (juce::Colours::black.withAlpha (0.25f));
        g.fillEllipse (btn.getLocalBounds().toFloat().reduced (btn.getWidth() * 0.12f));
    }
}

BrownTremEditor::BrownTremEditor (BrownTremProcessor& p)
    : AudioProcessorEditor (&p), proc (p)
{
    background = juce::ImageCache::getFromMemory (BinaryData::pedal_png, BinaryData::pedal_pngSize);
    laf.setStrip (juce::ImageCache::getFromMemory (BinaryData::knob_strip_png, BinaryData::knob_strip_pngSize));

    setLookAndFeel (&laf);
    setupKnob (speed);
    setupKnob (intensity);
    setupKnob (presence);

    speedA     = std::make_unique<SA> (proc.apvts, "speed",     speed);
    intensityA = std::make_unique<SA> (proc.apvts, "intensity", intensity);
    presenceA  = std::make_unique<SA> (proc.apvts, "presence",  presence);

    footswitch.setClickingTogglesState (true);
    footswitch.setMouseCursor (juce::MouseCursor::PointingHandCursor);
    footswitch.onClick = [this] { repaint(); };
    addAndMakeVisible (footswitch);
    bypassA = std::make_unique<BA> (proc.apvts, "bypass", footswitch);

    constrainer.setFixedAspectRatio (1.5);
    constrainer.setSizeLimits (480, 320, 1440, 960);
    setConstrainer (&constrainer);
    setResizable (true, true);
    setSize (720, 480);
}

BrownTremEditor::~BrownTremEditor() { setLookAndFeel (nullptr); }

void BrownTremEditor::setupKnob (juce::Slider& s)
{
    s.setSliderStyle (juce::Slider::RotaryVerticalDrag);
    s.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    s.setRotaryParameters (juce::MathConstants<float>::pi * 1.25f,
                           juce::MathConstants<float>::pi * 2.75f, true);
    s.setMouseCursor (juce::MouseCursor::UpDownResizeCursor);
    addAndMakeVisible (s);
}

void BrownTremEditor::paint (juce::Graphics& g)
{
    g.setImageResamplingQuality (juce::Graphics::highResamplingQuality);
    g.drawImage (background, getLocalBounds().toFloat(), juce::RectanglePlacement::stretchToFit);

    const float s = scale();
    const bool active = ! footswitch.getToggleState();
    const auto led = juce::Rectangle<float> (Layout::ledR * 2 * s, Layout::ledR * 2 * s)
                         .withCentre (Layout::ledC * s);

    if (! active)   // a foto ten o LED aceso: tapamolo cando hai bypass
    {
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff4a1612), led.getCentreX(), led.getCentreY(),
                                                 juce::Colour (0xff1c0606), led.getRight(), led.getBottom(), true));
        g.fillEllipse (led.reduced (led.getWidth() * 0.06f));
    }
    else
    {
        g.setColour (juce::Colour (0xffff2a1a).withAlpha (0.22f));
        g.fillEllipse (led.expanded (led.getWidth() * 0.5f));
    }
}

void BrownTremEditor::resized()
{
    const float s = scale();
    auto knobRect = [s] (juce::Point<float> origin)
    {
        return juce::Rectangle<float> (origin.x * s, origin.y * s,
                                       Layout::knobW * s, Layout::knobH * s).toNearestInt();
    };
    speed.setBounds     (knobRect (Layout::speedO));
    intensity.setBounds (knobRect (Layout::intensityO));
    presence.setBounds  (knobRect (Layout::presenceO));

    footswitch.setBounds (juce::Rectangle<float> (Layout::footR * 2 * s, Layout::footR * 2 * s)
                              .withCentre (Layout::footC * s).toNearestInt());
}
