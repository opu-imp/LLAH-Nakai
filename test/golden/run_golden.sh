#!/bin/bash
#
# 正解データとの比較テスト
#
# test/golden/inputs/ の画像で 4 つのプログラムを実行し、出力が
# test/golden/expected/ に保存した正解 (OpenCV 4.13 でビルドした main の出力) と
# 同じかを確認する。OpenCV のバージョンを変えたときに、結果が変わっていないことを確かめるためのもの。
#
# 使い方:
#   cd <プロジェクトルート>
#   (各ディレクトリで make しておく)
#   bash test/golden/run_golden.sh                 # 比較する
#   bash test/golden/run_golden.sh --update        # 正解データを作り直す (基準にするビルドでだけ使う)
#   bash test/golden/run_golden.sh --bin-root DIR  # DIR 以下にビルドしたプログラムを比較する
#   bash test/golden/run_golden.sh --keep          # 作業ディレクトリを消さずに残す
#
# 判定:
#   JPEG 以外の出力 (特徴点、ハッシュ表、スコア、BMP 画像など) はすべて一致が必要。
#   JPEG はエンコーダ (libjpeg) の版で変わるので、違っても警告だけにする。
#   処理時間を表す行は比較の前に取り除く。
#

set -euo pipefail
export LC_ALL=C

GOLDEN_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT_DIR="$(cd "$GOLDEN_DIR/../.." && pwd)"
BIN_ROOT="$ROOT_DIR"
EXPECTED="$GOLDEN_DIR/expected"
UPDATE=0
KEEP=0
TIMEOUT=120

while [ $# -gt 0 ]; do
    case "$1" in
        --update) UPDATE=1 ;;
        --keep) KEEP=1 ;;
        --bin-root) shift; BIN_ROOT="$(cd "$1" && pwd)" ;;
        *) echo "unknown option: $1" >&2; exit 2 ;;
    esac
    shift
done

DS="$BIN_ROOT/demo_server/llahdoc"
OS="$BIN_ROOT/old_stable/llahdoc"
MOS="$BIN_ROOT/llahmos/llahmos"
ANNEX="$BIN_ROOT/anno_extract/annex"
ACR2="$BIN_ROOT/anno_extract/acr2/acr2"
for b in "$DS" "$OS" "$MOS" "$ANNEX" "$ACR2"; do
    if [ ! -x "$b" ]; then
        echo "error: $b がない。先に make でビルドすること" >&2
        exit 2
    fi
done

# 作業ディレクトリ。DB に画像のパスが記録されるので、パスは短い相対パスに固定する。
# 入力は各ケースのディレクトリに in/ というリンクで見せる。anno_extract の GetBasename は
# パスの最初の '.' を拡張子とみなすので、'../' で始まるパスを渡すとスタックを壊す (移行前からのバグ)。
WORK="$(mktemp -d "${TMPDIR:-/tmp}/llah_golden_XXXXXX")"
if [ "$KEEP" -eq 0 ]; then
    trap 'rm -rf "$WORK"' EXIT
fi
cp -R "$GOLDEN_DIR/inputs" "$WORK/in"
mkdir -p "$WORK/out"

# 処理時間の行を取り除く (ここに挙げた形の行だけを消す)
strip_timing() {
    sed -E \
        -e '/^retrieval time : /d' \
        -e '/^(Feature point extraction [12]|Storage|Retrieval|Estimate parameter): [0-9]+ ms$/d' \
        -e '/^Create mosaic image: [0-9]+$/d' \
        -e '/^\(?[A-Za-z &]+: [0-9]+ msec\)?$/d'
}

# run_case <ケース名> <コマンド...>
# ケースごとのディレクトリで実行し、標準出力・標準エラー・終了コードと、書き出されたファイルを残す
run_case() {
    local name="$1"; shift
    local dir="$WORK/out/$name"
    # プログラムは出力先ディレクトリを作らないので、db/ と out/ を用意しておく
    mkdir -p "$dir/db" "$dir/out"
    ln -sfn ../../in "$dir/in"
    local rc=0
    (cd "$dir" && perl -e 'alarm shift; exec @ARGV' "$TIMEOUT" "$@" > .stdout 2> .stderr) || rc=$?
    strip_timing < "$dir/.stdout" > "$dir/stdout.txt"
    strip_timing < "$dir/.stderr" > "$dir/stderr.txt"
    rm -f "$dir/.stdout" "$dir/.stderr"
    echo "$rc" > "$dir/exit.txt"
    printf '  %-28s exit=%s\n' "$name" "$rc"
}

echo "=== 実行 ($BIN_ROOT) ==="

