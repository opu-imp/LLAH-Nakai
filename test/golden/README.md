# 正解データとの比較テスト (test/golden)

OpenCV のバージョンを上げたときに、プログラムの結果が変わっていないことを確かめるためのテスト。
`expected/` には、OpenCV 4.13 でビルドした main (7ecdd8e) の出力を保存してある。
PR #1 では、demo_server、old_stable、llahmos について、この OpenCV 4.13 版の特徴点抽出が移行前のコードと同じ結果になることを確認した。
ただし比べた相手は、移行前のコードを OpenCV 4.13 に残っている旧 C API で動かしたもので、本物の OpenCV 2 でビルドしたものではない。

## 使い方

```sh
for d in demo_server old_stable llahmos anno_extract; do make -C $d; done
bash test/golden/run_golden.sh
```

最後に `一致 N  不一致 0` と出れば、正解データと同じ結果。不一致があると、ファイル名と差分の先頭を表示して、終了コード 1 で終わる。

| オプション | 意味 |
|---|---|
| `--bin-root DIR` | `DIR` 以下にビルドしたプログラムを比較する (別の作業ツリーや別の OpenCV でビルドしたもの) |
| `--keep` | 作業ディレクトリを残す (中身を調べたいとき) |
| `--update` | 正解データを作り直す。基準にするビルドでだけ使う |

## OpenCV のバージョンを切り替えて確かめる

Makefile は、pkg-config で `opencv5` が見つかれば OpenCV 5 を、なければ OpenCV 4 を使う。
`make OPENCV=opencv4` のように指定もできる。
切り替えたときは必ず `make clean` する。`.o` はヘッダが変わっても作り直されない。

```sh
for d in demo_server old_stable llahmos anno_extract; do make -C $d clean; make -C $d OPENCV=opencv5; done
bash test/golden/run_golden.sh
```

Homebrew の OpenCV を入れ替えずに OpenCV 5 で試すときは、OpenCV 5 のソースから必要なモジュールだけをビルドする。
次の手順は、2026-11-27 に OpenCV 5.1.0 で確認した。

```sh
cmake opencv-5.1.0 -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$HOME/opencv5 \
  -DCMAKE_INSTALL_NAME_DIR=$HOME/opencv5/lib -DCMAKE_MACOSX_RPATH=OFF \
  -DBUILD_LIST=core,imgproc,imgcodecs,highgui,videoio -DOPENCV_GENERATE_PKGCONFIG=ON \
  -DWITH_IPP=OFF -DWITH_FFMPEG=OFF -DWITH_OPENEXR=OFF -DWITH_AVIF=OFF -DWITH_PROTOBUF=OFF \
  -DWITH_OPENCL=OFF -DWITH_ADE=OFF -DBUILD_TESTS=OFF -DBUILD_PERF_TESTS=OFF -DBUILD_EXAMPLES=OFF \
  -DBUILD_opencv_apps=OFF -DBUILD_JAVA=OFF -DBUILD_opencv_python3=OFF
make -j8 install
export PKG_CONFIG_PATH=$HOME/opencv5/lib/pkgconfig   # この後の make で opencv5 が選ばれる
```

## 何を比べるか

| プログラム | ケース | 比べる出力 |
|---|---|---|
| demo_server, old_stable | 英文 6 ページで DB を構築し、検索画像 5 枚で検索 | `point.dat` (特徴点), `disc.txt`, `config.dat`, `hash.dat`, 検索結果とスコア |
| demo_server, old_stable | 日本語モード (`-J`) で 2 ページを登録し、1 枚で検索 | 同上 |
| llahmos | 重なりのある 2 枚のモザイキング | 特徴点数、対応点数、スコア、`disc.txt`、出力 BMP |
| anno_extract | 元画像と、赤と青で書き込みをした画像 | クラスタごとの点 (`out/*.txt`, `point.dat`)、`disc.txt` |

- JPEG 以外の出力は、すべてバイト単位で一致する必要がある。
- JPEG はエンコーダ (libjpeg) の版で変わるので、違っても警告だけにする。
- 処理時間の行は、比較の前に取り除く。
- 大きいファイル (`hash.dat`、BMP、JPEG) はチェックサムだけを `expected/MANIFEST.sha256` に残す。それ以外は `expected/files/` に全文がある。

入力画像は `inputs/` にあり、`make_inputs.py` で作った。

- 検索画像は、カメラで撮ったように文字が画像の端で切れている。
- 端の処理 (`ClearImageBorder`、`BORDER_REPLICATE`) の違いも検出できる。
- 画像は PNG にしてある。JPEG のデコーダの差が混ざらないようにするため。

## 検出できることの確認

正解データを作ったときに、次のビルドと比べて、すべて不一致になることを確かめた。

| 比べたビルド | 不一致 |
|---|---|
| OpenCV 2 互換の補正を入れる前 (f4ebad1) | 14 ケース、21 ファイル |
| `ClearImageBorder` だけを外したもの | 21 ファイル |
| `drawContours` の `hierarchy, 1` だけを外したもの | 12 ファイル |
| `GaussianBlur` の `BORDER_REPLICATE` だけを外したもの | 14 ファイル |

同じビルドを別のディレクトリで実行し直したときは、全ファイルが一致した。
AddressSanitizer 付きでビルドしたときは、AddressSanitizer 自身のメッセージが入る `annex/stderr.txt` を除いて、全ファイルが一致した。

## 限界

- 基準は macOS (arm64) で作った。`qsort` の同順位の並びなどが違うので、Linux では正解データと一致しない可能性がある。
- PNG には EXIF がないので、`IMREAD_IGNORE_ORIENTATION` の有無は検出できない。
- anno_extract は、上の 3 つの補正を外しても出力が変わらなかった。anno_extract への補正の影響は、このテストでは確認できない。
- `vs/dcamc` (Windows 版) は対象外。
- 移行前からあるバグで、テストの入力の選び方に影響したもの:
  - `hist.c` の `AddDataHist` は、値がちょうど上限のときに配列の 1 つ外へ書き込む。demo_server、old_stable、llahmos の DB 構築で毎回起きる。AddressSanitizer 付きのビルドでも出力は同じだったので、今の入力では結果に影響していない。
  - llahmos は、2 枚の間の対応点が 4 点未満だと異常終了する。`projrecov.c` の RANSAC が 4 点を無作為に選ぶときに 0 で剰余を取り、配列の外を読むため。対応点が十分に取れる入力を選んである。
  - anno_extract の `GetBasename` は、パスの最初の `.` を拡張子の始まりとみなすので、`../` で始まるパスを渡すとスタックを壊す。`-P` のパスは 128 バイトの配列にコピーされる。そのため `run_golden.sh` は短い相対パスだけを渡す。
