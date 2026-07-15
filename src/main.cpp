// ============================================================
//  DUCK FACE - Analog Bass Line Synthesizer
//  TD-3 Yellow style GUI with graffiti duck face.
//  raylib GUI + audio.
// ============================================================
#include "raylib.h"
#include "Synth.h"
#include "Sequencer.h"
#include <cstring>
#include <cstdio>
#include <string>
#include <mutex>
#include <cmath>

static const int W = 1280, H = 720;

// ---------- global audio state ----------
static Synth      gSynth;
static Sequencer  gSeq;
static std::mutex gLock;
static const float SR = 44100.0f;

static void SynthAudioCallback(void* buffer, unsigned int frames) {
    short* out = (short*)buffer;
    std::lock_guard<std::mutex> lk(gLock);
    for (unsigned int i = 0; i < frames; i++) {
        gSeq.process(gSynth, SR);
        float s = gSynth.process();
        if (s > 1) s = 1; if (s < -1) s = -1;
        out[i] = (short)(s * 32000);
    }
}

// ---------- colors ----------
static const Color YEL      = { 255, 179, 0, 255 };
static const Color YEL_DK   = { 214, 143, 0, 255 };
static const Color INK      = { 30, 24, 8, 255 };
static const Color KNOB_SIL = { 225, 225, 228, 255 };
static const Color LED_ON   = { 255, 40, 40, 255 };
static const Color LED_OFF  = { 90, 30, 30, 255 };
static const Color BTN_W    = { 238, 238, 235, 255 };
static const Color BTN_B    = { 25, 25, 25, 255 };

// ---------- widgets ----------
struct Knob { float* val; Rectangle r; const char* label; };
static Knob* gActiveKnob = nullptr;

static void DrawKnob(Knob& k) {
    Vector2 c = { k.r.x + k.r.width / 2, k.r.y + k.r.height / 2 };
    float rad = k.r.width / 2;
    // ticks
    for (int i = 0; i <= 10; i++) {
        float a = (135 + i * 27) * DEG2RAD;
        DrawLineEx({ c.x + std::cos(a) * (rad + 4), c.y + std::sin(a) * (rad + 4) },
                   { c.x + std::cos(a) * (rad + 9), c.y + std::sin(a) * (rad + 9) }, 2, INK);
    }
    DrawCircleV(c, rad, YEL_DK);
    DrawCircleV(c, rad - 3, KNOB_SIL);
    float a = (135 + *k.val * 270) * DEG2RAD;
    DrawLineEx(c, { c.x + std::cos(a) * (rad - 6), c.y + std::sin(a) * (rad - 6) }, 3, INK);
    int tw = MeasureText(k.label, 13);
    DrawText(k.label, (int)(c.x - tw / 2), (int)(k.r.y - 20), 13, INK);

    Vector2 m = GetMousePosition();
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointCircle(m, c, rad + 6))
        gActiveKnob = &k;
    if (gActiveKnob == &k) {
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            *k.val -= GetMouseDelta().y * 0.006f;
            if (*k.val < 0) *k.val = 0; if (*k.val > 1) *k.val = 1;
        } else gActiveKnob = nullptr;
    }
    if (CheckCollisionPointCircle(m, c, rad + 6)) {
        float w = GetMouseWheelMove();
        if (w != 0) { *k.val += w * 0.04f; if (*k.val < 0) *k.val = 0; if (*k.val > 1) *k.val = 1; }
    }
}

static bool Button(Rectangle r, const char* label, Color col, bool ledOn = false, bool hasLed = false) {
    Vector2 m = GetMousePosition();
    bool hover = CheckCollisionPointRec(m, r);
    bool click = hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    Color c = col;
    if (hover) c = ColorBrightness(col, 0.15f);
    DrawRectangleRounded(r, 0.2f, 4, INK);
    DrawRectangleRounded({ r.x + 2, r.y + 2, r.width - 4, r.height - 4 }, 0.2f, 4, c);
    if (hasLed) DrawCircle((int)(r.x + r.width / 2), (int)(r.y - 10), 5, ledOn ? LED_ON : LED_OFF);
    if (label && label[0]) {
        int tw = MeasureText(label, 12);
        DrawText(label, (int)(r.x + r.width / 2 - tw / 2), (int)(r.y + r.height + 5), 12, INK);
    }
    return click;
}