# demo_server / old_stable: DB 構築と検索
for prog in ds os; do
    if [ "$prog" = ds ]; then bin="$DS"; else bin="$OS"; fi
    run_case "${prog}_construct" "$bin" -c in/reg/ png -h db/
    for q in "$WORK"/in/query/*.png; do
        qn="$(basename "$q" .png)"
        run_case "${prog}_${qn}" "$bin" -h "../${prog}_construct/db/" "in/query/$qn.png"
    done
    # 日本語モード (GetConnectedImageJp2)
    run_case "${prog}_jp_construct" "$bin" -J -c in/jp_reg/ png -h db/
    run_case "${prog}_jp_query" "$bin" -J -h "../${prog}_jp_construct/db/" in/jp_query/jq_rot_jpage01.png
done

# llahmos: 重なりのある 2 枚のモザイキング
run_case mos "$MOS" -h db/ in/mos/mos_a.png in/mos/mos_b.png

# anno_extract: 書き込みの抽出
# -P のパスは 128 バイトの固定長配列にコピーされるので、短いリンクで渡す
mkdir -p "$WORK/out/annex"
ln -sfn "$ACR2" "$WORK/out/annex/acr2"
run_case annex "$ANNEX" -n5 -m5 -d50 -c2 -P ./acr2 -o out/ \
    in/annot/orig_page01.png in/annot/annot_page01.png

# 全出力ファイルのチェックサム
(cd "$WORK/out" && find . -type f | sort | xargs shasum -a 256) > "$WORK/MANIFEST.sha256"

# 正解データとして全文を残すファイル (大きいバイナリはチェックサムだけ)
is_stored() {
    case "$1" in
        *.jpg|*.bmp|*/hash.dat) return 1 ;;
        *) return 0 ;;
    esac
}

provenance() {
    echo "generated: $(date -u +%Y-%m-%dT%H:%M:%SZ)"
    echo "commit: $(git -C "$BIN_ROOT" rev-parse HEAD 2>/dev/null || echo unknown)"
    local dirty
    dirty="$(git -C "$BIN_ROOT" status --porcelain -- demo_server old_stable llahmos anno_extract 2>/dev/null | grep -v '^??' || true)"
    echo "source dirs modified: $([ -n "$dirty" ] && echo yes || echo no)"
    echo "os: $(uname -sm) $(sw_vers -productVersion 2>/dev/null || true)"
    echo "compiler: $(c++ --version 2>/dev/null | head -1)"
    echo "pkg-config: opencv4 $(pkg-config --modversion opencv4 2>/dev/null || echo -), opencv5 $(pkg-config --modversion opencv5 2>/dev/null || echo -)"
    for b in "$DS" "$OS" "$MOS" "$ANNEX"; do
        local lib
        lib="$( (otool -L "$b" 2>/dev/null || ldd "$b" 2>/dev/null) | grep -o 'libopencv_core[^ ]*' | head -1)"
        echo "opencv: ${b#$BIN_ROOT/} -> $lib"
    done
}

if [ "$UPDATE" -eq 1 ]; then
    rm -rf "$EXPECTED"
    mkdir -p "$EXPECTED/files"
    cp "$WORK/MANIFEST.sha256" "$EXPECTED/MANIFEST.sha256"
    (cd "$WORK/out" && find . -type f | sort) | while read -r f; do
        if is_stored "$f"; then
            mkdir -p "$EXPECTED/files/$(dirname "$f")"
            cp "$WORK/out/$f" "$EXPECTED/files/$f"
        fi
    done
    provenance > "$EXPECTED/PROVENANCE.txt"
    echo "=== 正解データを更新した: $EXPECTED ==="
    cat "$EXPECTED/PROVENANCE.txt"
    exit 0
fi

echo "=== 比較 ==="
echo "基準:"
sed 's/^/  /' "$EXPECTED/PROVENANCE.txt"

FAIL=0
WARN=0
OK=0
# 両方の一覧をファイル名で突き合わせる
join -1 2 -2 2 -a 1 -a 2 -e MISSING -o 0,1.1,2.1 \
    <(sort -k2 "$EXPECTED/MANIFEST.sha256") <(sort -k2 "$WORK/MANIFEST.sha256") > "$WORK/joined"
while read -r f exp act; do
    if [ "$exp" = "$act" ]; then
        OK=$((OK + 1))
        continue
    fi
    if [ "$exp" = MISSING ]; then
        echo "  FAIL 余分なファイル: $f"; FAIL=$((FAIL + 1)); continue
    fi
    if [ "$act" = MISSING ]; then
        echo "  FAIL ファイルがない: $f"; FAIL=$((FAIL + 1)); continue
    fi
    case "$f" in
        *.jpg) echo "  WARN JPEG が違う (エンコーダの差の可能性): $f"; WARN=$((WARN + 1)) ;;
        *)
            echo "  FAIL 内容が違う: $f"; FAIL=$((FAIL + 1))
            if [ -f "$EXPECTED/files/$f" ]; then
                diff "$EXPECTED/files/$f" "$WORK/out/$f" | head -8 | sed 's/^/        /' || true
            fi
            ;;
    esac
done < "$WORK/joined"

echo "=== 結果: 一致 $OK  不一致 $FAIL  警告 $WARN ==="
if [ "$KEEP" -eq 1 ]; then echo "作業ディレクトリ: $WORK"; fi
[ "$FAIL" -eq 0 ]
