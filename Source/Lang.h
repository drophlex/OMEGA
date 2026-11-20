// OMEGA — UI strings. English by default, Spanish optional. (Source is UTF-8; MSVC builds with /utf-8.)
#pragma once
#include <juce_core/juce_core.h>
#include <atomic>

namespace om
{
inline std::atomic<int>& langFlag() { static std::atomic<int> l { 0 }; return l; }

inline juce::String tr (const char* key)
{
    struct E { const char* k; const char* en; const char* es; };
    static const E table[] = {
        { "amount", "OMEGA", "OMEGA" },
        { "dragHint", "Drag the galaxy up/down to set the amount", "Arrastra la galaxia arriba/abajo para la intensidad" },
        { "input", "INPUT", "ENTRADA" }, { "output", "OUTPUT", "SALIDA" },
        { "inGain", "Input", "Entrada" }, { "outGain", "Output", "Salida" },
        { "autoInput", "Auto input", "Entrada auto" }, { "learning", "Listening…", "Escuchando…" },
        { "sweet", "Sweet spot", "Zona ideal" }, { "low", "too low", "muy bajo" }, { "hot", "too hot", "muy alto" },
        { "target", "Target", "Objetivo" }, { "ceiling", "Ceiling", "Techo" }, { "drive", "Loudness", "Volumen" },
        { "learnLoud", "Match target", "Ajustar al objetivo" },
        { "momentary", "Momentary", "Momentáneo" }, { "short", "Short-term", "Corto plazo" }, { "integrated", "Integrated", "Integrado" },
        { "truepeak", "True peak", "Pico real" }, { "range", "Range", "Rango" }, { "dynamics", "Dynamics (PLR)", "Dinámica (PLR)" },
        { "resetMeters", "Reset", "Reiniciar" },
        { "compare", "COMPARE  (level-matched)", "COMPARAR  (mismo volumen)" }, { "delta", "Δ  ONLY OMEGA", "Δ  SOLO OMEGA" },
        { "advanced", "ADVANCED", "AVANZADO" }, { "simple", "SIMPLE", "SIMPLE" },
        { "eqLow", "Low", "Graves" }, { "eqMid", "Mid", "Medios" }, { "eqHigh", "High", "Agudos" }, { "eqAir", "Air", "Aire" },
        { "tame", "Tame", "Domar" }, { "punch", "Punch", "Golpe" }, { "glue", "Glue", "Pegamento" }, { "harmonics", "Harmonics", "Armónicos" },
        { "width", "Width", "Ancho" }, { "clip", "Clip", "Clip" }, { "bassMono", "Bass mono", "Graves mono" }, { "match", "Ref. match", "Igualar ref." },
        { "loadRef", "Load reference…", "Cargar referencia…" }, { "noRef", "No reference (drop a song here)", "Sin referencia (suelta una canción aquí)" },
        { "surround", "Master a surround file…", "Masterizar archivo surround…" }, { "rendering", "Rendering", "Procesando" },
        { "rendered", "Done: ", "Listo: " },
        { "quality", "Quality", "Calidad" },
        { "tone", "TONE COMPASS", "BRÚJULA TONAL" }, { "dark", "dark", "oscuro" }, { "bright", "bright", "brillante" }, { "balanced", "balanced", "equilibrado" },
        { "balance", "Balance it", "Equilibrar" },
        { "stream", "Streaming will turn this down by %s dB", "El streaming bajará esto %s dB" },
        { "streamOk", "Fits streaming loudness", "Volumen correcto para streaming" },
        { "chain", "SIGNAL PATH", "RECORRIDO DE LA SEÑAL" },
        { "trial", "Trial: %d days left", "Prueba: quedan %d días" }, { "buy", "Buy licence", "Comprar licencia" }, { "licensed", "Licensed", "Con licencia" },
        { "licTitle", "OMEGA licence", "Licencia de OMEGA" },
        { "licText", "Everything is unlocked for 15 days. After that OMEGA passes the audio through untouched until you activate a licence.\n\n"
                     "One licence = one person, all your computers, Mac and Windows, VST3 and AU, every 1.x update.\n\n"
                     "1. Press BUY (PayPal).  2. Your key arrives by e-mail at the address you paid with.  3. Paste e-mail and key here and press ACTIVATE.",
                     "Todo está desbloqueado durante 15 días. Después OMEGA deja pasar el audio sin procesar hasta que actives una licencia.\n\n"
                     "Una licencia = una persona, todas tus computadoras, Mac y Windows, VST3 y AU, todas las actualizaciones 1.x.\n\n"
                     "1. Presiona COMPRAR (PayPal).  2. Tu llave llega por correo a la dirección con la que pagaste.  3. Pega correo y llave aquí y presiona ACTIVAR." },
        { "buyOmega", "OMEGA  US$49", "OMEGA  US$49" }, { "buyBundle", "OMEGA + CONSTELLATION  US$69", "OMEGA + CONSTELLATION  US$69" },
        { "email", "E-mail", "Correo" }, { "key", "Licence key", "Llave de licencia" }, { "activate", "ACTIVATE", "ACTIVAR" },
        { "actOk", "Activated — thank you for supporting Polaris Sound Lab!", "Activada. ¡Gracias por apoyar a Polaris Sound Lab!" },
        { "actBad", "That key does not match this e-mail. Copy it again, complete.", "Esa llave no corresponde a este correo. Cópiala otra vez completa." },
        { "expired", "Trial ended — audio passes through untouched. Get a licence to keep using OMEGA.", "Terminó la prueba: el audio pasa sin procesar. Consigue una licencia para seguir usando OMEGA." },
        { "close", "Close", "Cerrar" },
        { "footer", "Design by Dark Funk Durcal · Created at Polaris Sound Lab · Mérida, Yucatán", "Diseño de Dark Funk Durcal · Creado en Polaris Sound Lab · Mérida, Yucatán" },
        { "lang", "ES", "EN" }, { "undo", "Undo", "Deshacer" }, { "redo", "Redo", "Rehacer" },
        { "help",
          "OMEGA — the last plug-in on your master\n\n"
          "1. Put OMEGA at the end of the Master chain (with CONSTELLATION, keep its SKY right after OMEGA so its meters see the final master).\n"
          "2. Press AUTO INPUT and play the loudest part for a few seconds: OMEGA sets the input in the sweet spot.\n"
          "3. Choose a UNIVERSE (character) and drag the galaxy up/down to set the amount.\n"
          "4. Choose a TARGET (Streaming −14 is right for Spotify/YouTube/DistroKid) and press MATCH TARGET while the loudest part plays.\n"
          "5. Press COMPARE: you hear your mix without OMEGA at the same loudness — honest A/B, no 'louder = better' trick.\n"
          "   Δ ONLY OMEGA lets you hear exactly what OMEGA adds.\n"
          "6. The SIGNAL PATH shows every stage working in real time: nothing is hidden.\n"
          "7. ADVANCED: EQ, Tame (removes harshness only when it appears), Punch, Glue, Harmonics, Width (with a correlation guard),\n"
          "   Bass mono, Clip, reference match, quality (oversampling) and surround-file mastering.\n\n"
          "Click anywhere to close.",
          "OMEGA — el último plug-in de tu master\n\n"
          "1. Pon OMEGA al final de la cadena del Master (con CONSTELLATION, deja su CIELO justo después de OMEGA para que sus medidores vean el master final).\n"
          "2. Presiona ENTRADA AUTO y reproduce la parte más fuerte unos segundos: OMEGA deja la entrada en la zona ideal.\n"
          "3. Elige un UNIVERSO (carácter) y arrastra la galaxia arriba/abajo para la intensidad.\n"
          "4. Elige un OBJETIVO (Streaming −14 es el correcto para Spotify/YouTube/DistroKid) y presiona AJUSTAR AL OBJETIVO en la parte más fuerte.\n"
          "5. Presiona COMPARAR: escuchas tu mezcla sin OMEGA al mismo volumen; un A/B honesto, sin el truco de 'más fuerte = mejor'.\n"
          "   Δ SOLO OMEGA te deja escuchar exactamente lo que agrega OMEGA.\n"
          "6. El RECORRIDO DE LA SEÑAL muestra cada etapa trabajando en tiempo real: nada está escondido.\n"
          "7. AVANZADO: EQ, Domar (quita lo áspero solo cuando aparece), Golpe, Pegamento, Armónicos, Ancho (con protección de fase),\n"
          "   Graves mono, Clip, igualar referencia, calidad (sobremuestreo) y master de archivos surround.\n\n"
          "Haz clic en cualquier parte para cerrar." },
    };
    const bool es = langFlag().load() == 1;
    for (auto& e : table)
        if (std::strcmp (e.k, key) == 0) return juce::String::fromUTF8 (es ? e.es : e.en);
    return key;
}
} // namespace om
