#!/usr/bin/env python3
"""
テスト用画像を生成するスクリプト。
文書画像風のテスト画像を2枚（原本＋注釈付き）作成する。

必要環境: Python 3, OpenCV (pip install opencv-python)

使い方:
    python3 generate_test_images.py
"""

import os
import sys

try:
    import cv2
    import numpy as np
except ImportError:
    print("ERROR: opencv-python が必要です。")
    print("  pip install opencv-python")
    sys.exit(1)

OUTPUT_DIR = os.path.dirname(os.path.abspath(__file__))
IMG_W, IMG_H = 640, 480
NUM_FEATURE_POINTS = 30
SEED = 42


def generate_test_images():
    np.random.seed(SEED)

    # --- 画像1: 原本（テスト文書風） ---
    img1 = np.ones((IMG_H, IMG_W, 3), dtype=np.uint8) * 240  # 薄いグレー背景

    # テキスト風の線を描画
    for y in range(60, IMG_H - 40, 30):
        length = np.random.randint(200, 550)
        cv2.line(img1, (40, y), (40 + length, y), (60, 60, 60), 1)

    # タイトル
    cv2.putText(img1, "Test Document Page 1", (120, 35), cv2.FONT_HERSHEY_SIMPLEX, 0.9, (30, 30, 30), 2)

    # 特徴点になる小さなドット
    for _ in range(NUM_FEATURE_POINTS):
        x = np.random.randint(30, IMG_W - 30)
        y = np.random.randint(30, IMG_H - 30)
        cv2.circle(img1, (x, y), 3, (0, 0, 0), -1)

    # --- 画像2: 注釈付き（原本 + 赤い注釈） ---
    img2 = img1.copy()

    # 赤い下線（注釈）
    cv2.line(img2, (40, 155), (350, 155), (0, 0, 255), 3)
    cv2.line(img2, (40, 245), (280, 245), (0, 0, 255), 3)

    # 赤丸囲み
    cv2.circle(img2, (450, 300), 60, (0, 0, 255), 2)

    # 手書きメモ風
    cv2.putText(img2, "IMPORTANT!", (320, 180), cv2.FONT_HERSHEY_SIMPLEX, 0.8, (0, 0, 255), 2)
    cv2.putText(img2, "Check this", (370, 310), cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 0, 255), 2)

    # --- 保存 ---
    path1 = os.path.join(OUTPUT_DIR, "test_doc1.jpg")
    path2 = os.path.join(OUTPUT_DIR, "test_doc2.jpg")

    cv2.imwrite(path1, img1)
    cv2.imwrite(path2, img2)

    print(f"Generated: {path1}")
    print(f"Generated: {path2}")
    return path1, path2


if __name__ == "__main__":
    generate_test_images()
