#!/usr/bin/env python3
# OMEGA manual text (English + Spanish). Run, then build_manual.py.
import json, pathlib

def chain(lang):
    t = {"en": ("Every track", "your effects", "★ CONSTELLATION STAR (optional)", "Master", "your EQ / compressor (optional)", "Ω OMEGA", "✦ CONSTELLATION SKY (optional, last)"),
         "es": ("Cada pista", "tus efectos", "★ CONSTELLATION ESTRELLA (opcional)", "Master", "tu EQ / compresor (opcional)", "Ω OMEGA", "✦ CONSTELLATION CIELO (opcional, al final)")}[lang]
    box = lambda txt, bg, fg="#0b0f24": f'<div style="background:{bg};color:{fg};border-radius:8px;padding:2.2mm 2.5mm;font-size:9pt;font-weight:700;text-align:center">{txt}</div>'
    ar = '<div style="color:#9aa3d6;font-weight:900;align-self:center;text-align:center">→</div>'
    return (f'<div style="display:grid;grid-template-columns:24mm 1fr 6mm 1.3fr;gap:2mm;margin:2mm 0">{box(t[0],"#0b0f24","#e8ecff")}{box(t[1],"#e1e3f4")}{ar}{box(t[2],"#58e1ff")}</div>'
            f'<div style="display:grid;grid-template-columns:24mm 1.2fr 6mm 1fr 6mm 1.3fr;gap:2mm;margin:2mm 0 4mm">{box(t[3],"#0b0f24","#e8ecff")}{box(t[4],"#e1e3f4")}{ar}{box(t[5],"#ffd166")}{ar}{box(t[6],"#e1e3f4")}</div>')

