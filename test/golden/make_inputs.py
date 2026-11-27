#!/usr/bin/env python3
"""正解データ (test/golden) 用の入力画像を生成する。

生成した画像はリポジトリに保存してあるので、通常は実行する必要はない。
フォントの描画結果は環境によって変わるため、作り直すと正解データも作り直しになる。

    python3 test/golden/make_inputs.py      # test/golden/inputs/ を上書き

必要なもの: macOS のシステムフォント (Times, ヒラギノ明朝), Pillow, opencv-python, numpy

入力の種類:
  reg/     登録用の文書画像 (英文 6 ページ。page05 は文字が画像の端にかかるよう切り詰める)
  query/   検索用のカメラ風画像 (回転・射影・切り出し。どれも文字が画像の端で切れる)
  mos/     llahmos 用の、重なりのある 2 枚の切り出し画像
  annot/   anno_extract 用のカラー画像 (元画像と、赤と青で書き込みをした画像)
  jp_reg/  日本語モード (-J) の登録用画像 (2 ページ)
  jp_query/ 日本語モードの検索用画像
"""
import os
import random

import cv2
import numpy as np
from PIL import Image, ImageDraw, ImageFont

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, "inputs")
W, H = 1240, 1754  # A4, 150 dpi

EN_FONT = "/System/Library/Fonts/Times.ttc"
JP_FONT = "/System/Library/Fonts/ヒラギノ明朝 ProN.ttc"
SYLLABLES = ("ka ri to me ne sa lo vi qu an pe dor tim ex ul ba ze fro mi gar "
             "sol un te pra cos ver").split()
JP_CHARS = ("あいうえおかきくけこさしすせそたちつてとなにぬねのはひふへほまみむめもやゆよ"
            "らりるれろわをん日本語文書画像検索特徴点抽出登録実験結果方法提案手法精度向上")


def en_page(seed):
    rng = random.Random(seed)
    font = ImageFont.truetype(EN_FONT, 26)
    img = Image.new("L", (W, H), 255)
    d = ImageDraw.Draw(img)
    y = 120
    while y < H - 140:
        x = 110
        while True:
            w = "".join(rng.choice(SYLLABLES) for _ in range(rng.randint(1, 3)))
            tw = d.textlength(w, font=font)
            if x + tw > W - 110:
                break
            d.text((x, y), w, fill=0, font=font)
            x += tw + 12
        y += 40
    return np.array(img)


def jp_page(seed):
    rng = random.Random(seed)
    font = ImageFont.truetype(JP_FONT, 28)
    img = Image.new("L", (W, H), 255)
    d = ImageDraw.Draw(img)
    for y in range(120, H - 140, 46):
        n = 34
        s = "".join(rng.choice(JP_CHARS) for _ in range(n))
        # 句読点を混ぜる
        s = "".join(c + ("、" if rng.random() < 0.08 else "") for c in s)[:n]
        d.text((110, y), s, fill=0, font=font)
    return np.array(img)


def warp_affine(img, angle, scale, fill):
    h, w = img.shape[:2]
    m = cv2.getRotationMatrix2D((w / 2, h / 2), angle, scale)
    return cv2.warpAffine(img, m, (w, h), flags=cv2.INTER_LINEAR, borderValue=fill)


def warp_persp(img, fill):
    h, w = img.shape[:2]
    src = np.float32([[0, 0], [w, 0], [w, h], [0, h]])
    dst = np.float32([[60, 40], [w - 20, 110], [w - 120, h - 30], [10, h - 90]])
    m = cv2.getPerspectiveTransform(src, dst)
    return cv2.warpPerspective(img, m, (w, h), flags=cv2.INTER_LINEAR, borderValue=fill)


def save(sub, name, img):
    d = os.path.join(OUT, sub)
    os.makedirs(d, exist_ok=True)
    cv2.imwrite(os.path.join(d, name), img, [cv2.IMWRITE_PNG_COMPRESSION, 9])


def main():
    pages = [en_page(1000 + i) for i in range(7)]  # page06 は登録しない (検索で外れる例)

    for i in range(5):
        save("reg", f"page{i:02d}.png", pages[i])
    # page05: 余白を落として、文字が画像の上下左右の端にかかるようにする
    save("reg", "page05.png", pages[5][130:1600, 120:1120])

    # 検索画像: カメラで撮ったように、紙の外側は暗く、文字は画像の端で切れる
    save("query", "q_rot_page01.png", warp_affine(pages[1], 12, 0.9, 60)[300:1260, 160:1080])
    save("query", "q_persp_page03.png", warp_persp(pages[3], 40)[250:1210, 80:1180])
    save("query", "q_crop_page04.png", pages[4][500:1300, 200:1000])
    save("query", "q_tight_page05.png", warp_affine(pages[5], -4, 1.0, 50)[400:1200, 300:1000])
    save("query", "q_unreg_page06.png", pages[6][400:1200, 150:1050])

    # llahmos: 同じページを横にずらして撮った、重なりのある 2 枚
    save("mos", "mos_a.png", pages[2][200:1300, 40:840])
    save("mos", "mos_b.png", pages[2][260:1360, 420:1220])

    # anno_extract: カラーの元画像と、赤と青のペンで書き込みをした画像
    orig = cv2.cvtColor(pages[1], cv2.COLOR_GRAY2BGR)
    annot = orig.copy()
    red, blue = (40, 40, 220), (200, 60, 30)
    cv2.ellipse(annot, (520, 400), (180, 45), 0, 0, 360, red, 4)
    cv2.line(annot, (180, 905), (930, 912), red, 4)
    cv2.putText(annot, "check this", (650, 1230), cv2.FONT_HERSHEY_SCRIPT_SIMPLEX, 1.8, blue, 4)
    cv2.arrowedLine(annot, (1000, 600), (820, 700), blue, 4)
    save("annot", "orig_page01.png", orig)
    save("annot", "annot_page01.png", warp_affine(annot, 2, 1.0, (255, 255, 255)))

    # 日本語モード
    jp = [jp_page(2000 + i) for i in range(2)]
    for i, p in enumerate(jp):
        save("jp_reg", f"jpage{i:02d}.png", p)
    save("jp_query", "jq_rot_jpage01.png", warp_affine(jp[1], 8, 0.85, 60)[300:1260, 150:1090])


if __name__ == "__main__":
    main()
