#!/usr/bin/env python3
"""Builds the illustrated OMEGA manuals (EN, ES, EN+ES) as PDF with headless Chrome."""
import base64, pathlib, subprocess, sys, json

ROOT = pathlib.Path(__file__).resolve().parents[1]
IMG = ROOT / "docs" / "images"
OUT = ROOT / "manual"
OUT.mkdir(exist_ok=True)

def img(name):
    return "data:image/png;base64," + base64.b64encode((IMG / name).read_bytes()).decode()

CSS = """
@page { size: A4; margin: 0; }
* { box-sizing: border-box; }
body { margin: 0; font: 10.5pt/1.5 "Inter", "Helvetica Neue", Arial, sans-serif; color: #1a1f3a; }
.page { width: 210mm; min-height: 297mm; padding: 18mm 17mm 16mm; position: relative; page-break-after: always; background: #fbfbff; }
.page:last-child { page-break-after: auto; }
.cover { background: radial-gradient(circle at 30% 25%, #3a2470 0%, transparent 45%), radial-gradient(circle at 80% 75%, #5a3a10 0%, transparent 45%), #04030c; color: #e8ecff; padding: 26mm 20mm; }
.cover h1 { font-size: 40pt; letter-spacing: 2px; margin: 10mm 0 2mm; }
.cover .tag { color: #ffd166; font-size: 14pt; font-weight: 700; }
.cover .sub { color: #9aa3d6; font-size: 11pt; margin-top: 3mm; }
.cover img { width: 100%; border-radius: 10px; margin-top: 12mm; box-shadow: 0 0 40px #58e1ff44; }
.cover .foot { position: absolute; bottom: 16mm; left: 20mm; right: 20mm; color: #9aa3d6; font-size: 9.5pt; display: flex; justify-content: space-between; }
.star { width: 46px; height: 46px; }
h2 { font-size: 19pt; margin: 0 0 4mm; color: #0b0f24; border-bottom: 3px solid #ffd166; padding-bottom: 2mm; }
h2 .n { color: #c9932a; margin-right: 6px; }
h3 { font-size: 12.5pt; margin: 5mm 0 1.5mm; color: #2c2a7a; }
p { margin: 0 0 2.6mm; }
.shot { width: 100%; border-radius: 8px; border: 1px solid #c9cdea; margin: 2mm 0 1mm; }
.cap { font-size: 8.8pt; color: #6a6f99; margin-bottom: 4mm; }
table { width: 100%; border-collapse: collapse; margin: 2mm 0 4mm; font-size: 9.6pt; }
th { text-align: left; background: #0b0f24; color: #e8ecff; padding: 2mm 2.5mm; font-weight: 700; }
td { padding: 1.8mm 2.5mm; border-bottom: 1px solid #e1e3f4; vertical-align: top; }
tr:nth-child(even) td { background: #f2f3fc; }
td b { color: #2c2a7a; }
.box { border-radius: 8px; padding: 3mm 4mm; margin: 3mm 0; }
.tip { background: #e9fbff; border-left: 4px solid #2fb9df; }
.warn { background: #fff4ec; border-left: 4px solid #ff8a3d; }
.key { display: inline-block; background: #0b0f24; color: #ffd166; border-radius: 4px; padding: 0 5px; font-size: 9pt; font-weight: 700; }
ol, ul { margin: 0 0 3mm; padding-left: 6mm; } li { margin-bottom: 1.2mm; }
.two { display: grid; grid-template-columns: 1fr 1fr; gap: 6mm; }
.pnum { position: absolute; bottom: 8mm; right: 17mm; font-size: 8.5pt; color: #9aa3d6; }
.brand { position: absolute; bottom: 8mm; left: 17mm; font-size: 8.5pt; color: #9aa3d6; }
.donate { color: #d936b5; font-weight: 700; }
"""

STAR = '<svg class="star" viewBox="0 0 40 40"><defs><linearGradient id="g" x1="0" y1="0" x2="0" y2="1"><stop offset="0" stop-color="#fff"/><stop offset="1" stop-color="#ffd166"/></linearGradient></defs><path fill="none" stroke="url(#g)" stroke-width="4.5" stroke-linecap="round" d="M13 33 A14 14 0 1 1 27 33"/><rect x="5" y="32" width="11" height="4" rx="1.5" fill="#ffd166"/><rect x="24" y="32" width="11" height="4" rx="1.5" fill="#ffd166"/></svg>'

T = json.loads((ROOT / "tools" / "manual_text.json").read_text(encoding="utf-8"))

def build(lang):
    t = T[lang]
    pages = []
    pages.append(f"""<div class="page cover">{STAR}<h1>OMEGA</h1><div class="tag">{t['tag']}</div><div class="sub">{t['sub']}</div>
<img src="{img('omega_main.png')}"><div class="foot"><span>{t['manual']} · v1.0</span><span>Polaris Sound Lab · Mérida, Yucatán, México</span></div></div>""")
    for i, sec in enumerate(t["sections"], 1):
        body = sec["html"]
        import re
        body = re.sub(r"\{\{([\w.-]+\.png)\}\}", lambda m: img(m.group(1)), body)
        pages.append(f'<div class="page"><h2><span class="n">{i:02d}</span>{sec["title"]}</h2>{body}'
                     f'<div class="brand">OMEGA · {t["footer"]}</div><div class="pnum">{i + 1}</div></div>')
    return "\n".join(pages)

def render(langs, name):
    html = f'<!DOCTYPE html><html><head><meta charset="utf-8"><style>{CSS}</style></head><body>' + "".join(build(l) for l in langs) + "</body></html>"
    src = OUT / f".{name}.html"
    src.write_text(html, encoding="utf-8")
    js = f"""const {{ chromium }} = require('/opt/npm-tools/node_modules/playwright');
(async () => {{ const b = await chromium.launch({{ executablePath: '/usr/bin/google-chrome' }}); const p = await b.newPage();
await p.goto('file://{src}'); await p.pdf({{ path: '{OUT / (name + ".pdf")}', format: 'A4', printBackground: true, preferCSSPageSize: true }}); await b.close(); }})();"""
    (OUT / ".render.js").write_text(js)
    subprocess.run(["node", str(OUT / ".render.js")], check=True)
    src.unlink(); (OUT / ".render.js").unlink()
    print("built", name)

render(["en"], "Omega-Manual-EN")
render(["es"], "Omega-Manual-ES")
render(["en", "es"], "Omega-Manual-EN-ES")
