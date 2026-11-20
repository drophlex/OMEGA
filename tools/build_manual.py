#!/usr/bin/env python3
"""Builds the illustrated manuals (EN, ES, EN+ES) as PDF with headless Chrome.
Print design "signal print": flat colour, Swiss grid, condensed grotesk, technical codes,
katakana, registration marks and barcodes. Fonts are embedded from Resources/fonts (OFL)."""
import base64, hashlib, json, pathlib, re, subprocess

ROOT = pathlib.Path(__file__).resolve().parents[1]
IMG, FONTS, OUT = ROOT / "docs" / "images", ROOT / "Resources" / "fonts", ROOT / "manual"
OUT.mkdir(exist_ok=True)

# ── product configuration ──────────────────────────────────────────────
CFG = json.loads((ROOT / "tools" / "manual_cfg.json").read_text(encoding="utf-8"))
P, KEY, KEY2, KANA, CODE, VER = CFG["product"], CFG["key"], CFG["key2"], CFG["kana"], CFG["code"], CFG["version"]

INK, PAPER, PAPER2, GREY = "#0b0b0c", "#ede9df", "#e0dbcf", "#8f8b82"

def b64(path): return base64.b64encode(path.read_bytes()).decode()
def img(name): return "data:image/png;base64," + b64(IMG / name)
def face(fam, file, weight=400): return f"@font-face{{font-family:'{fam}';src:url(data:font/ttf;base64,{b64(FONTS / file)}) format('truetype');font-weight:{weight};}}"

