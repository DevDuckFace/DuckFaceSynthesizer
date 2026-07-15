# DUCK FACE — Analog Bass Line Synthesizer (virtual)
![image alt](https://github.com/DevDuckFace/DuckFaceSynthesizer/blob/601cccafbe1534eb118ceecfeeac39ddc1ef8794/Screenshot_7.png)
[Download HERE](https://github.com/DevDuckFace/DuckFaceSynthesizer/releases/download/synth1.0/DuckFaceSynth-Setup-1.0.0.exe)

## Funcionalidades (baseadas no TD-3 / TD-3-MO)
- VCO analógico modelado (polyBLEP): ondas **Sawtooth** e **Square** (chave WAVEFORM)
- **Sub Oscillator** (uma oitava abaixo, estilo TD-3-MO)
- Filtro **ladder low-pass de 4 polos** ressonante com CUTOFF, RESONANCE, ENVELOPE (env mod) e DECAY
- **ACCENT** (intensidade nos passos acentuados — aumenta filtro + volume)
- **SLIDE** (glide entre notas, com knob SLIDE TIME estilo MO)
- **Distorção** integrada com knobs DISTORTION e TONE + chave ON/OFF
- **Sequenciador de 16 passos**, 8 patterns, com por passo: nota, gate, accent, slide
- Modos **PLAY / WRITE** (no WRITE, tocar uma tecla grava a nota e avança o passo, como o WRITE/NEXT)
- **CLEAR**, **RAND** (pattern aleatório), **TRANSPOSE ±**, seletor de **oitava**
- **TEMPO** (knob 60–240 BPM) + **TAP tempo**, START/STOP (ou barra de espaço)
- Teclado de 13 teclas (C..C) para audição (PLAY) ou escrita (WRITE)

## Como compilar (Windows)
Pré-requisitos: **CMake** e **Visual Studio** (Build Tools) — ou MinGW.

```
build.bat
```
O CMake baixa a raylib automaticamente. O executável fica em `build\Release\DuckFaceSynth.exe`.


## Controles
- Knobs: arraste vertical com o mouse ou use a roda do mouse.
- Passos: clique para selecionar (em WRITE, clique também liga/desliga o gate).
- Botões ACCENT / SLIDE / GATE editam o passo selecionado.
- ESPAÇO = start/stop.


