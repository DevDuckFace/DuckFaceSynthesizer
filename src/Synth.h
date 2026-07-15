#pragma once
#include <cmath>
#include <cstdint>

// ============================================================
//  DUCK FACE - Analog Bass Line Synthesizer (TD-3 style)
//  Synth voice: VCO (saw/square) -> 4-pole ladder LPF -> VCA
//  Envelope with Decay, Accent, Slide (glide), Sub Osc,
//  Distortion + Tone.
// ============================================================

enum class Waveform { Saw, Square };

struct SynthParams {
    float tune       = 0.5f;  // 0..1  (-12..+12 semitones)
    float cutoff     = 0.6f;  // 0..1
    float resonance  = 0.3f;  // 0..1
    float envMod     = 0.5f;  // 0..1  envelope -> cutoff amount
    float decay      = 0.4f;  // 0..1
    float accentAmt  = 0.5f;  // 0..1
    float volume     = 0.8f;  // 0..1
    float distortion = 0.3f;  // 0..1 drive
    float tone       = 0.7f;  // 0..1 post-dist lowpass
    float slideTime  = 0.3f;  // 0..1
    bool  distOn     = false;
    bool  subOsc     = false;
    Waveform wave    = Waveform::Saw;
};

class Synth {
public:
    void setSampleRate(float sr) { m_sr = sr; }

    // Trigger a note. note = MIDI note number.
    void noteOn(int note, bool accent, bool slide) {
        float freq = 440.0f * std::pow(2.0f, (note - 69) / 12.0f);
        m_targetFreq = freq;
        if (!slide || !m_gate) m_freq = freq;      // slide glides, normal jumps
        m_sliding = slide;
        m_accent  = accent;
        m_gate    = true;
        if (!slide) {                              // retrigger envelopes
            m_env    = 1.0f + (accent ? params.accentAmt * 0.8f : 0.0f);
            m_ampEnv = 1.0f;
        }
    }

    void noteOff() { m_gate = false; }
    void allOff()  { m_gate = false; m_env = 0; m_ampEnv = 0; }

    float process() {
        // ---- glide ----
        float glide = m_sliding ? std::pow(0.5f, 1.0f / (m_sr * (0.005f + params.slideTime * 0.15f)))
                                : 0.0f;
        m_freq = m_targetFreq + (m_freq - m_targetFreq) * glide;

        float tuneSemis = (params.tune - 0.5f) * 24.0f;
        float f = m_freq * std::pow(2.0f, tuneSemis / 12.0f);

        // ---- oscillator (polyblep) ----
        float dt = f / m_sr;
        m_phase += dt;
        if (m_phase >= 1.0f) m_phase -= 1.0f;
        float osc;
        if (params.wave == Waveform::Saw) {
            osc = 2.0f * m_phase - 1.0f;
            osc -= polyblep(m_phase, dt);
        } else {
            osc = (m_phase < 0.5f) ? 1.0f : -1.0f;
            osc += polyblep(m_phase, dt);
            float p2 = m_phase + 0.5f; if (p2 >= 1.0f) p2 -= 1.0f;
            osc -= polyblep(p2, dt);
        }
        // ---- sub oscillator (1 octave down, square) ----
        if (params.subOsc) {
            m_subPhase += dt * 0.5f;
            if (m_subPhase >= 1.0f) m_subPhase -= 1.0f;
            osc = 0.7f * osc + 0.5f * ((m_subPhase < 0.5f) ? 1.0f : -1.0f);
        }

        // ---- envelopes (exp decay) ----
        float decT = 0.03f + params.decay * 1.2f;                     // filter env
        float envCoef = std::pow(0.001f, 1.0f / (m_sr * decT));
        m_env *= envCoef;
        float ampT = m_gate ? 2.5f : 0.01f;                           // amp gate w/ release
        m_ampEnv *= std::pow(0.001f, 1.0f / (m_sr * ampT));
        if (m_gate && m_ampEnv < 0.9f) m_ampEnv = 0.9f;

        // ---- 4-pole ladder filter ----
        float acc = m_accent ? params.accentAmt : 0.0f;
        float cut = params.cutoff + params.envMod * m_env * 0.6f + acc * m_env * 0.3f;
        if (cut > 1.0f) cut = 1.0f;
        float fc = 40.0f * std::pow(2.0f, cut * 8.0f);                // 40Hz..10kHz
        if (fc > m_sr * 0.45f) fc = m_sr * 0.45f;
        float g = std::tan(3.14159265f * fc / m_sr);
        float G = g / (1.0f + g);
        float k = params.resonance * 4.2f;

        float x = osc - k * m_z4;
        x = std::tanh(x);                                             // soft clip feedback
        m_z1 += 2.0f * G * (x    - m_z1); 
        m_z2 += 2.0f * G * (m_z1 - m_z2);
        m_z3 += 2.0f * G * (m_z2 - m_z3);
        m_z4 += 2.0f * G * (m_z3 - m_z4);
        float out = m_z4;

        // ---- VCA ----
        float accBoost = m_accent ? (1.0f + params.accentAmt) : 1.0f;
        out *= m_ampEnv * accBoost;

        // ---- distortion + tone ----
        if (params.distOn) {
            float drive = 1.0f + params.distortion * 15.0f;
            out = std::tanh(out * drive) * 0.9f;
            float tf = 500.0f * std::pow(2.0f, params.tone * 4.5f);   // tone LPF
            float tg = std::tan(3.14159265f * tf / m_sr);
            float tG = tg / (1.0f + tg);
            m_toneZ += 2.0f * tG * (out - m_toneZ);
            out = m_toneZ;
        }
        return out * params.volume * 0.8f;
    }

    SynthParams params;

private:
    static float polyblep(float t, float dt) {
        if (t < dt)        { t /= dt; return t + t - t * t - 1.0f; }
        if (t > 1.0f - dt) { t = (t - 1.0f) / dt; return t * t + t + t + 1.0f; }
        return 0.0f;
    }
    float m_sr = 44100.0f;
    float m_phase = 0, m_subPhase = 0;
    float m_freq = 110.0f, m_targetFreq = 110.0f;
    bool  m_gate = false, m_accent = false, m_sliding = false;
    float m_env = 0, m_ampEnv = 0;
    float m_z1 = 0, m_z2 = 0, m_z3 = 0, m_z4 = 0, m_toneZ = 0;
};