static void Toggle(Rectangle r, bool* v, const char* off, const char* on) {
    Vector2 m = GetMousePosition();
    if (CheckCollisionPointRec(m, r) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) *v = !*v;
    DrawRectangleRounded(r, 0.4f, 4, INK);
    float hw = r.width / 2;
    Rectangle t = { *v ? r.x + hw : r.x, r.y, hw, r.height };
    DrawRectangleRounded({ t.x + 2, t.y + 2, t.width - 4, t.height - 4 }, 0.4f, 4, KNOB_SIL);
    DrawText(off, (int)(r.x - MeasureText(off, 12) - 6), (int)(r.y + 3), 12, INK);
    DrawText(on, (int)(r.x + r.width + 6), (int)(r.y + 3), 12, INK);
}

// ---------- duck face graffiti ----------
static void DrawDuckFace(float cx, float cy) {
    Color g = { 15, 15, 15, 235 };
    // head outline (hand-drawn double stroke = graffiti feel)
    DrawCircleLines((int)cx, (int)cy, 62, g);
    DrawCircleLines((int)cx + 1, (int)cy + 1, 63, g);
    DrawCircleLines((int)cx - 1, (int)cy, 61, g);
    // eyes
    DrawCircleV({ cx - 22, cy - 18 }, 7, g);
    DrawCircleV({ cx + 22, cy - 18 }, 7, g);
    // duck bill (pouty kissy duck face)
    DrawEllipse((int)cx, (int)(cy + 18), 30, 13, g);
    DrawEllipse((int)cx, (int)(cy + 18), 24, 8, YEL);
    DrawLineEx({ cx - 24, cy + 18 }, { cx + 24, cy + 18 }, 3, g);
    // little cheek puffs
    DrawCircleLines((int)cx - 34, (int)cy + 14, 6, g);
    DrawCircleLines((int)cx + 34, (int)cy + 14, 6, g);
    // graffiti drip
    DrawRectangle((int)cx + 40, (int)cy + 44, 3, 14, g);
    DrawCircleV({ cx + 41.5f, cy + 60 }, 3, g);
    // tag text
    const char* t = "DUCK FACE";
    int fs = 30;
    int tw = MeasureText(t, fs);
    // rough graffiti: draw offset copies
    DrawText(t, (int)(cx - tw / 2 + 2), (int)(cy + 74 + 2), fs, Fade(g, 0.35f));
    DrawText(t, (int)(cx - tw / 2), (int)(cy + 74), fs, g);
    DrawLineEx({ cx - tw / 2.0f - 8, cy + 108 }, { cx + tw / 2.0f + 8, cy + 106 }, 3, g);
}

