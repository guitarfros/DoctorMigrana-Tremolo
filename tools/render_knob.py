# SPDX-License-Identifier: AGPL-3.0-only
# Copyright (C) 2026 GuitarFros
# Xera Assets/knob_strip.png (48 frames, 8 columnas):  python3 tools/render_knob.py strip
import numpy as np, cv2, math, sys
rng = np.random.default_rng(11)
S = 2; W, H = 300, 340; Wm, Hm = W*S, H*S
Y, X = np.mgrid[0:Hm, 0:Wm].astype(np.float32) / S
unit = lambda v: np.array(v, np.float32) / np.linalg.norm(v)
L  = unit([-0.45, -0.55, 0.70]); Hv = unit(L + np.array([0, 0, 1], np.float32))

base = np.array([98, 52, 32], np.float32)           # baquelita (mostrada da foto)
CAP = (150.0, 135.0); RX, RY, WALL = 98.0, 90.0, 52.0
SK  = (150.0, 205.0); SRX, SRY = 124.0, 98.0
LOBES, LOBE_AMP = 7, 0.045
DIRT = np.array([140, 106, 84], np.float32)         # po / desgaste claro
LIGHT = np.array([212, 184, 148], np.float32)       # rabuñaduras

def smooth(a, b, x): t = np.clip((x-a)/(b-a), 0, 1); return t*t*(3-2*t)
def blur(a, s): return cv2.GaussianBlur(a, (0, 0), s*S)
def norm01(a): return (a - a.mean()) / (a.std() + 1e-6)
def shade(n, col, amb, dif, spec, shin):
    d = np.clip((n*L).sum(-1), 0, 1)[..., None]
    sp = (np.clip((n*Hv).sum(-1), 0, 1)**shin)[..., None]*spec
    return col*(amb+dif*d) + 255*sp
def mix(c, tint, m): return c*(1-m[..., None]) + tint*m[..., None]

def make_tex():
    T = {}
    T['grain'] = blur(rng.normal(0, 1, (Hm, Wm)).astype(np.float32), 0.45)
    sc = np.zeros((Hm, Wm), np.uint8)
    for _ in range(14):
        a = rng.uniform(0, 2*math.pi); r0 = rng.uniform(0, 95); l = rng.uniform(10, 60)
        x0 = CAP[0]+math.cos(a)*r0; y0 = CAP[1]+math.sin(a)*r0*0.9; b = rng.uniform(0, math.pi)
        cv2.line(sc, (int(x0*S), int(y0*S)), (int((x0+math.cos(b)*l)*S), int((y0+math.sin(b)*l)*S)),
                 int(rng.uniform(60, 200)), 1, cv2.LINE_AA)
    T['scr'] = sc.astype(np.float32)/255
    T['scuff'] = smooth(1.0, 2.2, norm01(blur(rng.normal(0, 1, (Hm, Wm)).astype(np.float32), 3.5)))
    T['grime'] = smooth(-0.6, 1.8, norm01(blur(rng.normal(0, 1, (Hm, Wm)).astype(np.float32), 12)))
    pits = np.zeros((Hm, Wm), np.uint8); chips = np.zeros((Hm, Wm), np.uint8)
    for m, n_, rmax in [(pits, 22, 1.6), (chips, 10, 1.3)]:
        for _ in range(n_):
            a = rng.uniform(0, 2*math.pi); r = 100*math.sqrt(rng.uniform(0, 1))
            cv2.circle(m, (int((CAP[0]+math.cos(a)*r)*S), int((CAP[1]+math.sin(a)*r*0.9)*S)),
                       int(rng.uniform(0.7, rmax)*S), 255, -1, cv2.LINE_AA)
    T['pits'] = blur(pits.astype(np.float32)/255, 0.5); T['chips'] = blur(chips.astype(np.float32)/255, 0.5)
    return T
TEX = make_tex()

