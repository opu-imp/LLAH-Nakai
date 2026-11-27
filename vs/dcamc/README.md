# dcamc (Windows 版)

カメラ (DirectShow) から取り込んだ画像で LLAH の検索をする、Windows 専用のプログラム。
以前は Visual Studio 2005 のプロジェクト (`dcamc.sln`, `dcamc.vcproj`) だったが、今の Visual Studio と Windows SDK ではビルドできないので、CMake に置き換えた。
ソースファイル、リンクするライブラリ、プリプロセッサ定義は元のプロジェクトと同じ。

## 必要なもの

- Visual Studio 2022 以降 (「C++ によるデスクトップ開発」)。CI では Visual Studio 2026 で確認している
- CMake 3.16 以降 (Visual Studio に付属するものでよい)
- OpenCV 4 または OpenCV 5。core, imgproc, imgcodecs, highgui, videoio を使う

## ビルド

リポジトリのルートで、OpenCV の場所を `OpenCV_DIR` に指定する。

```bat
cmake -S vs/dcamc -B build -A x64 -DOpenCV_DIR=C:/opencv/build
cmake --build build --config Release
```

- `OpenCV_DIR` には、`OpenCVConfig.cmake` があるディレクトリを指定する。OpenCV 公式の Windows 版 (リリースページの `opencv-X-windows.exe`、x64) なら、展開した `opencv/build`。
- 元のプロジェクトと同じ 32 ビット版にするときは `-A Win32` にする。その場合は 32 ビット用にビルドした OpenCV が要る。
- `build/dcamc.sln` を Visual Studio で開いてもよい。Visual Studio の「フォルダーを開く」で `vs/dcamc` を開いても、CMake プロジェクトとして読み込まれる。

OpenCV の公式配布に Windows 版がないバージョンは、ソースから必要なモジュールだけをビルドする。
手順は `.github/workflows/dcamc-windows.yml` の「Build OpenCV」と同じ。

## 実行

- 作業ディレクトリの `client.ini` を読む。Visual Studio から実行すると、作業ディレクトリは `vs/dcamc/dcamc` になる。
- OpenCV の DLL (`opencv_*.dll`) が `PATH` にあるか、`dcamc.exe` と同じ場所にある必要がある。

## 移行での変更点

- `qedit.h` は Windows SDK から削除されたので、使っている SampleGrabber の宣言だけを `compat/qedit.h` に用意した。ソースの `#include <qedit.h>` はそのまま。
- ソースは Shift_JIS なので、どの言語の Windows でも同じに読めるよう、`/source-charset:.932 /execution-charset:.932` を付けている。
- `proctime.h` で `windows.h` を読み込むようにした。`DWORD` を使っているのに、`nn5dcamc.cpp` は `windows.h` なしでこのヘッダを読んでいる。移行前は OpenCV 1.x の `highgui.h` が `windows.h` を読み込んでいたので通っていた。
- OpenCV 5 では `goodFeaturesToTrack` が features モジュールに移ったので、OpenCV 5 のときは features もリンクする。
- GitHub Actions (`.github/workflows/dcamc-windows.yml`) でビルドを確認している。カメラがないので、実行はしていない。
  - OpenCV 公式の Windows 版 5.1.0 と 4.13.0 (x64)
  - ソースからビルドした OpenCV 5.1.0 (x64)、4.13.0 (x64 と Win32)