// ---------- main ----------
int main() {
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(W, H, "DUCK FACE - Analog Bass Line Synthesizer");
    SetTargetFPS(60);

    InitAudioDevice();
    gSynth.setSampleRate(SR);
    AudioStream stream = LoadAudioStream((unsigned)SR, 16, 1);
    SetAudioStreamCallback(stream, SynthAudioCallback);
    PlayAudioStream(stream);

    // default demo pattern (acid line)
    {
        int notes[16] = { 33,33,45,33,36,33,48,33, 33,45,33,40,33,52,36,33 };
        for (int i = 0; i < 16; i++) {
            Step& s = gSeq.patterns[0].steps[i];
            s.on = true; s.note = notes[i];
            s.accent = (i % 4 == 0); s.slide = (i == 6 || i == 13);
        }
    }

    SynthParams& P = gSynth.params;
    float tempoKnob = 0.4f;   // maps 60..240
    float slideKnob = P.slideTime;
    bool waveSquare = false, distOn = false, subOsc = false;

    Knob knobs[] = {
        { &P.tune,       { 60,  70, 56, 56 }, "TUNE" },
        { &P.cutoff,     { 170, 70, 56, 56 }, "CUTOFF" },
        { &P.resonance,  { 280, 70, 56, 56 }, "RESONANCE" },
        { &P.envMod,     { 390, 70, 56, 56 }, "ENVELOPE" },
        { &P.decay,      { 500, 70, 56, 56 }, "DECAY" },
        { &P.accentAmt,  { 610, 70, 56, 56 }, "ACCENT" },
        { &P.distortion, { 940, 70, 56, 56 }, "DISTORTION" },
        { &P.tone,       { 1050,70, 56, 56 }, "TONE" },
        { &P.volume,     { 1160,70, 56, 56 }, "LEVEL" },
        { &tempoKnob,    { 70, 210, 76, 76 }, "TEMPO" },
        { &slideKnob,    { 940, 210, 56, 56 }, "SLIDE TIME" },
    };

    const char* noteNames[13] = { "C","C#","D","D#","E","F","F#","G","G#","A","A#","B","C" };
    int noteSemis[13]         = {  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12 };
    int selectedStep = 0;
    int octave = 2; // base octave (C2 = MIDI 36)
    double lastTap = 0; 

    while (!WindowShouldClose()) {
        gSeq.tempo = 60.0f + tempoKnob * 180.0f;
        P.slideTime = slideKnob;
        P.wave   = waveSquare ? Waveform::Square : Waveform::Saw;
        P.distOn = distOn;
        P.subOsc = subOsc;

        // keyboard shortcuts
        if (IsKeyPressed(KEY_SPACE)) { std::lock_guard<std::mutex> lk(gLock); gSeq.toggle(gSynth); }

        BeginDrawing();
        ClearBackground({ 45, 40, 30, 255 });
        DrawRectangleRounded({ 15, 15, W - 30, H - 30 }, 0.03f, 8, YEL);
        DrawRectangleRoundedLines({ 15, 15, W - 30, H - 30 }, 0.03f, 8, INK);

        // header
        DrawText("DUCK FACE", 45, 28, 30, INK);
        DrawText("Computer Controlled", 230, 38, 18, INK);
        DrawText("Analog Bass Line Synthesizer", 950, 32, 20, INK);

        for (auto& k : knobs) DrawKnob(k);

        // duck face center
        DrawDuckFace(W / 2.0f + 90, 250);

        // sections
        DrawText("WAVEFORM", 190, 190, 14, INK);
        Toggle({ 195, 215, 60, 22 }, &waveSquare, "SAW", "SQR");
        DrawText("DISTORTION", 1035, 190, 14, INK);
        Toggle({ 1045, 215, 60, 22 }, &distOn, "OFF", "ON");
        DrawText("SUB OSC", 195, 260, 14, INK);
        Toggle({ 195, 282, 60, 22 }, &subOsc, "OFF", "ON");

        // mode selector
        DrawText("MODE", 330, 190, 14, INK);
        const char* modes[2] = { "PLAY", "WRITE" };
        for (int i = 0; i < 2; i++) {
            bool sel = (int)gSeq.mode == i;
            if (Button({ 320.0f + i * 75, 212, 68, 26 }, "", sel ? KNOB_SIL : BTN_W))
                gSeq.mode = (SeqMode)i;
            DrawText(modes[i], (int)(330 + i * 75), 218, 14, INK);
            if (sel) DrawRectangleLinesEx({ 320.0f + i * 75, 212, 68, 26 }, 2, LED_ON);
        }

        // transport
        {
            bool run = gSeq.running;
            if (Button({ 45, 560, 70, 70 }, "START/STOP", BTN_B, run, true)) {
                std::lock_guard<std::mutex> lk(gLock); gSeq.toggle(gSynth);
            }
            if (Button({ 45, 470, 70, 34 }, "CLEAR", BTN_W)) {
                std::lock_guard<std::mutex> lk(gLock); gSeq.clearPattern();
            }
            if (Button({ 130, 470, 70, 34 }, "RAND", BTN_W)) {
                std::lock_guard<std::mutex> lk(gLock); gSeq.randomize();
            }
            // TAP tempo
            if (Button({ 1160, 560, 70, 70 }, "TAP", BTN_B)) {
                double now = GetTime();
                if (lastTap > 0 && now - lastTap < 2.0)
                    gSeq.tempo = (float)(60.0 / (now - lastTap));
                if (gSeq.tempo < 60) gSeq.tempo = 60; if (gSeq.tempo > 240) gSeq.tempo = 240;
                tempoKnob = (gSeq.tempo - 60.0f) / 180.0f;
                lastTap = now;
            }
        }

        // pattern selector 1..8
        DrawText("PATTERN", 240, 560, 14, INK);
        for (int i = 0; i < 8; i++) {
            char lbl[4]; std::snprintf(lbl, 4, "%d", i + 1);
            bool sel = gSeq.currentPattern == i;
            if (Button({ 240.0f + i * 44, 582, 36, 44 }, lbl, sel ? KNOB_SIL : BTN_W, sel, true)) {
                std::lock_guard<std::mutex> lk(gLock); gSeq.currentPattern = i;
            }
        }

        // transpose + accent/slide edit for selected step
        DrawText("TIME MODE / EDIT", 640, 560, 14, INK);
        if (Button({ 640, 582, 60, 44 }, "TRANS -", BTN_W)) gSeq.transpose--;
        if (Button({ 710, 582, 60, 44 }, "TRANS +", BTN_W)) gSeq.transpose++;
        {
            std::lock_guard<std::mutex> lk(gLock);
            Step& st = gSeq.patterns[gSeq.currentPattern].steps[selectedStep];
            if (Button({ 785, 582, 60, 44 }, "ACCENT", BTN_W, st.accent, true)) st.accent = !st.accent;
            if (Button({ 855, 582, 60, 44 }, "SLIDE",  BTN_W, st.slide,  true)) st.slide  = !st.slide;
            if (Button({ 925, 582, 60, 44 }, "GATE",   BTN_W, st.on,     true)) st.on     = !st.on;
        }
        DrawText(TextFormat("TRANSPOSE: %+d  TEMPO: %d BPM", gSeq.transpose, (int)gSeq.tempo), 640, 640, 14, INK);
        if (Button({ 1000, 582, 50, 44 }, "OCT -", BTN_W)) { if (octave > 0) octave--; }
        if (Button({ 1060, 582, 50, 44 }, "OCT +", BTN_W)) { if (octave < 5) octave++; }
        DrawText(TextFormat("OCT %d", octave), 1005, 640, 14, INK);

        // ---- 16 step sequencer row ----
        DrawText("STEP SEQUENCER  (click a step to select, then use keyboard / edit buttons)", 45, 350, 14, INK);
        {
            std::lock_guard<std::mutex> lk(gLock);
            Pattern& pat = gSeq.patterns[gSeq.currentPattern];
            for (int i = 0; i < 16; i++) {
                Rectangle r = { 45.0f + i * 74, 372, 64, 50 };
                Step& st = pat.steps[i];
                bool playing = gSeq.currentStep == i && gSeq.running;
                Color c = st.on ? (st.accent ? Color{ 255,120,60,255 } : KNOB_SIL) : BTN_W;
                if (selectedStep == i) c = ColorBrightness(c, -0.15f);
                DrawRectangleRounded(r, 0.15f, 4, INK);
                DrawRectangleRounded({ r.x + 2, r.y + 2, r.width - 4, r.height - 4 }, 0.15f, 4, c);
                DrawCircle((int)(r.x + r.width / 2), (int)(r.y - 8), 5, playing ? LED_ON : LED_OFF);
                if (st.on) {
                    int n = st.note % 12, oc = st.note / 12 - 1;
                    DrawText(TextFormat("%s%d%s", noteNames[n], oc, st.slide ? "~" : ""), (int)r.x + 8, (int)r.y + 16, 16, INK);
                }
                DrawText(TextFormat("%d", i + 1), (int)r.x + 4, (int)(r.y + r.height + 4), 11, INK);
                if (CheckCollisionPointRec(GetMousePosition(), r) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    selectedStep = i;
                    if (gSeq.mode == SeqMode::PatternWrite) st.on = !st.on;
                }
            }
        }

        // ---- keyboard (13 keys) ----
        DrawText("PITCH MODE - click a key to set the selected step's note (WRITE) or audition (PLAY)", 45, 445, 13, INK);
        for (int i = 0; i < 13; i++) {
            bool black = (i==1||i==3||i==6||i==8||i==10);
            Rectangle r = { 240.0f + i * 52, black ? 462.0f : 470.0f, 44, black ? 60.0f : 76.0f };
            if (Button(r, noteNames[i], black ? BTN_B : BTN_W)) {
                int note = (octave + 1) * 12 + noteSemis[i];
                std::lock_guard<std::mutex> lk(gLock);
                if (gSeq.mode == SeqMode::PatternWrite) {
                    Step& st = gSeq.patterns[gSeq.currentPattern].steps[selectedStep];
                    st.on = true; st.note = note;
                    selectedStep = (selectedStep + 1) % 16;   // auto-advance like TD-3 WRITE/NEXT
                } else {
                    gSynth.noteOn(note, false, false);
                }
            }
        }
        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && gSeq.mode == SeqMode::PatternPlay && !gSeq.running) {
            std::lock_guard<std::mutex> lk(gLock); gSynth.noteOff();
        }

        DrawText("SPACE = start/stop   |   mouse drag or wheel = knobs", 45, 675, 13, INK);
        EndDrawing();
    }

    StopAudioStream(stream);
    UnloadAudioStream(stream);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}
