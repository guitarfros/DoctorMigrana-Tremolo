// SPDX-License-Identifier: AGPL-3.0-only
// Copyright (C) 2026 GuitarFros
#pragma once
#include <JuceHeader.h>

// Tremolo inspirado no "vibrato" de bias do Fender Deluxe 6G3 (brownface):
// oscilador de desfasamento (aprox. senoidal) que move o bias das 6V6,
// o que modula o ganho E engade distorsion asimetrica dependente da fase.
class BrownTremProcessor : public juce::AudioProcessor
{
public:
    BrownTremProcessor();
    ~BrownTremProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout&) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override
    { return juce::String (juce::CharPointer_UTF8 ("Doctor Migrana \xe2\x80\x93 Tremolo")); }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    juce::AudioProcessorValueTreeState apvts;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

    // Biquad high-shelf (RBJ) sen alocacions -> seguro no fio de audio
    struct Biquad
    {
        float b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0;
        float z1[2] { 0, 0 }, z2[2] { 0, 0 };

        void setHighShelf (double sr, double f0, double q, double gainDb)
        {
            const double A  = std::pow (10.0, gainDb / 40.0);
            const double w  = juce::MathConstants<double>::twoPi * f0 / sr;
            const double cw = std::cos (w), sw = std::sin (w);
            const double alpha = sw / (2.0 * q);
            const double sq = 2.0 * std::sqrt (A) * alpha;
            const double a0 = (A + 1) - (A - 1) * cw + sq;
            b0 = (float) (A * ((A + 1) + (A - 1) * cw + sq) / a0);
            b1 = (float) (-2.0 * A * ((A - 1) + (A + 1) * cw) / a0);
            b2 = (float) (A * ((A + 1) + (A - 1) * cw - sq) / a0);
            a1 = (float) (2.0 * ((A - 1) - (A + 1) * cw) / a0);
            a2 = (float) (((A + 1) - (A - 1) * cw - sq) / a0);
        }
        float process (int ch, float x)
        {
            const float y = b0 * x + z1[ch];
            z1[ch] = b1 * x - a1 * y + z2[ch];
            z2[ch] = b2 * x - a2 * y;
            return y;
        }
        void reset() { z1[0] = z1[1] = z2[0] = z2[1] = 0; }
    };

    std::atomic<float>* speedParam     = nullptr;
    std::atomic<float>* intensityParam = nullptr;
    std::atomic<float>* presenceParam  = nullptr;
    std::atomic<float>* bypassParam    = nullptr;

    juce::SmoothedValue<float> speedSm, depthSm, activeSm;
    Biquad presenceFilter;
    float lastPresenceDb = -1000.0f;
    double phase = 0.0;
    double sr = 44100.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BrownTremProcessor)
};
