#!/usr/bin/env python3
"""report/results.csv 를 읽어 비교 그래프를 SVG로 찍는다.

외부 라이브러리를 쓰지 않는다. matplotlib 설치 여부와 무관하게 돌아야
어느 환경에서든 `make charts` 한 줄로 그림이 나온다. SVG는 그냥 텍스트라
좌표를 직접 계산해 태그를 찍으면 된다.

    make charts        (= make csv 뒤에 python3 tools/plot.py)

만드는 파일 (report/ 아래)
    growth-time.svg          n을 키울 때 실행 시간 (로그-로그)
    growth-compares.svg      n을 키울 때 비교 횟수 (로그-로그)
    shapes-time.svg          입력 모양별 실행 시간
    shapes-compares.svg      입력 모양별 비교 횟수
    shapes-moves.svg         입력 모양별 이동 횟수
    shapes-depth.svg         입력 모양별 재귀 깊이
"""

import csv
import math
import os

HERE = os.path.dirname(os.path.abspath(__file__))
REPORT = os.path.join(os.path.dirname(HERE), "report")
CSV_PATH = os.path.join(REPORT, "results.csv")

# 정렬마다 색을 고정한다. 그림이 여러 장이라 색이 흔들리면 읽기 어렵다.
COLORS = {"merge": "#1d4ed8", "quick": "#c2410c", "heap": "#15803d"}
ORDER = ["merge", "quick", "heap"]

W, H = 640, 400
# 그림 영역의 여백. 아래쪽이 넓은 것은 눈금·축 이름 밑에 범례가 한 줄 더
# 들어가기 때문이다.
PAD_L, PAD_R, PAD_T, PAD_B = 78, 18, 46, 78


def load():
    with open(CSV_PATH, newline="", encoding="utf-8") as f:
        rows = list(csv.DictReader(f))
    for r in rows:
        r["n"] = int(r["n"])
        r["millis"] = float(r["millis"])
        r["compares"] = int(r["compares"])
        r["moves"] = int(r["moves"])
        r["depth"] = int(r["depth"])
    return rows


def esc(s):
    return str(s).replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;")


def header(title):
    return [
        f'<svg xmlns="http://www.w3.org/2000/svg" width="{W}" height="{H}" '
        f'viewBox="0 0 {W} {H}" font-family="sans-serif">',
        f'<rect width="{W}" height="{H}" fill="#ffffff"/>',
        f'<text x="{W / 2}" y="27" font-size="15" font-weight="600" '
        f'text-anchor="middle" fill="#111827">{esc(title)}</text>',
    ]


def legend(names):
    """범례는 그림 맨 아래 가운데에 가로로 깐다.

    한 칸이 색 네모(11) + 사이(5) + 이름 이므로 step만큼 띄우고, 전체 너비를
    재어 W의 가운데에 맞춘다."""
    step = 80
    width = (len(names) - 1) * step + 56
    x0 = (W - width) / 2
    y = H - 16

    out = []
    for i, name in enumerate(names):
        cx = x0 + i * step
        out.append(
            f'<rect x="{cx:.1f}" y="{y - 9}" width="11" height="11" rx="2" '
            f'fill="{COLORS[name]}"/>'
        )
        out.append(
            f'<text x="{cx + 16:.1f}" y="{y}" font-size="12" '
            f'fill="#374151">{name}</text>'
        )
    return out


def axes(xlabel, ylabel):
    x0, y0 = PAD_L, H - PAD_B
    x1, y1 = W - PAD_R, PAD_T
    return [
        f'<line x1="{x0}" y1="{y0}" x2="{x1}" y2="{y0}" stroke="#9ca3af" stroke-width="1"/>',
        f'<line x1="{x0}" y1="{y0}" x2="{x0}" y2="{y1}" stroke="#9ca3af" stroke-width="1"/>',
        f'<text x="{(x0 + x1) / 2}" y="{y0 + 36}" font-size="12" fill="#6b7280" '
        f'text-anchor="middle">{esc(xlabel)}</text>',
        f'<text x="16" y="{(y0 + y1) / 2}" font-size="12" fill="#6b7280" '
        f'text-anchor="middle" transform="rotate(-90 16 {(y0 + y1) / 2})">'
        f'{esc(ylabel)}</text>',
    ]


