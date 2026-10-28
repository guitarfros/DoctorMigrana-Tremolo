// SPDX-License-Identifier: AGPL-3.0-only
// Copyright (C) 2026 GuitarFros
#include "PluginProcessor.h"
#include "PluginEditor.h"

BrownTremProcessor::BrownTremProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "STATE", createLayout())
{
    speedParam     = apvts.getRawParameterValue ("speed");
    intensityParam = apvts.getRawParameterValue ("intensity");
    presenceParam  = apvts.getRawParameterValue ("presence");
    bypassParam    = apvts.getRawParameterValue ("bypass");
}

juce::AudioProcessorValueTreeState::ParameterLayout BrownTremProcessor::createLayout()
{
    using namespace juce;
    AudioProcessorValueTreeState::ParameterLayout l;

    NormalisableRange<float> speedRange (0.5f, 12.0f, 0.01f);
    speedRange.setSkewForCentre (4.0f);

    l.add (std::make_unique<AudioParameterFloat> (ParameterID { "speed", 1 }, "Speed",
                                                  speedRange, 4.5f,
                                                  AudioParameterFloatAttributes().withLabel ("Hz")));
    l.add (std::make_unique<AudioParameterFloat> (ParameterID { "intensity", 1 }, "Intensity",
                                                  NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.5f));
    l.add (std::make_unique<AudioParameterFloat> (ParameterID { "presence", 1 }, "Presence",
                                                  NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.3f));
    l.add (std::make_unique<AudioParameterBool>  (ParameterID { "bypass", 1 }, "Bypass", false));
    return l;
}

void BrownTremProcessor::prepareToPlay (double sampleRate, int)
{
    sr = sampleRate;
    speedSm.reset (sr, 0.05);
    depthSm.reset (sr, 0.03);
    activeSm.reset (sr, 0.02);
    speedSm.setCurrentAndTargetValue (speedParam->load());
    depthSm.setCurrentAndTargetValue (intensityParam->load());
    activeSm.setCurrentAndTargetValue (bypassParam->load() > 0.5f ? 0.0f : 1.0f);
    presenceFilter.reset();
    lastPresenceDb = -1000.0f;
    phase = 0.0;
}

bool BrownTremProcessor::isBusesLayoutSupported (const BusesLayout& l) const
{
    const auto in = l.getMainInputChannelSet(), out = l.getMainOutputChannelSet();
    return in == out && (in == juce::AudioChannelSet::mono() || in == juce::AudioChannelSet::stereo());
}

void BrownTremProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int numCh = juce::jmin (2, buffer.getNumChannels());
    const int n = buffer.getNumSamples();

    speedSm.setTargetValue (speedParam->load());
    depthSm.setTargetValue (intensityParam->load());
    activeSm.setTargetValue (bypassParam->load() > 0.5f ? 0.0f : 1.0f);

    // Presence: o control do amp reduce a realimentacion negativa nos agudos
    // -> modelado como high-shelf (0..+9 dB a 3.2 kHz)
    const float presDb = presenceParam->load() * 9.0f;
    if (std::abs (presDb - lastPresenceDb) > 0.01f)
    {
        presenceFilter.setHighShelf (sr, 3200.0, 0.7, presDb);
        lastPresenceDb = presDb;
    }

    constexpr double twoPi = juce::MathConstants<double>::twoPi;

    for (int i = 0; i < n; ++i)
    {
        const float speed  = speedSm.getNextValue();
        const float depth  = depthSm.getNextValue();
        const float active = activeSm.getNextValue();

        phase += (double) speed / sr;
        if (phase >= 1.0) phase -= 1.0;

        // LFO unipolar 0..1 (oscilador de desfasamento ~ senoide)
        const float lfo = 0.5f * (1.0f + (float) std::sin (twoPi * phase));

        // Bias-vary: ao subir o LFO, as 6V6 vanse mais "frias"
        const float gain = 1.0f - 0.85f * depth * lfo;      // modulacion de ganho
        const float c    = 0.35f * depth * lfo;             // desprazamento de bias
        const float tc   = std::tanh (c);
        constexpr float k = 1.4f;                           // dureza da curva

        for (int ch = 0; ch < numCh; ++ch)
        {
            float* d = buffer.getWritePointer (ch);
            const float dry = d[i];

            // saturacion asimetrica que depende do bias
            float wet = (std::tanh (k * dry * gain + c) - tc) / k;
            wet = presenceFilter.process (ch, wet);

            d[i] = dry + active * (wet - dry);
        }
    }
}

juce::AudioProcessorEditor* BrownTremProcessor::createEditor() { return new BrownTremEditor (*this); }

void BrownTremProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, dest);
}

void BrownTremProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new BrownTremProcessor(); }