EN = {
"tag": "The last plug-in on your master.",
"sub": "Mix-bus and mastering processor · honest level-matched A/B · BS.1770 loudness & true peak<br>VST3 · AU · Mac & Windows · stereo to 7.1.4 · 15-day full trial · US$49",
"manual": "User manual", "footer": "User manual",
"sections": [
{"title": "What is OMEGA?", "html": """
<p>OMEGA finishes a mix in one plug-in: tone, multiband glue, harmonics, width, punch, clipping and a true-peak limiter — tuned to work together, so a mix sounds <b>bigger, clearer and louder</b> without getting harsh, thin or crushed. You turn one control (the galaxy), pick a character (a <b>Universe</b>) and set a loudness target.</p>
<p>What makes it different is that it <b>never hides anything</b>. Every stage shows what it is doing, the loudness is measured the way Spotify, YouTube and Apple measure it, and you can compare with your mix <b>at exactly the same loudness</b> — so you choose with your ears, not with volume.</p>
<h3>What people ask for in one-knob mix-bus processors — and what OMEGA does</h3>
<table><tr><th style="width:38%">Common complaint</th><th>OMEGA</th></tr>
<tr><td>"You can't tell what it's doing."</td><td><b>Signal Path</b>: 10 stages lit in real time with their numbers (dB of reduction per band, width %, clipping, limiting, true peak).</td></tr>
<tr><td>"RMS meters, no LUFS."</td><td>Full <b>BS.1770 / EBU R128</b> metering: momentary, short-term, integrated, loudness range, true peak (4× oversampled), PLR.</td></tr>
<tr><td>"Louder sounds better, so you can't judge it."</td><td><b>COMPARE</b> plays your mix without OMEGA at the same loudness. <b>Δ ONLY OMEGA</b> lets you hear exactly what it adds.</td></tr>
<tr><td>"Sometimes too hyped, dull or too wide."</td><td>Six <b>Universes</b> from transparent to heavy, a <b>Tone Compass</b> with one-click <b>Balance</b>, <b>Tame</b> that removes harshness only when it appears, and a <b>correlation guard</b> that stops the widener before your mix gets phasey.</td></tr>
<tr><td>"Not very versatile."</td><td>Simple view for speed, <b>Advanced</b> view with 12 controls, A/B snapshots, undo/redo, reference matching and stereo → 7.1.4.</td></tr>
<tr><td>"Needs an online account to run."</td><td>Offline licence: no account, no internet, no dongle.</td></tr>
<tr><td>"Expensive."</td><td><b>US$49</b> (bundle with CONSTELLATION US$69), with a <b>15-day full trial</b>.</td></tr></table>
<div class="box tip">Everything is in English by default; press <span class="key">ES</span> (top right) for Spanish.</div>
"""},
{"title": "Install and where it goes", "html": """
<h3>macOS (Apple Silicon and Intel)</h3>
<ol><li>Download <b>OMEGA-macOS.zip</b> from the latest release on GitHub and unzip it.</li>
<li>Right-click <b>install-mac.command</b> → <b>Open</b>. It installs the VST3 and AU in your user plug-in folders.</li>
<li>Ableton Live → <b>Settings → Plug-Ins</b> → <b>Use VST3 Plug-in System Folders</b> → <b>Rescan</b>. Logic Pro finds the AU automatically.</li></ol>
<h3>Windows 10 / 11</h3>
<ol><li>Download <b>OMEGA-Windows.zip</b>, unzip, double-click <b>install-windows.bat</b> → <b>Yes</b>.</li>
<li>Ableton Live → <b>Options → Preferences → Plug-Ins</b> → <b>Use VST3 Plug-in System Folders</b> → <b>Rescan</b>.</li></ol>
<h3>Where it goes</h3>
""" + chain("en") + """
<ul><li>OMEGA goes at the <b>end of your Master</b>. It can replace your whole master chain, or sit after your own EQ / compressor.</li>
<li>With CONSTELLATION: stars on the tracks, OMEGA on the Master, and the SKY right after OMEGA so its meters and surround recorder see the finished master.</li>
<li>Mix <b>into</b> OMEGA from the start (at a low amount) and raise it at the end — the mix decisions you make will already account for it.</li></ul>
<div class="box tip">Latency is about 1.6 ms (4× quality) and is reported to the DAW, which compensates it automatically.</div>
"""},
{"title": "Quick start (60 seconds)", "html": """
<img class="shot" src="{{omega_main.png}}"><div class="cap">The main window (simple view), Universe <b>Quasar</b> at 70 %.</div>
<ol><li><b>Auto input</b> (left): press it and play the loudest part of the song for a few seconds. OMEGA sets the input so your mix sits in the green sweet spot.</li>
<li><b>Universe</b> (top): pick a character — start with <b>Nebula</b> (warm glue) or <b>Quasar</b> (bright, modern).</li>
<li><b>The galaxy</b>: drag it up or down to set how much OMEGA works (Shift = fine). Double-click = 50 %.</li>
<li><b>Target</b> (right): <b>Streaming −14</b> for Spotify, YouTube, Tidal, Amazon and DistroKid. Press <b>Match target</b> and play the loudest part: OMEGA raises the loudness until it lands on the target, keeping the true peak under the ceiling.</li>
<li><b>COMPARE</b> (bottom): listen with and without OMEGA at the same loudness. Keep the version you prefer.</li></ol>
"""},
{"title": "The screen", "html": """
<table><tr><th style="width:26%">Area</th><th>What it does</th></tr>
<tr><td><b>Universes</b></td><td>Six characters. Each one sets the tone curve, glue, punch, harmonics, width, clipping and how much Tame listens.</td></tr>
<tr><td><b>A · B</b></td><td>Two complete settings. Click the other letter to switch; both are kept.</td></tr>
<tr><td><b>Undo · Redo</b></td><td>Every change, including Universe and assistants.</td></tr>
<tr><td><b>Trial (gold)</b></td><td>Days left; opens the licence window. Disappears once activated.</td></tr>
<tr><td><b>Input</b></td><td>Input gain, input meter (green zone = sweet spot −21 … −13 LUFS), Auto input.</td></tr>
<tr><td><b>Tone Compass</b></td><td>Long-term spectral tilt of the output: dark · balanced · bright, with <b>Balance it</b>.</td></tr>
<tr><td><b>Galaxy</b></td><td>The big control and a live picture of the sound (next page).</td></tr>
<tr><td><b>Signal Path</b></td><td>Every stage and what it is doing right now.</td></tr>
<tr><td><b>Loudness readout</b></td><td>Short-term (big), momentary, integrated, true peak max, loudness range, dynamics (PLR).</td></tr>
<tr><td><b>Target · Match target</b></td><td>Loudness goal and the assistant that reaches it.</td></tr>
<tr><td><b>Output · Ceiling · Loudness</b></td><td>Output trim, true-peak ceiling (default −1 dBTP) and limiter drive.</td></tr>
<tr><td><b>Output meter</b></td><td>Short-term LUFS with the target line in gold.</td></tr>
<tr><td><b>COMPARE · Δ</b></td><td>Level-matched bypass and difference listening.</td></tr>
<tr><td><b>ADVANCED</b></td><td>Opens the 12 detail controls, reference, quality and surround mastering.</td></tr></table>
"""},
{"title": "Reading the galaxy", "html": """
<p>The galaxy is not decoration: every part of it is a meter, so you can feel what OMEGA is doing from across the room.</p>
<table><tr><th style="width:32%">What you see</th><th>What it means</th></tr>
<tr><td><b>Rotation speed</b></td><td>Loudness (short-term). It slows down when the transport stops.</td></tr>
<tr><td><b>Brightness and size of the stars</b></td><td>Energy of the music.</td></tr>
<tr><td><b>Width of the spiral</b></td><td>Stereo width after OMEGA (it narrows if the correlation guard steps in).</td></tr>
<tr><td><b>Three orbits</b></td><td>Glue on low (orange), mid (gold) and high (blue). Thicker and brighter = more gain reduction.</td></tr>
<tr><td><b>Glow of the core</b></td><td>Harmonics (saturation) at work.</td></tr>
<tr><td><b>Black hole in the centre</b></td><td>The limiter: it grows with gain reduction. Small and calm = transparent master.</td></tr>
<tr><td><b>Gold ring</b></td><td>The OMEGA amount (0 – 100 %).</td></tr>
<tr><td><b>Colour</b></td><td>The selected Universe.</td></tr>
<tr><td><b>Horizon</b></td><td>The output spectrum; the gold dashed line is your reference song.</td></tr></table>
<div class="box tip">A healthy master: the orbits breathe gently, the black hole stays small, the spiral keeps its shape and the compass sits in the green.</div>
"""},
{"title": "Universes", "html": """
<table><tr><th style="width:22%">Universe</th><th style="width:30%">Character</th><th>Use it for</th></tr>
<tr><td><b>Nebula</b></td><td>Warm glue, smooth top</td><td>Funk, soul, R&amp;B, pop with vocals, anything that should feel expensive and relaxed.</td></tr>
<tr><td><b>Pulsar</b></td><td>Punch and groove</td><td>Drums-forward music: funk, breaks, house, rock. Transients come forward.</td></tr>
<tr><td><b>Quasar</b></td><td>Bright, wide, modern</td><td>Modern pop, hip-hop, trap, EDM toplines: the "radio-ready" sheen.</td></tr>
<tr><td><b>Supernova</b></td><td>Loud and dense</td><td>Club music and anything that must be loud. More clipping and glue.</td></tr>
<tr><td><b>Event Horizon</b></td><td>Dark, vintage, heavy</td><td>Lo-fi, dark techno, IDM, analog-style mixes — top end goes down, saturation up. Good when you always pull the highs down.</td></tr>
<tr><td><b>Pure Light</b></td><td>Transparent mastering</td><td>Acoustic, jazz, classical, ambient, or when the mix is already finished and only needs loudness.</td></tr></table>
<p>The Universe is a <b>starting point</b>: the Advanced controls add to it (0 = the Universe's own setting), so you can change Universe without losing your tweaks.</p>
"""},
{"title": "Input and loudness", "html": """
<h3>Gain staging</h3>
<p>Like any analog-modelled chain, OMEGA responds to how hard you hit it. The <b>green zone</b> of the input meter (−21 … −13 LUFS) is where the Universes were tuned. <b>Auto input</b> listens for 6 seconds and places your loudest part around −17 LUFS.</p>
<h3>Targets</h3>
<table><tr><th style="width:34%">Target</th><th>Use it for</th></tr>
<tr><td><b>Streaming −14</b></td><td>Spotify, YouTube, Tidal, Amazon, Deezer — and DistroKid (one file for every platform).</td></tr>
<tr><td><b>Apple Music −16</b></td><td>When you want to keep the most dynamics on Apple Music.</td></tr>
<tr><td><b>Loud −10</b></td><td>Competitive loudness for genres that live loud (they will be turned down on streaming).</td></tr>
<tr><td><b>Club −7</b></td><td>DJ promos and club systems.</td></tr></table>
<p><b>Match target</b> raises the limiter drive step by step while the loudest part plays, until the short-term loudness lands on the target (usually 6 – 15 s). The <b>Ceiling</b> (default −1 dBTP, the streaming recommendation) is never exceeded: tested at −14 LUFS with every Universe, the true peak stayed below −1.6 dBTP.</p>
<h3>The numbers</h3>
<table><tr><th style="width:30%">Reading</th><th>Meaning</th></tr>
<tr><td>Momentary / Short-term</td><td>Loudness over 0.4 s / 3 s (LUFS).</td></tr>
<tr><td>Integrated</td><td>Loudness of the whole song (gated, as platforms measure it). Press play from the start for a real value.</td></tr>
<tr><td>True peak (max)</td><td>Highest inter-sample peak (4× oversampled). Green = under the ceiling.</td></tr>
<tr><td>Range (LRA)</td><td>How much the loudness moves between parts (LU).</td></tr>
<tr><td>Dynamics (PLR)</td><td>True peak minus integrated: bigger = punchier, more open.</td></tr></table>
<p>Under the bottom buttons OMEGA tells you how much streaming will turn your song down — useful to decide whether more loudness is worth it.</p>
"""},
{"title": "Honest comparison: COMPARE, Δ, A/B", "html": """
<p>The biggest trap in mastering is that louder always sounds better for a few seconds. OMEGA removes that trap.</p>
<ul><li><b>COMPARE (level-matched)</b> plays your mix <b>without</b> OMEGA, raised to exactly the loudness OMEGA is producing, and perfectly time-aligned. Switch on and off: only the sound changes, not the volume. (Tested: the difference stays within 1 LU.)</li>
<li><b>Δ ONLY OMEGA</b> plays the difference — only what OMEGA adds or removes. Great to hear if Tame, Harmonics or the limiter are doing too much.</li>
<li><b>A · B</b> keep two complete settings (for example Nebula vs Quasar, or two loudness targets) to switch instantly.</li>
<li><b>Undo / Redo</b> for every change.</li></ul>
<div class="box warn">Remember to switch COMPARE and Δ off before you export.</div>
"""},
{"title": "The signal path", "html": """
<p>OMEGA processes in this order. The Signal Path strip shows each stage with its live value.</p>
<table><tr><th style="width:16%">Stage</th><th>What it does</th></tr>
<tr><td><b>IN</b></td><td>Input gain (dB).</td></tr>
<tr><td><b>EQ</b></td><td>Universe curve + your Low (90 Hz shelf), Mid (1.1 kHz), High (5.5 kHz shelf), Air (14 kHz) and the reference match.</td></tr>
<tr><td><b>TAME</b></td><td>Dynamic de-harsh around 4 kHz: it only cuts (up to 6 dB) when that region jumps out of the mix.</td></tr>
<tr><td><b>PUNCH</b></td><td>Transient shaper: brings the attack of drums forward without raising peaks much.</td></tr>
<tr><td><b>GLUE</b></td><td>3-band compression (crossovers 150 Hz and 3 kHz, phase-aligned), linked across channels, program-dependent release. Shows reduction for low / mid / high.</td></tr>
<tr><td><b>WIDTH</b></td><td>Widens only the sides above ~120 Hz; correlation guard; optional Bass mono.</td></tr>
<tr><td><b>HARM</b></td><td>Tape/tube-style harmonics, oversampled (no aliasing).</td></tr>
<tr><td><b>CLIP</b></td><td>Soft clipper that shaves the fastest peaks before the limiter, so the limiter works less.</td></tr>
<tr><td><b>LIMIT</b></td><td>Look-ahead limiter at 4× (up to 8×) the sample rate, adaptive release, no overshoot.</td></tr>
<tr><td><b>OUT</b></td><td>Output trim and final ceiling; the true peak is shown here.</td></tr></table>
"""},
{"title": "Advanced controls", "html": """
<img class="shot" style="width:84%;display:block;margin:0 auto" src="{{omega_adv.png}}"><div class="cap">ADVANCED view.</div>
<table><tr><th style="width:20%">Control</th><th style="width:16%">Range</th><th>What it does</th></tr>
<tr><td><b>Low · Mid · High · Air</b></td><td>±6 dB</td><td>Added to the Universe curve. If you always pull the highs down in your master, do it here.</td></tr>
<tr><td><b>Tame</b></td><td>−1 … +1</td><td>More or less harshness control than the Universe.</td></tr>
<tr><td><b>Punch · Glue · Harmonics</b></td><td>−1 … +1</td><td>More or less of each stage relative to the Universe.</td></tr>
<tr><td><b>Width</b></td><td>−1 … +1</td><td>Narrower or wider sides (the guard still protects mono).</td></tr>
<tr><td><b>Clip</b></td><td>−1 … +1</td><td>How much the clipper works before the limiter.</td></tr>
<tr><td><b>Bass mono</b></td><td>off · 20 – 300 Hz</td><td>Everything below this frequency becomes mono (club and vinyl safety).</td></tr>
<tr><td><b>Ref. match</b></td><td>0 … 1</td><td>How much of the reference song's tone to apply (see next page).</td></tr>
<tr><td><b>Quality</b></td><td>2× · 4× · 8×</td><td>Oversampling of harmonics, clipper and limiter. 4× is the default; 8× for the final export if you like.</td></tr></table>
"""},
{"title": "Tone Compass and reference match", "html": """
<h3>Tone Compass</h3>
<p>OMEGA measures the long-term balance of your output from the lows to the highs (octave bands 63 Hz – 8 kHz). Most finished commercial mixes fall in the green "balanced" zone. If the needle sits on <b>bright</b> or <b>dark</b>, press <b>Balance it</b>: OMEGA adjusts High and Low for you. It is a guide — trust your ears and your genre.</p>
<h3>Reference match</h3>
<ol><li>In ADVANCED press <b>Load reference…</b> or simply <b>drop a song</b> (WAV, AIFF, FLAC, MP3) on the window.</li>
<li>OMEGA analyses its tone and loudness; its curve appears as a gold dashed line on the horizon.</li>
<li>Play your song for a while, then raise <b>Ref. match</b>: a gentle 10-band EQ moves your tone toward the reference (limited to ±4 dB per band, so it never wrecks the mix).</li>
<li>Use COMPARE to check you really prefer it.</li></ol>
<div class="box tip">The reference is saved with your project.</div>
"""},
{"title": "Mastering surround files", "html": """
<p>OMEGA works on stereo, 5.1, 7.1, 5.1.4, 7.1.2 and 7.1.4 tracks (in DAWs with surround buses), with the dynamics linked across all channels and the LFE left alone. It can also master a <b>surround file</b> — for example the 5.1 / 7.1.4 WAV that CONSTELLATION's surround recorder writes from Ableton:</p>
<ol><li>Set OMEGA the way you like it on your stereo master.</li>
<li>ADVANCED → <b>Master a surround file…</b> → choose the WAV.</li>
<li>OMEGA processes it in the background with the same settings and writes <b>&lt;name&gt;_OMEGA.wav</b> next to it — same channels, same channel order, same length (latency compensated).</li></ol>
<p>That file is ready for Final Cut Pro (5.1 for YouTube) or for the Dolby Atmos Renderer (7.1.4 bed). This closes the gap of CONSTELLATION's surround export, which is built before your master chain.</p>
<div class="box tip">For surround use a quieter target than stereo (around −16 to −18 LUFS): surround and Atmos deliveries are mastered with more headroom.</div>
"""},
{"title": "Recipes", "html": """
<table><tr><th style="width:26%">Goal</th><th>Settings</th></tr>
<tr><td><b>Your usual chain in one plug-in</b> (lows up a little, mids to taste, highs down, compressor, limiter)</td><td>Universe <b>Nebula</b> 50 – 60 %, ADVANCED Low +1, High −1 to −2, Glue 0, target Streaming −14 → Match target.</td></tr>
<tr><td><b>Funk / disco</b></td><td><b>Pulsar</b> 60 %, Punch +0.3, Bass mono 100 Hz, Width 0.</td></tr>
<tr><td><b>Hip-hop / trap</b></td><td><b>Quasar</b> 70 %, Harmonics +0.2, Bass mono 120 Hz, Clip +0.2.</td></tr>
<tr><td><b>Techno / house for clubs</b></td><td><b>Supernova</b> 70 %, target Club −7 (and a −14 version for streaming), Bass mono 150 Hz.</td></tr>
<tr><td><b>Dark, IDM, lo-fi</b></td><td><b>Event Horizon</b> 60 %, Tame +0.3, Air −1.</td></tr>
<tr><td><b>Acoustic, ambient, film</b></td><td><b>Pure Light</b> 40 %, target Apple −16, Glue −0.3.</td></tr>
<tr><td><b>Release on DistroKid</b></td><td>Target Streaming −14, ceiling −1 dBTP, COMPARE / Δ off, export one stereo WAV (24-bit). (With CONSTELLATION: 3D Phones off.)</td></tr></table>
<div class="box tip">Mix into OMEGA at 30 – 40 % while you mix, then set the final amount and target at the end.</div>
"""},
{"title": "Trial and licence", "html": """
<p>OMEGA is <b>complete for 15 days</b> — every feature. After that, OMEGA passes your audio through untouched until you activate a licence; your sessions are never damaged.</p>
<img class="shot" style="width:76%;display:block;margin:0 auto" src="{{omega_licence.png}}"><div class="cap">The licence window (gold Trial button at the top).</div>
<table><tr><th style="width:28%">Licence</th><th></th></tr>
<tr><td><b>Price</b></td><td><b>US$49</b> once — or <b>US$69</b> for OMEGA + CONSTELLATION. No subscription.</td></tr>
<tr><td><b>Includes</b></td><td>One person, all your computers, Mac and Windows, VST3 and AU, every 1.x update.</td></tr>
<tr><td><b>Buy</b></td><td>Press the buy button (PayPal). Your key arrives by e-mail at the address you paid with.</td></tr>
<tr><td><b>Activate</b></td><td>Paste your e-mail and the whole key → <b>ACTIVATE</b>. Works offline, no account.</td></tr></table>
"""},
{"title": "Questions and credits", "html": """
<table><tr><th style="width:38%">Question</th><th>Answer</th></tr>
<tr><td>Is it only for the master?</td><td>It shines on the master, but it also works on drum, vocal or music buses (try Pulsar on drums, Nebula on vocals).</td></tr>
<tr><td>CPU?</td><td>About 4 % of one core at 4× quality in our tests. 8× doubles the oversampled part.</td></tr>
<tr><td>The loudness assistant doesn't reach the target.</td><td>Play the loudest part while it listens, and check the input is in the green zone (Auto input).</td></tr>
<tr><td>It sounds too bright / too dark.</td><td>Check the Tone Compass and press Balance it, or change Universe.</td></tr>
<tr><td>Can it pump?</td><td>Glue uses program-dependent release; if you hear pumping lower Glue or the amount, or set a lower target.</td></tr>
<tr><td>Does it work in Final Cut Pro?</td><td>Final Cut uses AU effects; for 5.1 video audio the simplest way is "Master a surround file" and import the result.</td></tr></table>
<h3>Credits</h3>
<p>Design by <b>Dark Funk Durcal</b> — <i>What is Dark Funk Durcal? Search it on Spotify, YouTube and every audio platform.</i><br>
Created at <b>Polaris Sound Lab</b>, Mérida, Yucatán, México · Instagram @polaris.sound.lab · YouTube @PolarisSoundLab · github.com/drophlex/OMEGA · Built with JUCE.</p>
"""},
]}