def line_chart(rows, field, title, ylabel, path):
    """n을 키우며 재는 그림. 양축 로그 — 기울기가 곧 복잡도 차수다."""
    data = {}
    for r in rows:
        if r["scope"] != "growth":
            continue
        data.setdefault(r["algo"], []).append((r["n"], r[field]))
    for v in data.values():
        v.sort()
    if not data:
        return

    xs = sorted({n for v in data.values() for n, _ in v})
    ys = [y for v in data.values() for _, y in v if y > 0]
    if not ys:
        return

    lx0, lx1 = math.log10(min(xs)), math.log10(max(xs))
    ly0, ly1 = math.log10(min(ys)), math.log10(max(ys))
    if lx1 - lx0 < 1e-9:
        lx1 = lx0 + 1
    pad = (ly1 - ly0) * 0.08 or 0.5
    ly0, ly1 = ly0 - pad, ly1 + pad

    px0, px1 = PAD_L, W - PAD_R
    py0, py1 = H - PAD_B, PAD_T

    def X(n):
        return px0 + (math.log10(n) - lx0) / (lx1 - lx0) * (px1 - px0)

    def Y(v):
        return py0 - (math.log10(v) - ly0) / (ly1 - ly0) * (py0 - py1)

    out = header(title)

    # 가로 눈금 — 10의 거듭제곱마다
    e = math.floor(ly0)
    while e <= ly1:
        if e >= ly0:
            gy = Y(10 ** e)
            out.append(
                f'<line x1="{px0}" y1="{gy:.1f}" x2="{px1}" y2="{gy:.1f}" '
                f'stroke="#e5e7eb" stroke-width="1"/>'
            )
            out.append(
                f'<text x="{px0 - 8}" y="{gy + 4:.1f}" font-size="11" fill="#6b7280" '
                f'text-anchor="end">1e{e}</text>'
            )
        e += 1

    out += axes("n (원소 개수, 로그축)", ylabel + " (로그축)")

    for n in xs:
        out.append(
            f'<text x="{X(n):.1f}" y="{py0 + 18}" font-size="11" fill="#6b7280" '
            f'text-anchor="middle">{n}</text>'
        )

    for name in ORDER:
        if name not in data:
            continue
        pts = " ".join(f"{X(n):.1f},{Y(v):.1f}" for n, v in data[name] if v > 0)
        out.append(
            f'<polyline points="{pts}" fill="none" stroke="{COLORS[name]}" '
            f'stroke-width="2"/>'
        )
        for n, v in data[name]:
            if v > 0:
                out.append(
                    f'<circle cx="{X(n):.1f}" cy="{Y(v):.1f}" r="3" '
                    f'fill="{COLORS[name]}"/>'
                )

    out += legend([n for n in ORDER if n in data])
    out.append("</svg>")
    write(path, out)


def bar_chart(rows, field, title, ylabel, path, logscale=False):
    """입력 모양별로 묶어 세우는 막대. 같은 n에서 무엇이 달라지는지 본다.

    같은 자료를 선형과 로그 두 벌로 그린다. 하나로는 절반씩 놓치기 때문이다.

      선형 — 격차의 크기를 보여 준다. 막대 높이가 곧 값의 비율이라, 퀵의 최악만
             천장을 뚫고 나머지가 바닥에 깔리는 그림이 그대로 결론이 된다.
             대신 작은 값끼리는 견줄 수 없다.
      로그 — 작은 값도 자기 높이를 갖는다. 대신 눈금 한 칸이 10배라 630배의
             격차가 두 칸 남짓으로 눌려 보인다. 그리고 0은 로그 축에 놓을
             자리가 아예 없다 — 없는 것과 작은 것은 다르다.
    """
    kinds, data = [], {}
    for r in rows:
        if r["scope"] != "kinds":
            continue
        if r["kind"] not in kinds:
            kinds.append(r["kind"])
        data[(r["kind"], r["algo"])] = r[field]
    if not kinds:
        return

    vals = [v for v in data.values() if v > 0]
    if not vals:
        return
    vmax, vmin = max(vals), min(vals)

    px0, px1 = PAD_L, W - PAD_R
    py0, py1 = H - PAD_B, PAD_T
    plot_h = py0 - py1

    if logscale:
        top = math.log10(vmax) + 0.35
        base = math.log10(vmin) - 0.25
        if top - base < 1e-9:
            top = base + 1

        def height_of(v):
            if v <= 0:
                return 0.0
            return max(0.0, (math.log10(v) - base) / (top - base)) * plot_h
    else:
        def height_of(v):
            return v / vmax * plot_h * 0.9

    out = header(title)

    # 선형 축에는 눈금을 깐다. 그래야 막대 높이를 값으로 읽을 수 있다.
    # 로그 축 쪽은 눈금 한 칸이 10배라 오히려 오해를 부르므로, 막대마다 적어
    # 둔 숫자로 갈음한다.
    if not logscale:
        for f in (0.25, 0.5, 0.75, 1.0):
            gy = py0 - f * plot_h * 0.9
            tick = f * vmax
            label = short(int(round(tick))) if isinstance(vmax, int) else short(tick)
            out.append(
                f'<line x1="{px0}" y1="{gy:.1f}" x2="{px1}" y2="{gy:.1f}" '
                f'stroke="#e5e7eb" stroke-width="1"/>'
            )
            out.append(
                f'<text x="{px0 - 8}" y="{gy + 4:.1f}" font-size="11" '
                f'fill="#6b7280" text-anchor="end">{esc(label)}</text>'
            )

    out += axes("입력 모양", ylabel + (" (로그축)" if logscale else ""))

    group_w = (px1 - px0) / len(kinds)
    bar_w = min(30.0, group_w / (len(ORDER) + 1.4))

    for gi, kind in enumerate(kinds):
        gx = px0 + gi * group_w
        out.append(
            f'<text x="{gx + group_w / 2:.1f}" y="{py0 + 18}" font-size="11" '
            f'fill="#6b7280" text-anchor="middle">{esc(kind)}</text>'
        )
        for bi, name in enumerate(ORDER):
            v = data.get((kind, name))
            if v is None:
                continue
            bx = gx + group_w / 2 + (bi - 1) * (bar_w + 4) - bar_w / 2
            bh = height_of(v)
            out.append(
                f'<rect x="{bx:.1f}" y="{py0 - bh:.1f}" width="{bar_w:.1f}" '
                f'height="{bh:.1f}" fill="{COLORS[name]}" rx="2"/>'
            )
            label = short(v)
            out.append(
                f'<text x="{bx + bar_w / 2:.1f}" y="{py0 - bh - 5:.1f}" font-size="9" '
                f'fill="#374151" text-anchor="middle">{esc(label)}</text>'
            )

    out += legend(ORDER)
    out.append("</svg>")
    write(path, out)