def kanji(n):
    d = "〇一二三四五六七八九"
    if n < 10: return d[n]
    return ("" if n < 20 else d[n // 10]) + "十" + ("" if n % 10 == 0 else d[n % 10])

def barcode(seed, w=150, h=26, col=INK):
    hsh = hashlib.sha256(seed.encode()).digest()
    x, bars = 0.0, []
    for i, byte in enumerate(hsh * 2):
        bw = 0.8 + (byte % 4) * 0.9
        if i % 2 == 0: bars.append(f'<rect x="{x:.1f}" y="0" width="{bw:.1f}" height="{h}" fill="{col}"/>')
        x += bw + 0.7
        if x > w - 4: break
    return f'<svg class="bc" width="{w}" height="{h}" viewBox="0 0 {w} {h}">{"".join(bars)}</svg>'

def reg(col=INK, s=14):
    return (f'<svg width="{s}" height="{s}" viewBox="-7 -7 14 14"><circle r="4" fill="none" stroke="{col}" stroke-width="0.8"/>'
            f'<path d="M-7 0H7M0-7V7" stroke="{col}" stroke-width="0.8"/></svg>')

CSS = "".join([face("Display", "Archivo-CondensedBlack.ttf", 900), face("UI", "Archivo-SemiCondBold.ttf", 700),
               face("Body", "Archivo-Regular.ttf", 400), face("Mono", "SpaceMono-Regular.ttf", 400), face("Mono", "SpaceMono-Bold.ttf", 700)]) + f"""
@page {{ size: A4; margin: 0; }}
* {{ box-sizing: border-box; }}
:root {{ --ink:{INK}; --paper:{PAPER}; --paper2:{PAPER2}; --key:{KEY}; --key2:{KEY2}; --grey:{GREY}; }}
body {{ margin:0; font:9.6pt/1.48 'Body', sans-serif; color:var(--ink); counter-reset: fig; }}
.jp {{ font-family:'Noto Sans CJK JP','Noto Sans JP',sans-serif; font-weight:900; }}
.page {{ width:210mm; height:297mm; padding:30mm 16mm 22mm 16mm; position:relative; page-break-after:always; background:var(--paper); overflow:hidden; }}
.page:last-child {{ page-break-after:auto; }}
b, strong {{ font-family:'UI'; font-weight:700; }}
i {{ font-style:italic; }}

/* running head */
.rh {{ position:absolute; top:9mm; left:16mm; right:16mm; height:12mm; display:flex; align-items:stretch; gap:0; font:7pt/1 'Mono'; letter-spacing:.06em; text-transform:uppercase; }}
.rh .tab {{ background:var(--key); color:var(--ink); padding:0 3mm; display:flex; align-items:center; font-weight:700; }}
.rh .ink {{ background:var(--ink); color:var(--paper); padding:0 3mm; display:flex; align-items:center; }}
.rh .fill {{ flex:1; border-top:1.2pt solid var(--ink); border-bottom:1.2pt solid var(--ink); display:flex; align-items:center; justify-content:space-between; padding:0 3mm; }}
.rh .jp {{ font-size:9pt; }}
.rm {{ position:absolute; }} .rm.tl {{ top:3.5mm; left:3.5mm; }} .rm.tr {{ top:3.5mm; right:3.5mm; }} .rm.bl {{ bottom:3.5mm; left:3.5mm; }} .rm.br {{ bottom:3.5mm; right:3.5mm; }}

/* running foot */
.rf {{ position:absolute; bottom:8mm; left:16mm; right:16mm; display:flex; align-items:flex-end; justify-content:space-between; font:6.8pt/1.2 'Mono'; text-transform:uppercase; letter-spacing:.05em; border-top:1.2pt solid var(--ink); padding-top:2mm; }}
.rf .pn {{ font:900 26pt/0.8 'Display'; }}
.rf .mid {{ display:flex; gap:4mm; align-items:flex-end; }}

/* section head */
.sh {{ display:grid; grid-template-columns:auto 1fr; gap:0 5mm; align-items:end; margin:0 0 5mm; }}
.sh .num {{ font:900 78pt/0.74 'Display'; color:var(--key); letter-spacing:-.02em; -webkit-text-stroke: 0; }}
.sh .tt {{ border-bottom:3pt solid var(--ink); padding-bottom:2mm; }}
.sh .tt .meta {{ font:7pt/1 'Mono'; text-transform:uppercase; letter-spacing:.08em; display:flex; justify-content:space-between; margin-bottom:2mm; }}
.sh .tt .meta .jp {{ font-size:10pt; letter-spacing:0; }}
.sh h2 {{ font:900 25pt/0.95 'Display'; text-transform:uppercase; margin:0; letter-spacing:.005em; }}

h3 {{ font:700 8.2pt/1.2 'Mono'; text-transform:uppercase; letter-spacing:.06em; margin:4.5mm 0 1.8mm; padding-top:1.6mm; border-top:0.8pt solid var(--ink); display:flex; align-items:center; gap:2mm; }}
h3::before {{ content:''; width:2.6mm; height:2.6mm; background:var(--key); flex:none; }}
p {{ margin:0 0 2.3mm; }}
ol, ul {{ margin:0 0 2.6mm; padding-left:5.5mm; }} li {{ margin-bottom:1mm; }}
ul {{ list-style:none; padding-left:4.5mm; }} ul li::before {{ content:'■'; color:var(--key); font-size:6pt; margin-left:-4.5mm; width:4.5mm; display:inline-block; vertical-align:1.5pt; }}
ol li::marker {{ font:700 8pt 'Mono'; }}

.shot {{ width:100%; border:2pt solid var(--ink); background:var(--ink); margin:2mm 0 0; display:block; }}
.cap {{ font:6.9pt/1.35 'Mono'; text-transform:uppercase; letter-spacing:.04em; margin:1.2mm 0 3.5mm; display:flex; gap:2mm; }}
.cap::before {{ counter-increment: fig; content:'FIG.' counter(fig, decimal-leading-zero); background:var(--ink); color:var(--paper); padding:0.3mm 1.4mm; flex:none; height:fit-content; }}
.cap b {{ font-family:'Mono'; }}

table {{ width:100%; border-collapse:collapse; margin:1.5mm 0 3.5mm; font-size:8.7pt; line-height:1.38; border-bottom:1.6pt solid var(--ink); }}
th {{ text-align:left; background:var(--ink); color:var(--paper); padding:1.6mm 2.2mm; font:700 7pt/1.2 'Mono'; text-transform:uppercase; letter-spacing:.06em; }}
th:first-child {{ border-left:3mm solid var(--key); }}
td {{ padding:1.5mm 2.2mm; border-bottom:0.5pt solid var(--ink); vertical-align:top; }}
tr:nth-child(even) td {{ background:var(--paper2); }}
td b {{ font-family:'UI'; }}

.box {{ border:1.4pt solid var(--ink); padding:3.2mm 3.6mm 2.6mm; margin:4mm 0 3mm; position:relative; background:var(--paper); }}
.box::before {{ content:'// NOTE'; position:absolute; top:-2.2mm; left:3mm; background:var(--ink); color:var(--paper); font:700 6.4pt/1 'Mono'; letter-spacing:.08em; padding:0.8mm 1.6mm; }}
.box.tip {{ border-left:4mm solid var(--key); }}
.box.tip::before {{ content:'// INFO'; background:var(--key); color:var(--ink); }}
.box.warn {{ background:#f2e205; }}
.box.warn::before {{ content:'!! ALERT'; }}
.box p:last-child {{ margin-bottom:0; }}
.key {{ display:inline-block; background:var(--ink); color:var(--key); padding:0 1.4mm; font:700 7.6pt/1.5 'Mono'; }}
.two {{ display:grid; grid-template-columns:1fr 1fr; gap:5mm; }}
.donate {{ font-family:'UI'; color:var(--ink); background:var(--key2); padding:0 1mm; }}
a {{ color:var(--ink); }}

/* cover */
.cover {{ background:var(--key); padding:0; }}
.cover .in {{ position:absolute; inset:9mm 12mm 10mm 12mm; display:flex; flex-direction:column; }}
.cover .top {{ display:flex; justify-content:space-between; font:700 7pt/1.3 'Mono'; text-transform:uppercase; letter-spacing:.08em; border-bottom:1.5pt solid var(--ink); padding-bottom:2mm; }}
.cover .name {{ font:900 {CFG['cover_size']}/0.78 'Display'; letter-spacing:-.01em; margin:7mm 0 0 -1mm; white-space:nowrap; }}
.cover .row {{ display:grid; grid-template-columns:1fr auto; align-items:end; gap:6mm; margin-top:3mm; border-bottom:1.5pt solid var(--ink); padding-bottom:3mm; }}
.cover .kana {{ font-size:25pt; line-height:1; }}
.cover .tag {{ font:900 18pt/1 'Display'; text-transform:uppercase; text-align:right; }}
.cover .sub {{ font:7.4pt/1.5 'Mono'; text-transform:uppercase; letter-spacing:.04em; margin:3mm 0 5mm; max-width:150mm; }}
.cover .shotwrap {{ position:relative; background:var(--ink); padding:3mm; }}
.cover .shotwrap img {{ width:100%; display:block; }}
.cover .shotwrap .lbl {{ position:absolute; top:-4.2mm; right:0; background:var(--ink); color:var(--key); font:700 7pt/1 'Mono'; padding:1.2mm 2mm; letter-spacing:.08em; }}
.cover .grid {{ flex:1; display:grid; grid-template-columns:1.3fr 1fr 1fr; gap:0; margin-top:6mm; border-top:1.5pt solid var(--ink); }}
.cover .cell {{ border-right:1.5pt solid var(--ink); padding:3mm 3mm 0 0; font:7pt/1.4 'Mono'; text-transform:uppercase; }}
.cover .cell + .cell {{ padding-left:3mm; }} .cover .cell:last-child {{ border-right:0; }}
.cover .big {{ font:900 46pt/0.85 'Display'; }}
.cover .hatch {{ height:14mm; margin-top:3mm; background:repeating-linear-gradient(-45deg, var(--ink) 0 1.2pt, transparent 1.2pt 4pt); }}
.cover .dots {{ height:14mm; margin-top:3mm; background-image:radial-gradient(var(--ink) 0.9pt, transparent 1pt); background-size:3mm 3mm; }}
.cover .bottom {{ background:var(--ink); color:var(--paper); display:flex; justify-content:space-between; align-items:center; padding:3mm 4mm; margin-top:4mm; }}
.cover .bottom .lang {{ font:900 34pt/0.8 'Display'; color:var(--key); }}
.cover .bottom .m {{ font:700 8pt/1.3 'Mono'; text-transform:uppercase; letter-spacing:.1em; }}
.cover .sq {{ display:inline-block; width:4mm; height:4mm; background:var(--ink); margin-right:1.2mm; }}
.cover .sq.k2 {{ background:var(--key2); }} .cover .sq.p {{ background:var(--paper); }}
"""

LBL = {"en": {"sec": "SECTION", "of": "OF", "manual": "USER MANUAL", "lang": "EN", "fmt": "VST3 / AU / MAC / WIN"},
       "es": {"sec": "SECCIÓN", "of": "DE", "manual": "MANUAL DE USUARIO", "lang": "ES", "fmt": "VST3 / AU / MAC / WIN"}}

T = json.loads((ROOT / "tools" / "manual_text.json").read_text(encoding="utf-8"))

def restyle_inline(html):
    """Old inline colours/rounded boxes (chain diagrams) → flat print palette."""
    rep = {"border-radius:8px;": "border:1.2pt solid #0b0b0c;", "#e1e3f4": PAPER2, "#9aa3d6": INK, "#0b0f24": INK, "#e8ecff": PAPER,
           "#58e1ff": "#00a9e0", "#ffd166": KEY if P == "OMEGA" else "#f2e205", "#ff4fd8": "#ff2e88", "font-weight:700": "font-family:UI;font-weight:700", "font-weight:900": "font-family:Display;font-weight:900"}
    for a, b in rep.items(): html = html.replace(a, b)
    return html

def chrome(lang, i, n, title):
    L = LBL[lang]
    return (f'<span class="rm tl">{reg()}</span><span class="rm tr">{reg()}</span><span class="rm bl">{reg()}</span><span class="rm br">{reg()}</span>'
            f'<div class="rh"><div class="tab">{CODE}-M/{L["lang"]}</div><div class="ink">{P}</div>'
            f'<div class="fill"><span>{L["sec"]} {i:02d} {L["of"]} {n:02d}</span><span class="jp">{KANA}</span><span>POLARIS SOUND LAB // MID.YUC.MX</span></div></div>'
            f'<div class="rf"><div>{P} — {L["manual"]}<br>V{VER} // {L["fmt"]}</div>'
            f'<div class="mid">{barcode(f"{P}{lang}{i}", 120, 18)}<span>{CODE}.{lang.upper()}.{i:03d}</span></div><div class="pn">{i + 1:02d}</div></div>')

def cover(lang, t):
    L = LBL[lang]
    sub = re.sub(r"<br\s*/?>", " // ", t["sub"])
    return f"""<div class="page cover"><span class="rm tl">{reg()}</span><span class="rm tr">{reg()}</span><span class="rm bl">{reg()}</span><span class="rm br">{reg()}</span>
<div class="in">
 <div class="top"><span>POLARIS SOUND LAB<br>MÉRIDA / YUCATÁN / MX</span><span>{CODE}-01 // {L['fmt']}<br>V{VER} // 2026</span><span style="text-align:right">{L['manual']}<br><span class="sq"></span><span class="sq k2"></span><span class="sq p"></span></span></div>
 <div class="name">{P}</div>
 <div class="row"><div class="kana jp">{KANA}</div><div class="tag">{t['tag']}</div></div>
 <div class="sub">{sub}</div>
 <div class="shotwrap"><span class="lbl">FIG.00 // {P} {CFG['fig0']}</span><img src="{img(CFG['cover_img'])}"></div>
 <div class="grid">
  <div class="cell"><div class="big">{L['lang']}</div><div style="margin-top:2mm">{L['manual']}<br>{CFG['tagline_'+lang]}</div><div class="hatch"></div></div>
  <div class="cell">{CFG['spec1']}<div class="dots"></div></div>
  <div class="cell">{CFG['spec2_'+lang]}<br><br>{barcode(P + lang, 130, 30)}</div>
 </div>
 <div class="bottom"><span class="m">{P} // {CFG['claim_'+lang]}</span><span class="lang jp" style="font-size:18pt">{KANA}</span><span class="lang">V{VER}</span></div>
</div></div>"""

def build(lang):
    t = T[lang]; secs = t["sections"]; n = len(secs)
    pages = [cover(lang, t)]
    for i, sec in enumerate(secs, 1):
        body = re.sub(r"\{\{([\w.-]+\.png)\}\}", lambda m: img(m.group(1)), restyle_inline(sec["html"]))
        head = (f'<div class="sh"><div class="num">{i:02d}</div><div class="tt"><div class="meta"><span>{LBL[lang]["sec"]} {i:02d} // {P}</span>'
                f'<span class="jp">第{kanji(i)}章</span></div><h2>{sec["title"]}</h2></div></div>')
        pages.append(f'<div class="page">{chrome(lang, i, n, sec["title"])}{head}{body}</div>')
    return "\n".join(pages)

def render(langs, name):
    html = f'<!DOCTYPE html><html><head><meta charset="utf-8"><style>{CSS}</style></head><body>' + "".join(build(l) for l in langs) + "</body></html>"
    src = OUT / f".{name}.html"
    src.write_text(html, encoding="utf-8")
    js = f"""const {{ chromium }} = require('/opt/npm-tools/node_modules/playwright');
(async () => {{ const b = await chromium.launch({{ executablePath: '/usr/bin/google-chrome' }}); const p = await b.newPage();
await p.goto('file://{src}'); await p.evaluate(() => document.fonts.ready);
const over = await p.evaluate(() => [...document.querySelectorAll('.page')].map((e, i) => {{
  const lim = e.getBoundingClientRect().bottom - 22 * 3.7795; let m = 0;
  for (const c of e.children) {{ if (c.classList.contains('rf') || c.classList.contains('rm')) continue; m = Math.max(m, c.getBoundingClientRect().bottom); }}
  return (m > lim && !e.classList.contains('cover')) ? i + 1 : 0; }}).filter(x => x));
if (over.length) console.log('OVERFLOW pages', over.join(','));
await p.pdf({{ path: '{OUT / (name + ".pdf")}', format: 'A4', printBackground: true, preferCSSPageSize: true }}); await b.close(); }})();"""
    (OUT / ".render.js").write_text(js)
    subprocess.run(["node", str(OUT / ".render.js")], check=True)
    src.unlink(); (OUT / ".render.js").unlink()
    print("built", name)

if __name__ == "__main__":
    import sys
    if "--html" in sys.argv:   # debug: keep html for one language
        (OUT / "preview.html").write_text(f'<!DOCTYPE html><html><head><meta charset="utf-8"><style>{CSS}</style></head><body>{build("en")}</body></html>', encoding="utf-8")
    else:
        for langs, suffix in ((["en"], "EN"), (["es"], "ES"), (["en", "es"], "EN-ES")):
            render(langs, f"{CFG['file']}-Manual-{suffix}")
