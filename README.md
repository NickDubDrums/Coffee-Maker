# Coffee Maker

> **Professional Mastering. Guaranteed Louder.**

Plugin VST3/AU per mastering engineers. Satura, alza di +4.5 dB e limita hard. Impilalo più volte per risultati sempre più devastanti.

---

## DSP Chain

```
Input → Gain (+4.5 dB) → SoftClipper (tanh, drive=2.5) → Limiter (−0.3 dBFS) → Output
```

Nessun parametro utente. Nessun controllo. Solo caffè.

---

## Build — Windows

Richiede: **CMake 3.22+**, **Visual Studio 2026**, **Git**

```bash
git clone https://github.com/TUO_USER/Coffee_Maker.git
cd Coffee_Maker

# Genera la soluzione VS2026 (scarica JUCE 8.0.8 automaticamente)
cmake --preset windows-vs2026

# Build da riga di comando
cmake --build build --config Release

# Oppure apri la soluzione in Visual Studio:
start build\Coffee_Maker.sln
```

Il VST3 finisce in: `build/Coffee_Maker_artefacts/Release/VST3/`

---

## Build — macOS

Richiede: **CMake 3.22+**, **Xcode**

```bash
cmake --preset macos-universal
cmake --build build-mac --config Release
```

Universal Binary (arm64 + x86_64), macOS 11.0+.  
VST3 + AU in: `build-mac/Coffee_Maker_artefacts/Release/`

---

## CI — GitHub Actions

Ogni push su `main` e ogni tag `v*` triggera il workflow macOS.  
L'artefatto `.zip` (VST3 + AU Universal) è scaricabile dalla tab **Actions**.

Per creare una release: `git tag v1.0.0 && git push --tags`

---

## Uso Consigliato

| Istanze nella catena | Risultato |
|---|---|
| 1× | +4.5 dB, leggera saturazione. Caldo. |
| 2× | +9 dB, saturazione visibile. Forse troppo. |
| 3× | Brick-wall. Sicuramente troppo. |
| 4×+ | Segnale piatto. Esattamente come volevi. |