def short(v):
    """막대 위 숫자. 33550336을 그대로 찍으면 옆 막대를 침범하므로 줄여 쓴다."""
    if isinstance(v, float):
        return f"{v:.2f}"
    if v >= 1_000_000:
        return f"{v / 1_000_000:.1f}M"
    if v >= 10_000:
        return f"{v / 1000:.0f}K"
    return f"{v:,}"


def write(path, lines):
    os.makedirs(REPORT, exist_ok=True)
    full = os.path.join(REPORT, path)
    with open(full, "w", encoding="utf-8") as f:
        f.write("\n".join(lines))
    print(f"  report/{path}")


def shapes_n(rows):
    """입력 모양별 그림이 어느 n에서 잰 것인지. 제목에 박아 넣지 않고 CSV에서 읽는다."""
    ns = {r["n"] for r in rows if r["scope"] == "kinds"}
    return ns.pop() if len(ns) == 1 else None


def main():
    if not os.path.exists(CSV_PATH):
        raise SystemExit("report/results.csv 가 없다. 먼저 `make csv` 를 돌린다.")
    rows = load()
    print("그래프 생성")

    line_chart(rows, "millis", "n을 키울 때 실행 시간 (무작위 입력)", "시간 (ms)",
               "growth-time.svg")
    line_chart(rows, "compares", "n을 키울 때 비교 횟수 (무작위 입력)", "비교 횟수",
               "growth-compares.svg")

    n = shapes_n(rows)
    at = f" (n = {n})" if n else ""
    # 시간은 로그만 그린다. 선형으로 그리면 바로 아래 비교 횟수 그림과 같은
    # 모양이 되어 (퀵만 솟고 나머지는 바닥) 두 장을 실을 값이 없다.
    bar_chart(rows, "millis", "입력 모양별 실행 시간" + at, "시간 (ms)",
              "shapes-time.svg", logscale=True)

    # 비교와 깊이는 두 벌씩. 퀵의 최악이 얼마나 압도적인지는 선형에서만 보이고,
    # 나머지 둘을 서로 견주는 것은 로그에서만 된다.
    bar_chart(rows, "compares", "입력 모양별 비교 횟수" + at + " — 선형 축",
              "비교 횟수", "shapes-compares-linear.svg")
    bar_chart(rows, "compares", "입력 모양별 비교 횟수" + at + " — 로그 축",
              "비교 횟수", "shapes-compares.svg", logscale=True)
    bar_chart(rows, "depth", "입력 모양별 재귀 깊이" + at + " — 선형 축",
              "재귀 깊이", "shapes-depth-linear.svg")
    bar_chart(rows, "depth", "입력 모양별 재귀 깊이" + at + " — 로그 축",
              "재귀 깊이", "shapes-depth.svg", logscale=True)

    # 이동은 선형만. 값의 범위가 좁아 로그가 필요 없고, 무엇보다 퀵이 정렬된
    # 입력에서 기록하는 0회를 로그 축에는 그릴 자리가 없다.
    bar_chart(rows, "moves", "입력 모양별 이동 횟수" + at, "이동 횟수",
              "shapes-moves.svg")


if __name__ == "__main__":
    main()