def render(a):
    th_a = a - math.pi/2
    Mrot = cv2.getRotationMatrix2D((CAP[0]*S, CAP[1]*S), -math.degrees(a), 1.0)
    R = {k: cv2.warpAffine(v, Mrot, (Wm, Hm)) for k, v in TEX.items()}   # rotan coa tapa
    rgb = np.zeros((Hm, Wm, 3), np.float32); A = np.zeros((Hm, Wm), np.float32)
    def over(col, m):
        nonlocal rgb, A
        m = m[..., None]
        rgb = col*m + rgb*(1-m); A = m[..., 0] + A*(1-m[..., 0])

    def lobed(cx, cy):
        u = (X-cx)/RX; v = (Y-cy)/RY
        th = np.arctan2(v, u); r = np.hypot(u, v)
        return r/(1+LOBE_AMP*np.cos(LOBES*(th-th_a))), th

    def weather(col, T, edge=None, wear=1.0):
        col = col*(1 + 0.09*T['grain'])[..., None]
        col = col*(1 - 0.13*wear*T['grime'])[..., None]                       # sucidade
        col = mix(col, DIRT, 0.16*wear*T['scuff'])                            # po / desgaste
        if edge is not None: col = mix(col, DIRT*1.05, 0.26*edge*(0.5+0.5*T['scuff']))
        col = mix(col, LIGHT, 0.20*T['scr'])                                  # rabuñaduras
        col = col*(1 - 0.45*T['pits'])[..., None]                             # picados
        col = mix(col, LIGHT*0.9, 0.35*T['chips'])                            # lascas
        lum = col.mean(-1, keepdims=True)
        return col*0.84 + lum*0.16                                            # mate: menos saturación

    # sombra de contacto sobre a chapa
    su = ((X-SK[0])/SRX)**2 + ((Y-SK[1])/SRY)**2
    sh = blur((su <= 1).astype(np.float32), 9)
    sh = cv2.warpAffine(sh, np.float32([[1, 0, 9*S], [0, 1, 14*S]]), (Wm, Hm))
    over(np.zeros((Hm, Wm, 3), np.float32), sh*0.6)

    # saia (base): non rota
    u = (X-SK[0])/SRX; v = (Y-SK[1])/SRY; rho = np.hypot(u, v); ths = np.arctan2(v, u)
    phi = np.radians(58)*smooth(0.80, 1.0, rho)
    n = np.stack([np.sin(phi)*np.cos(ths), np.sin(phi)*np.sin(ths), np.cos(phi)], -1)
    col = shade(n, base*0.92, 0.30, 0.95, 0.07, 14)
    col *= (1 - 0.35*smooth(0.9, 1.0, rho))[..., None]
    capsil = (lobed(CAP[0], CAP[1])[0] <= 1)
    for k in range(0, int(WALL)+1, 4): capsil |= (lobed(CAP[0], CAP[1]+k)[0] <= 1)
    cs = cv2.warpAffine(blur(capsil.astype(np.float32), 7), np.float32([[1, 0, 7*S], [0, 1, 12*S]]), (Wm, Hm))
    col *= (1 - 0.55*cs)[..., None]
    col = weather(col, TEX, edge=smooth(0.80, 1.0, rho), wear=1.0)
    col = mix(col, np.array([112, 58, 26], np.float32), 0.22*TEX['grime']*smooth(0.55, 1.0, rho))   # manchas de óxido
    over(col, (rho <= 1).astype(np.float32))

    # parede da tapa (lóbulos)
    wall = np.zeros((Hm, Wm), bool); tmap = np.zeros((Hm, Wm), np.float32); thmap = np.zeros((Hm, Wm), np.float32)
    for k in np.arange(0, WALL+1, 1.5):
        r_k, th_k = lobed(CAP[0], CAP[1]+k); m = (r_k <= 1) & ~wall
        wall |= m; tmap[m] = k/WALL; thmap[m] = th_k[m]
    d = thmap + 0.30*np.sin(LOBES*(thmap-th_a))
    n = np.stack([np.cos(d), np.sin(d), np.full_like(d, 0.25)], -1); n /= np.linalg.norm(n, axis=-1, keepdims=True)
    col = shade(n, base, 0.27, 0.9, 0.10, 12) * (1-0.55*tmap)[..., None]
    col = weather(col, R, edge=smooth(0.0, 0.12, 1-tmap)*0.5, wear=0.9)
    over(col, wall.astype(np.float32))

    # cara superior
    rho_t, _ = lobed(CAP[0], CAP[1]); top = rho_t <= 1
    h = 14*(1 - np.clip(rho_t, 0, 1)**5)
    gy, gx = np.gradient(h); gx *= S; gy *= S
    n = np.stack([-gx, -gy, np.ones_like(h)], -1); n /= np.linalg.norm(n, axis=-1, keepdims=True)
    col = shade(n, base*1.04, 0.22, 1.10, 0.09, 12)
    col += 255*0.04*(np.clip((n*Hv).sum(-1), 0, 1)**40)[..., None]            # brillo moi suave
    col = weather(col, R, edge=smooth(0.84, 1.0, rho_t))
    # punteiro crema, sucio e desgastado
    pm = np.zeros((Hm, Wm), np.uint8)
    p0 = (int((CAP[0]+math.sin(a)*RX*0.22)*S), int((CAP[1]-math.cos(a)*RY*0.22)*S))
    p1 = (int((CAP[0]+math.sin(a)*RX*0.88)*S), int((CAP[1]-math.cos(a)*RY*0.88)*S))
    cv2.line(pm, p0, p1, 255, int(5.5*S), cv2.LINE_AA)
    pm = pm.astype(np.float32)/255
    psh = cv2.warpAffine(blur(pm, 1.6), np.float32([[1, 0, 2*S], [0, 1, 2.5*S]]), (Wm, Hm))
    col *= (1 - 0.50*psh)[..., None]
    cream = np.array([206, 186, 140], np.float32)*(0.80+0.18*np.clip((n*L).sum(-1), 0, 1))[..., None]
    cream = cream*(1 - 0.30*R['grime'])[..., None]*(1 + 0.10*R['grain'])[..., None]
    pmk = pm*0.88*(1 - 0.50*R['scuff'])*(1 - 0.85*R['pits'])
    col = col*(1-pmk[..., None]) + cream*pmk[..., None]
    over(col, top.astype(np.float32))

    out = np.dstack([rgb*A[..., None], A])
    out = cv2.resize(out, (W, H), interpolation=cv2.INTER_AREA)
    al = out[..., 3:4]; rgbs = np.where(al > 1e-4, out[..., :3]/np.maximum(al, 1e-4), 0)
    return np.clip(np.dstack([rgbs, al*255]), 0, 255).astype(np.uint8)

if __name__ == '__main__':
    N, COLS = 48, 8
    start, end = 1.25*math.pi, 2.75*math.pi
    if sys.argv[1] == 'preview':
        for name, pos in [('p0', 0.0), ('p5', 0.5), ('p10', 1.0)]:
            cv2.imwrite(f'spr_{name}.png', cv2.cvtColor(render(start + pos*(end-start)), cv2.COLOR_RGBA2BGRA))
    else:
        rows = (N+COLS-1)//COLS
        strip = np.zeros((rows*H, COLS*W, 4), np.uint8)
        for i in range(N):
            r, c = divmod(i, COLS)
            strip[r*H:(r+1)*H, c*W:(c+1)*W] = render(start + (i/(N-1))*(end-start))
        cv2.imwrite('knob_strip.png', cv2.cvtColor(strip, cv2.COLOR_RGBA2BGRA)); print('strip', strip.shape)