ES = {
"tag": "El último plug-in de tu master.",
"sub": "Procesador de mix bus y mastering · A/B honesto al mismo volumen · loudness BS.1770 y pico real<br>VST3 · AU · Mac y Windows · estéreo a 7.1.4 · prueba completa de 15 días · US$49",
"manual": "Manual de usuario", "footer": "Manual de usuario",
"sections": [
{"title": "¿Qué es OMEGA?", "html": """
<p>OMEGA termina una mezcla en un solo plug-in: tono, pegamento multibanda, armónicos, ancho, golpe, clipper y limitador de pico real, afinados para trabajar juntos, para que la mezcla suene <b>más grande, clara y fuerte</b> sin volverse áspera, delgada ni aplastada. Mueves un control (la galaxia), eliges un carácter (un <b>Universo</b>) y fijas un objetivo de volumen.</p>
<p>Lo que lo hace diferente es que <b>no esconde nada</b>. Cada etapa muestra lo que hace, el volumen se mide como lo miden Spotify, YouTube y Apple, y puedes comparar con tu mezcla <b>exactamente al mismo volumen</b>: eliges con tus oídos, no con el volumen.</p>
<h3>Lo que la gente pide a los procesadores de un solo knob, y lo que hace OMEGA</h3>
<table><tr><th style="width:38%">Queja común</th><th>OMEGA</th></tr>
<tr><td>"No se sabe qué está haciendo."</td><td><b>Recorrido de la señal</b>: 10 etapas iluminadas en tiempo real con sus números (dB de reducción por banda, % de ancho, clip, limitador, pico real).</td></tr>
<tr><td>"Medidores RMS, sin LUFS."</td><td>Medición completa <b>BS.1770 / EBU R128</b>: momentáneo, corto plazo, integrado, rango, pico real (sobremuestreo 4×) y PLR.</td></tr>
<tr><td>"Más fuerte suena mejor, así no se puede juzgar."</td><td><b>COMPARAR</b> reproduce tu mezcla sin OMEGA al mismo volumen. <b>Δ SOLO OMEGA</b> te deja oír exactamente lo que agrega.</td></tr>
<tr><td>"A veces muy brillante, opaco o demasiado ancho."</td><td>Seis <b>Universos</b> de transparente a pesado, <b>Brújula tonal</b> con <b>Equilibrar</b> en un clic, <b>Domar</b> que quita lo áspero solo cuando aparece y una <b>protección de fase</b> que detiene el ensanchador antes de que la mezcla se desfase.</td></tr>
<tr><td>"Poco versátil."</td><td>Vista simple para ir rápido, vista <b>Avanzada</b> con 12 controles, A/B, deshacer/rehacer, igualar referencia y de estéreo a 7.1.4.</td></tr>
<tr><td>"Pide una cuenta en línea para funcionar."</td><td>Licencia sin internet: sin cuenta, sin conexión, sin llave USB.</td></tr>
<tr><td>"Caro."</td><td><b>US$49</b> (paquete con CONSTELLATION US$69), con <b>15 días de prueba completa</b>.</td></tr></table>
<div class="box tip">Todo viene en inglés; presiona <span class="key">ES</span> (arriba a la derecha) para español.</div>
"""},
{"title": "Instalación y dónde va", "html": """
<h3>macOS (Apple Silicon e Intel)</h3>
<ol><li>Descarga <b>OMEGA-macOS.zip</b> del último release de GitHub y descomprímelo.</li>
<li>Clic derecho en <b>install-mac.command</b> → <b>Abrir</b>. Instala el VST3 y el AU en tus carpetas de plug-ins.</li>
<li>Ableton Live → <b>Configuración → Plug-Ins</b> → <b>Usar carpetas de sistema VST3</b> → <b>Re-escanear</b>. Logic Pro encuentra el AU solo.</li></ol>
<h3>Windows 10 / 11</h3>
<ol><li>Descarga <b>OMEGA-Windows.zip</b>, descomprime y haz doble clic en <b>install-windows.bat</b> → <b>Sí</b>.</li>
<li>Ableton Live → <b>Opciones → Preferencias → Plug-Ins</b> → <b>Usar carpetas de sistema VST3</b> → <b>Re-escanear</b>.</li></ol>
<h3>Dónde va</h3>
""" + chain("es") + """
<ul><li>OMEGA va al <b>final de tu Master</b>. Puede reemplazar toda tu cadena de master o ir después de tu propio EQ / compresor.</li>
<li>Con CONSTELLATION: estrellas en las pistas, OMEGA en el Master y el CIELO justo después de OMEGA, para que sus medidores y su grabador surround vean el master terminado.</li>
<li>Mezcla <b>hacia</b> OMEGA desde el inicio (con poca intensidad) y súbelo al final: tus decisiones de mezcla ya lo tomarán en cuenta.</li></ul>
<div class="box tip">La latencia es de unos 1.6 ms (calidad 4×) y se reporta al DAW, que la compensa solo.</div>
"""},
{"title": "Inicio rápido (60 segundos)", "html": """
<img class="shot" src="{{omega_main.png}}"><div class="cap">La ventana principal (vista simple), Universo <b>Cuásar</b> al 70 %.</div>
<ol><li><b>Entrada auto</b> (izquierda): presiónalo y reproduce la parte más fuerte de la canción unos segundos. OMEGA ajusta la entrada para que tu mezcla quede en la zona verde.</li>
<li><b>Universo</b> (arriba): elige un carácter; empieza con <b>Nebulosa</b> (pegamento cálido) o <b>Cuásar</b> (brillante, moderno).</li>
<li><b>La galaxia</b>: arrástrala arriba o abajo para la intensidad de OMEGA (Shift = fino). Doble clic = 50 %.</li>
<li><b>Objetivo</b> (derecha): <b>Streaming −14</b> para Spotify, YouTube, Tidal, Amazon y DistroKid. Presiona <b>Ajustar al objetivo</b> y reproduce la parte más fuerte: OMEGA sube el volumen hasta llegar al objetivo, con el pico real siempre bajo el techo.</li>
<li><b>COMPARAR</b> (abajo): escucha con y sin OMEGA al mismo volumen. Quédate con la versión que prefieras.</li></ol>
"""},
{"title": "La pantalla", "html": """
<table><tr><th style="width:26%">Zona</th><th>Qué hace</th></tr>
<tr><td><b>Universos</b></td><td>Seis caracteres. Cada uno fija la curva de tono, pegamento, golpe, armónicos, ancho, clip y cuánto escucha Domar.</td></tr>
<tr><td><b>A · B</b></td><td>Dos ajustes completos. Haz clic en la otra letra para cambiar; se guardan los dos.</td></tr>
<tr><td><b>Deshacer · Rehacer</b></td><td>Cada cambio, incluidos Universo y asistentes.</td></tr>
<tr><td><b>Prueba (dorado)</b></td><td>Días restantes; abre la ventana de licencia. Desaparece al activar.</td></tr>
<tr><td><b>Entrada</b></td><td>Ganancia de entrada, medidor (zona verde = −21 … −13 LUFS), Entrada auto.</td></tr>
<tr><td><b>Brújula tonal</b></td><td>Inclinación espectral de la salida a largo plazo: oscuro · equilibrado · brillante, con <b>Equilibrar</b>.</td></tr>
<tr><td><b>Galaxia</b></td><td>El control grande y una imagen viva del sonido (página siguiente).</td></tr>
<tr><td><b>Recorrido de la señal</b></td><td>Cada etapa y lo que está haciendo en este momento.</td></tr>
<tr><td><b>Lectura de volumen</b></td><td>Corto plazo (grande), momentáneo, integrado, pico real máximo, rango y dinámica (PLR).</td></tr>
<tr><td><b>Objetivo · Ajustar</b></td><td>Meta de volumen y el asistente que la alcanza.</td></tr>
<tr><td><b>Salida · Techo · Volumen</b></td><td>Ajuste de salida, techo de pico real (−1 dBTP por defecto) y empuje del limitador.</td></tr>
<tr><td><b>Medidor de salida</b></td><td>LUFS de corto plazo con la línea dorada del objetivo.</td></tr>
<tr><td><b>COMPARAR · Δ</b></td><td>Bypass al mismo volumen y escucha de la diferencia.</td></tr>
<tr><td><b>AVANZADO</b></td><td>Abre los 12 controles de detalle, referencia, calidad y master surround.</td></tr></table>
"""},
{"title": "Cómo leer la galaxia", "html": """
<p>La galaxia no es adorno: cada parte es un medidor, así que sientes lo que hace OMEGA desde el otro lado del cuarto.</p>
<table><tr><th style="width:32%">Lo que ves</th><th>Lo que significa</th></tr>
<tr><td><b>Velocidad de giro</b></td><td>Volumen (corto plazo). Se frena cuando el transporte se detiene.</td></tr>
<tr><td><b>Brillo y tamaño de las estrellas</b></td><td>Energía de la música.</td></tr>
<tr><td><b>Ancho de la espiral</b></td><td>Ancho estéreo después de OMEGA (se cierra si entra la protección de fase).</td></tr>
<tr><td><b>Tres órbitas</b></td><td>Pegamento en graves (naranja), medios (dorado) y agudos (azul). Más gruesa y brillante = más reducción.</td></tr>
<tr><td><b>Brillo del núcleo</b></td><td>Armónicos (saturación) trabajando.</td></tr>
<tr><td><b>Agujero negro del centro</b></td><td>El limitador: crece con la reducción. Pequeño y tranquilo = master transparente.</td></tr>
<tr><td><b>Anillo dorado</b></td><td>La intensidad de OMEGA (0 – 100 %).</td></tr>
<tr><td><b>Color</b></td><td>El Universo elegido.</td></tr>
<tr><td><b>Horizonte</b></td><td>El espectro de salida; la línea dorada punteada es tu canción de referencia.</td></tr></table>
<div class="box tip">Un master sano: las órbitas respiran suave, el agujero negro se queda pequeño, la espiral mantiene su forma y la brújula queda en verde.</div>
"""},
{"title": "Universos", "html": """
<table><tr><th style="width:22%">Universo</th><th style="width:30%">Carácter</th><th>Úsalo para</th></tr>
<tr><td><b>Nebulosa</b></td><td>Pegamento cálido, agudos suaves</td><td>Funk, soul, R&amp;B, pop con voz, todo lo que deba sentirse caro y relajado.</td></tr>
<tr><td><b>Púlsar</b></td><td>Golpe y groove</td><td>Música con batería al frente: funk, breaks, house, rock. Los transientes salen.</td></tr>
<tr><td><b>Cuásar</b></td><td>Brillante, ancho, moderno</td><td>Pop moderno, hip-hop, trap, EDM: el brillo "listo para la radio".</td></tr>
<tr><td><b>Supernova</b></td><td>Fuerte y denso</td><td>Música de club y todo lo que deba sonar fuerte. Más clip y pegamento.</td></tr>
<tr><td><b>Horizonte</b></td><td>Oscuro, vintage, pesado</td><td>Lo-fi, techno oscuro, IDM, mezclas analógicas: bajan los agudos, sube la saturación. Ideal si siempre bajas agudos.</td></tr>
<tr><td><b>Luz pura</b></td><td>Mastering transparente</td><td>Acústico, jazz, clásico, ambient, o cuando la mezcla ya está terminada y solo necesita volumen.</td></tr></table>
<p>El Universo es un <b>punto de partida</b>: los controles Avanzados se suman a él (0 = el ajuste propio del Universo), así que puedes cambiar de Universo sin perder tus ajustes.</p>
"""},
{"title": "Entrada y volumen", "html": """
<h3>Ganancia de entrada</h3>
<p>Como cualquier cadena de estilo analógico, OMEGA responde a qué tan fuerte le pegas. La <b>zona verde</b> del medidor de entrada (−21 … −13 LUFS) es donde se afinaron los Universos. <b>Entrada auto</b> escucha 6 segundos y deja tu parte más fuerte cerca de −17 LUFS.</p>
<h3>Objetivos</h3>
<table><tr><th style="width:34%">Objetivo</th><th>Úsalo para</th></tr>
<tr><td><b>Streaming −14</b></td><td>Spotify, YouTube, Tidal, Amazon, Deezer… y DistroKid (un archivo para todas las plataformas).</td></tr>
<tr><td><b>Apple Music −16</b></td><td>Cuando quieres conservar la mayor dinámica en Apple Music.</td></tr>
<tr><td><b>Loud −10</b> (fuerte)</td><td>Volumen competitivo para géneros que viven fuertes (en streaming los bajarán).</td></tr>
<tr><td><b>Club −7</b></td><td>Promos para DJ y sistemas de club.</td></tr></table>
<p><b>Ajustar al objetivo</b> sube el empuje del limitador paso a paso mientras suena la parte más fuerte, hasta que el volumen de corto plazo cae en el objetivo (normalmente 6 – 15 s). El <b>Techo</b> (−1 dBTP por defecto, la recomendación de streaming) nunca se rebasa: probado a −14 LUFS con todos los Universos, el pico real quedó por debajo de −1.6 dBTP.</p>
<h3>Los números</h3>
<table><tr><th style="width:30%">Lectura</th><th>Significado</th></tr>
<tr><td>Momentáneo / Corto plazo</td><td>Volumen en 0.4 s / 3 s (LUFS).</td></tr>
<tr><td>Integrado</td><td>Volumen de toda la canción (con compuertas, como miden las plataformas). Dale play desde el inicio para un valor real.</td></tr>
<tr><td>Pico real (máx.)</td><td>El pico entre muestras más alto (sobremuestreo 4×). Verde = bajo el techo.</td></tr>
<tr><td>Rango (LRA)</td><td>Cuánto cambia el volumen entre partes (LU).</td></tr>
<tr><td>Dinámica (PLR)</td><td>Pico real menos integrado: más grande = más golpe y apertura.</td></tr></table>
<p>Bajo los botones de abajo OMEGA te dice cuánto bajará el streaming tu canción, para decidir si vale la pena más volumen.</p>
"""},
{"title": "Comparación honesta: COMPARAR, Δ, A/B", "html": """
<p>La trampa más grande del mastering es que más fuerte siempre suena mejor por unos segundos. OMEGA quita esa trampa.</p>
<ul><li><b>COMPARAR (mismo volumen)</b> reproduce tu mezcla <b>sin</b> OMEGA, subida exactamente al volumen que produce OMEGA y alineada en tiempo. Prende y apaga: solo cambia el sonido, no el volumen. (Probado: la diferencia queda dentro de 1 LU.)</li>
<li><b>Δ SOLO OMEGA</b> reproduce la diferencia: solo lo que OMEGA agrega o quita. Ideal para oír si Domar, Armónicos o el limitador trabajan de más.</li>
<li><b>A · B</b> guardan dos ajustes completos (por ejemplo Nebulosa contra Cuásar, o dos objetivos) para cambiar al instante.</li>
<li><b>Deshacer / Rehacer</b> para cada cambio.</li></ul>
<div class="box warn">Recuerda apagar COMPARAR y Δ antes de exportar.</div>
"""},
{"title": "El recorrido de la señal", "html": """
<p>OMEGA procesa en este orden. La franja del Recorrido muestra cada etapa con su valor en vivo.</p>
<table><tr><th style="width:16%">Etapa</th><th>Qué hace</th></tr>
<tr><td><b>IN</b></td><td>Ganancia de entrada (dB).</td></tr>
<tr><td><b>EQ</b></td><td>Curva del Universo + tus Graves (shelf 90 Hz), Medios (1.1 kHz), Agudos (shelf 5.5 kHz), Aire (14 kHz) y la igualación de referencia.</td></tr>
<tr><td><b>TAME</b></td><td>Domar: quita lo áspero alrededor de 4 kHz, solo cuando esa zona sobresale (hasta 6 dB).</td></tr>
<tr><td><b>PUNCH</b></td><td>Moldeador de transientes: adelanta el ataque de la batería sin subir mucho los picos.</td></tr>
<tr><td><b>GLUE</b></td><td>Compresión en 3 bandas (cortes en 150 Hz y 3 kHz, en fase), enlazada entre canales, liberación según el material. Muestra la reducción de graves / medios / agudos.</td></tr>
<tr><td><b>WIDTH</b></td><td>Abre solo los lados arriba de ~120 Hz; protección de fase; Graves mono opcional.</td></tr>
<tr><td><b>HARM</b></td><td>Armónicos estilo cinta/bulbo, con sobremuestreo (sin aliasing).</td></tr>
<tr><td><b>CLIP</b></td><td>Clipper suave que rasura los picos más rápidos antes del limitador, para que el limitador trabaje menos.</td></tr>
<tr><td><b>LIMIT</b></td><td>Limitador con anticipación a 4× (hasta 8×) la frecuencia de muestreo, liberación adaptativa, sin sobrepasos.</td></tr>
<tr><td><b>OUT</b></td><td>Ajuste de salida y techo final; aquí se muestra el pico real.</td></tr></table>
"""},
{"title": "Controles avanzados", "html": """
<img class="shot" style="width:84%;display:block;margin:0 auto" src="{{omega_adv.png}}"><div class="cap">Vista AVANZADA.</div>
<table><tr><th style="width:20%">Control</th><th style="width:16%">Rango</th><th>Qué hace</th></tr>
<tr><td><b>Graves · Medios · Agudos · Aire</b></td><td>±6 dB</td><td>Se suman a la curva del Universo. Si siempre bajas agudos en tu master, hazlo aquí.</td></tr>
<tr><td><b>Domar</b></td><td>−1 … +1</td><td>Más o menos control de lo áspero que el Universo.</td></tr>
<tr><td><b>Golpe · Pegamento · Armónicos</b></td><td>−1 … +1</td><td>Más o menos de cada etapa respecto al Universo.</td></tr>
<tr><td><b>Ancho</b></td><td>−1 … +1</td><td>Lados más estrechos o más anchos (la protección sigue cuidando el mono).</td></tr>
<tr><td><b>Clip</b></td><td>−1 … +1</td><td>Cuánto trabaja el clipper antes del limitador.</td></tr>
<tr><td><b>Graves mono</b></td><td>apagado · 20 – 300 Hz</td><td>Todo lo que está debajo de esta frecuencia se vuelve mono (seguridad para club y vinilo).</td></tr>
<tr><td><b>Igualar ref.</b></td><td>0 … 1</td><td>Cuánto del tono de la canción de referencia aplicar (página siguiente).</td></tr>
<tr><td><b>Calidad</b></td><td>2× · 4× · 8×</td><td>Sobremuestreo de armónicos, clipper y limitador. 4× por defecto; 8× para la exportación final si quieres.</td></tr></table>
"""},
{"title": "Brújula tonal e igualar referencia", "html": """
<h3>Brújula tonal</h3>
<p>OMEGA mide el balance a largo plazo de tu salida, de graves a agudos (bandas de octava de 63 Hz a 8 kHz). La mayoría de las mezclas comerciales terminadas caen en la zona verde "equilibrado". Si la aguja queda en <b>brillante</b> u <b>oscuro</b>, presiona <b>Equilibrar</b>: OMEGA ajusta Agudos y Graves por ti. Es una guía: confía en tus oídos y en tu género.</p>
<h3>Igualar referencia</h3>
<ol><li>En AVANZADO presiona <b>Cargar referencia…</b> o simplemente <b>suelta una canción</b> (WAV, AIFF, FLAC, MP3) en la ventana.</li>
<li>OMEGA analiza su tono y volumen; su curva aparece como línea dorada punteada en el horizonte.</li>
<li>Reproduce tu canción un rato y sube <b>Igualar ref.</b>: un EQ suave de 10 bandas acerca tu tono al de la referencia (limitado a ±4 dB por banda, así nunca arruina la mezcla).</li>
<li>Usa COMPARAR para confirmar que de verdad lo prefieres.</li></ol>
<div class="box tip">La referencia se guarda con tu proyecto.</div>
"""},
{"title": "Masterizar archivos surround", "html": """
<p>OMEGA trabaja en pistas estéreo, 5.1, 7.1, 5.1.4, 7.1.2 y 7.1.4 (en DAWs con buses surround), con la dinámica enlazada en todos los canales y el LFE sin tocar. También puede masterizar un <b>archivo surround</b>, por ejemplo el WAV 5.1 / 7.1.4 que escribe el grabador surround de CONSTELLATION desde Ableton:</p>
<ol><li>Deja OMEGA como te guste en tu master estéreo.</li>
<li>AVANZADO → <b>Masterizar archivo surround…</b> → elige el WAV.</li>
<li>OMEGA lo procesa en segundo plano con los mismos ajustes y escribe <b>&lt;nombre&gt;_OMEGA.wav</b> junto al original: mismos canales, mismo orden, misma duración (latencia compensada).</li></ol>
<p>Ese archivo queda listo para Final Cut Pro (5.1 para YouTube) o para el Dolby Atmos Renderer (cama 7.1.4). Así se resuelve que la exportación surround de CONSTELLATION se arma antes de tu cadena de master.</p>
<div class="box tip">Para surround usa un objetivo más bajo que en estéreo (alrededor de −16 a −18 LUFS): las entregas surround y Atmos se masterizan con más margen.</div>
"""},
{"title": "Recetas", "html": """
<table><tr><th style="width:26%">Meta</th><th>Ajustes</th></tr>
<tr><td><b>Tu cadena de siempre en un plug-in</b> (graves un poco arriba, medios al gusto, agudos abajo, compresor, limitador)</td><td>Universo <b>Nebulosa</b> 50 – 60 %, AVANZADO Graves +1, Agudos −1 a −2, Pegamento 0, objetivo Streaming −14 → Ajustar al objetivo.</td></tr>
<tr><td><b>Funk / disco</b></td><td><b>Púlsar</b> 60 %, Golpe +0.3, Graves mono 100 Hz, Ancho 0.</td></tr>
<tr><td><b>Hip-hop / trap</b></td><td><b>Cuásar</b> 70 %, Armónicos +0.2, Graves mono 120 Hz, Clip +0.2.</td></tr>
<tr><td><b>Techno / house para club</b></td><td><b>Supernova</b> 70 %, objetivo Club −7 (y una versión −14 para streaming), Graves mono 150 Hz.</td></tr>
<tr><td><b>Oscuro, IDM, lo-fi</b></td><td><b>Horizonte</b> 60 %, Domar +0.3, Aire −1.</td></tr>
<tr><td><b>Acústico, ambient, cine</b></td><td><b>Luz pura</b> 40 %, objetivo Apple −16, Pegamento −0.3.</td></tr>
<tr><td><b>Publicar en DistroKid</b></td><td>Objetivo Streaming −14, techo −1 dBTP, COMPARAR / Δ apagados, exporta un WAV estéreo (24 bits). (Con CONSTELLATION: 3D audífonos apagado.)</td></tr></table>
<div class="box tip">Mezcla hacia OMEGA al 30 – 40 % mientras mezclas, y fija la intensidad y el objetivo finales al terminar.</div>
"""},
{"title": "Prueba y licencia", "html": """
<p>OMEGA viene <b>completo durante 15 días</b>, con todas sus funciones. Después, OMEGA deja pasar tu audio sin procesar hasta que actives una licencia; tus sesiones nunca se dañan.</p>
<img class="shot" style="width:76%;display:block;margin:0 auto" src="{{omega_licence.png}}"><div class="cap">La ventana de licencia (botón dorado de Prueba, arriba).</div>
<table><tr><th style="width:28%">Licencia</th><th></th></tr>
<tr><td><b>Precio</b></td><td><b>US$49</b> un solo pago, o <b>US$69</b> por OMEGA + CONSTELLATION. Sin suscripción.</td></tr>
<tr><td><b>Incluye</b></td><td>Una persona, todas tus computadoras, Mac y Windows, VST3 y AU, todas las actualizaciones 1.x.</td></tr>
<tr><td><b>Comprar</b></td><td>Presiona el botón de compra (PayPal). Tu llave llega por correo a la dirección con la que pagaste.</td></tr>
<tr><td><b>Activar</b></td><td>Pega tu correo y la llave completa → <b>ACTIVAR</b>. Funciona sin internet y sin cuenta.</td></tr></table>
"""},
{"title": "Preguntas y créditos", "html": """
<table><tr><th style="width:38%">Pregunta</th><th>Respuesta</th></tr>
<tr><td>¿Es solo para el master?</td><td>Brilla en el master, pero también funciona en buses de batería, voces o música (prueba Púlsar en batería, Nebulosa en voces).</td></tr>
<tr><td>¿CPU?</td><td>Cerca de 4 % de un núcleo en calidad 4× en nuestras pruebas. 8× duplica la parte sobremuestreada.</td></tr>
<tr><td>El asistente no llega al objetivo.</td><td>Reproduce la parte más fuerte mientras escucha, y revisa que la entrada esté en la zona verde (Entrada auto).</td></tr>
<tr><td>Suena muy brillante / muy oscuro.</td><td>Mira la Brújula tonal y presiona Equilibrar, o cambia de Universo.</td></tr>
<tr><td>¿Puede "bombear"?</td><td>El Pegamento usa liberación según el material; si oyes bombeo baja Pegamento o la intensidad, o usa un objetivo más bajo.</td></tr>
<tr><td>¿Funciona en Final Cut Pro?</td><td>Final Cut usa efectos AU; para audio 5.1 de video lo más simple es "Masterizar archivo surround" e importar el resultado.</td></tr></table>
<h3>Créditos</h3>
<p>Diseño de <b>Dark Funk Durcal</b> — <i>What is Dark Funk Durcal? Search it on Spotify, YouTube and every audio platform.</i><br>
Creado en <b>Polaris Sound Lab</b>, Mérida, Yucatán, México · Instagram @polaris.sound.lab · YouTube @PolarisSoundLab · github.com/drophlex/OMEGA · Hecho con JUCE.</p>
"""},
]}

pathlib.Path(__file__).with_name("manual_text.json").write_text(json.dumps({"en": EN, "es": ES}, ensure_ascii=False), encoding="utf-8")
print("ok")
