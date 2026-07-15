#pragma once
#include <array>
#include <cstdlib>
#include "Synth.h"

// 16-step pattern sequencer, TD-3 style.
// 8 patterns x 16 steps. Each step: note, on/off, accent, slide.

struct Step {
    int  note   = 45;   // MIDI (A2)
    bool on     = false;
    bool accent = false;
    bool slide  = false;
};

struct Pattern {
    std::array<Step, 16> steps;
    int length = 16;
};

enum class SeqMode { PatternPlay, PatternWrite, TrackPlay, TrackWrite };

class Sequencer {
public:
    std::array<Pattern, 8> patterns;
    int   currentPattern = 0;
    int   currentStep    = -1;
    int   writeCursor    = 0;
    int   transpose      = 0;
    float tempo          = 120.0f;
    bool  running        = false;
    SeqMode mode         = SeqMode::PatternPlay;

    void start() { running = true; m_sampleCounter = 0; currentStep = -1; }
    void stop(Synth& synth) { running = false; currentStep = -1; synth.noteOff(); }
    void toggle(Synth& s) { running ? stop(s) : start(); }

    void clearPattern() { patterns[currentPattern] = Pattern{}; }

    void randomize() {
        Pattern& p = patterns[currentPattern];
        static const int scale[] = { 0, 2, 3, 5, 7, 8, 10 };
        for (auto& st : p.steps) {
            st.on     = (std::rand() % 100) < 70;
            st.note   = 45 + scale[std::rand() % 7] + 12 * (std::rand() % 2 - 1) + 12;
            st.accent = (std::rand() % 100) < 25;
            st.slide  = (std::rand() % 100) < 20;
        }
    }

    // Call per audio sample. Triggers synth on step boundaries.
    void process(Synth& synth, float sr) {
        if (!running) return;
        float samplesPerStep = sr * 60.0f / tempo / 4.0f;  // 16th notes
        if (m_sampleCounter <= 0.0f) {
            m_sampleCounter += samplesPerStep;
            advance(synth, samplesPerStep, sr);
        }
        m_sampleCounter -= 1.0f;
        // gate off at ~55% of step unless next step slides
        if (m_gateOffAt > 0 && --m_gateOffAt == 0) synth.noteOff();
    }

private:
    void advance(Synth& synth, float samplesPerStep, float sr) {
        (void)sr;
        Pattern& p = patterns[currentPattern];
        currentStep = (currentStep + 1) % p.length;
        const Step& st = p.steps[currentStep];
        const Step& nx = p.steps[(currentStep + 1) % p.length];
        if (st.on) {
            synth.noteOn(st.note + transpose, st.accent, st.slide);
            if (st.slide && nx.on) m_gateOffAt = 0;                       // tie
            else m_gateOffAt = (int)(samplesPerStep * 0.55f);
        } else {
            synth.noteOff();
            m_gateOffAt = 0;
        }
    }
    float m_sampleCounter = 0;
    int   m_gateOffAt = 0;
};
