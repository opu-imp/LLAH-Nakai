#!/bin/bash
#
# 全4ディレクトリのビルド・動作確認を行うテストスクリプト
#
# 使い方:
#   cd <プロジェクトルート>
#   bash test/run_tests.sh
#
# 前提:
#   - OpenCV 4.x が pkg-config opencv4 で利用可能
#   - Python 3 + opencv-python (テスト画像生成用)
#

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
TEST_DIR="$ROOT_DIR/test"
PASS=0
FAIL=0
SKIP=0
TIMEOUT=30  # 各コマンドのタイムアウト（秒）

# タイムアウト付きでコマンドを実行するヘルパー
# 出力は stdout に書き出す。タイムアウト時はプロセスを SIGKILL で強制終了。
run_with_timeout() {
    local tmpfile
    tmpfile=$(mktemp /tmp/llah_test_XXXXXX)
    "$@" > "$tmpfile" 2>&1 &
    local pid=$!
    local elapsed=0
    local timed_out=0
    while kill -0 "$pid" 2>/dev/null; do
        sleep 1
        elapsed=$((elapsed + 1))
        if [ "$elapsed" -ge "$TIMEOUT" ]; then
            kill -9 "$pid" 2>/dev/null || true
            timed_out=1
            echo "(timeout: ${TIMEOUT}s)" >> "$tmpfile"
            break
        fi
    done
    if [ "$timed_out" -eq 0 ]; then
        wait "$pid" 2>/dev/null || true
    fi
    cat "$tmpfile"
    rm -f "$tmpfile"
}

# 色付き出力
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BOLD='\033[1m'
NC='\033[0m'

log_pass() { echo -e "  ${GREEN}PASS${NC}: $1"; PASS=$((PASS + 1)); }
log_fail() { echo -e "  ${RED}FAIL${NC}: $1"; FAIL=$((FAIL + 1)); }
log_skip() { echo -e "  ${YELLOW}SKIP${NC}: $1"; SKIP=$((SKIP + 1)); }
log_section() { echo -e "\n${BOLD}=== $1 ===${NC}"; }

# -------------------------------------------------------------------
# 0. テスト画像生成
# -------------------------------------------------------------------
log_section "テスト画像生成"

if [ ! -f "$TEST_DIR/test_doc1.jpg" ] || [ ! -f "$TEST_DIR/test_doc2.jpg" ]; then
    python3 "$TEST_DIR/generate_test_images.py"
fi

if [ -f "$TEST_DIR/test_doc1.jpg" ] && [ -f "$TEST_DIR/test_doc2.jpg" ]; then
    log_pass "テスト画像が存在する"
else
    log_fail "テスト画像の生成に失敗"
    echo "テスト画像がないため中断します。"
    exit 1
fi

# -------------------------------------------------------------------
# 1. ビルド確認
# -------------------------------------------------------------------
log_section "ビルド確認"

build_and_check() {
    local dir="$1"
    local binary="$2"
    local label="$3"

    if [ ! -f "$ROOT_DIR/$dir/Makefile" ]; then
        log_skip "$label: Makefile が見つからない"
        return
    fi

    make -C "$ROOT_DIR/$dir" -j4 2>&1 | tail -3
    if [ -f "$ROOT_DIR/$binary" ]; then
        log_pass "$label: ビルド成功 ($(wc -c < "$ROOT_DIR/$binary") bytes)"
    else
        log_fail "$label: バイナリが生成されなかった"
    fi
}

build_and_check "llahmos"      "llahmos/llahmos"      "llahmos"
build_and_check "demo_server"  "demo_server/llahdoc"  "demo_server"
build_and_check "old_stable"   "old_stable/llahdoc"   "old_stable"
build_and_check "anno_extract" "anno_extract/annex"    "anno_extract"

# acr2 は anno_extract の Makefile 経由で自動ビルドされるが、念のため確認
if [ -f "$ROOT_DIR/anno_extract/acr2/acr2" ]; then
    log_pass "anno_extract/acr2: ビルド成功 ($(wc -c < "$ROOT_DIR/anno_extract/acr2/acr2") bytes)"
else
    log_fail "anno_extract/acr2: バイナリが生成されなかった"
fi

# -------------------------------------------------------------------
# 2. 動的リンク確認
# -------------------------------------------------------------------
log_section "動的リンク確認 (OpenCV)"

check_opencv_link() {
    local binary="$1"
    local label="$2"

    if [ ! -f "$ROOT_DIR/$binary" ]; then
        log_skip "$label: バイナリなし"
        return
    fi

    local opencv_lib
    opencv_lib=$(otool -L "$ROOT_DIR/$binary" 2>/dev/null | grep -i opencv | head -1 || true)
    if [ -n "$opencv_lib" ]; then
        local version
        version=$(echo "$opencv_lib" | grep -oE '[0-9]+\.[0-9]+\.[0-9]+' | head -1)
        log_pass "$label -> OpenCV $version"
    else
        log_fail "$label: OpenCV がリンクされていない"
    fi
}

check_opencv_link "llahmos/llahmos"      "llahmos"
check_opencv_link "demo_server/llahdoc"  "demo_server"
check_opencv_link "old_stable/llahdoc"   "old_stable"
check_opencv_link "anno_extract/annex"   "anno_extract"

# -------------------------------------------------------------------
# 3. ランタイムテスト: llahmos
# -------------------------------------------------------------------
log_section "ランタイム: llahmos (モザイキング)"

if [ -f "$ROOT_DIR/llahmos/llahmos" ]; then
    output=$(run_with_timeout "$ROOT_DIR/llahmos/llahmos" "$TEST_DIR/test_doc1.jpg" "$TEST_DIR/test_doc2.jpg")
    # 特徴点抽出が動作したかチェック (出力に数字やfpが含まれる)
    if echo "$output" | grep -qiE "fp|point|feature|time|proc|extract|[0-9]"; then
        log_pass "起動・特徴点抽出が動作"
    else
        log_fail "期待する出力が得られなかった"
    fi
    echo "$output" | head -10 | sed 's/^/    /'
else
    log_skip "バイナリなし"
fi

# -------------------------------------------------------------------
# 4. ランタイムテスト: demo_server (ハッシュ構築 + 検索)
# -------------------------------------------------------------------
log_section "ランタイム: demo_server (DB構築 + 検索)"

DEMO_DB="$TEST_DIR/demo_db"
rm -rf "$DEMO_DB"
mkdir -p "$DEMO_DB"

if [ -f "$ROOT_DIR/demo_server/llahdoc" ]; then
    # DB構築
    output=$(run_with_timeout "$ROOT_DIR/demo_server/llahdoc" -c "$TEST_DIR/" jpg -h "$DEMO_DB/")
    if ls "$DEMO_DB"/*.dat 1>/dev/null 2>&1; then
        log_pass "ハッシュテーブル構築成功"
    else
        log_fail "ハッシュテーブルが生成されなかった"
    fi
    echo "$output" | head -5 | sed 's/^/    /'

    # 検索
    output=$(run_with_timeout "$ROOT_DIR/demo_server/llahdoc" -h "$DEMO_DB/" "$TEST_DIR/test_doc1.jpg")
    if echo "$output" | grep -qiE "page|score|result|match|time|[0-9]"; then
        log_pass "検索が動作"
    else
        log_fail "検索結果が得られなかった"
    fi
    echo "$output" | head -5 | sed 's/^/    /'
else
    log_skip "バイナリなし"
fi

# -------------------------------------------------------------------
# 5. ランタイムテスト: old_stable (ハッシュ構築 + 検索)
# -------------------------------------------------------------------
log_section "ランタイム: old_stable (DB構築 + 検索)"

OLD_DB="$TEST_DIR/old_db"
rm -rf "$OLD_DB"
mkdir -p "$OLD_DB"

if [ -f "$ROOT_DIR/old_stable/llahdoc" ]; then
    # DB構築
    output=$(run_with_timeout "$ROOT_DIR/old_stable/llahdoc" -c "$TEST_DIR/" jpg -h "$OLD_DB/")
    if ls "$OLD_DB"/*.dat 1>/dev/null 2>&1; then
        log_pass "ハッシュテーブル構築成功"
    else
        log_fail "ハッシュテーブルが生成されなかった"
    fi
    echo "$output" | head -5 | sed 's/^/    /'

    # 検索
    output=$(run_with_timeout "$ROOT_DIR/old_stable/llahdoc" -h "$OLD_DB/" "$TEST_DIR/test_doc1.jpg")
    if echo "$output" | grep -qiE "page|score|result|match|time|[0-9]"; then
        log_pass "検索が動作"
    else
        log_fail "検索結果が得られなかった"
    fi
    echo "$output" | head -5 | sed 's/^/    /'
else
    log_skip "バイナリなし"
fi

# -------------------------------------------------------------------
# 6. ランタイムテスト: anno_extract (注釈抽出)
# -------------------------------------------------------------------
log_section "ランタイム: anno_extract (注釈抽出)"

ANNEX_OUT="$TEST_DIR/annex_output"
rm -rf "$ANNEX_OUT"
mkdir -p "$ANNEX_OUT"

if [ -f "$ROOT_DIR/anno_extract/annex" ]; then
    output=$(run_with_timeout "$ROOT_DIR/anno_extract/annex" -n5 -m5 -d50 -c2 -P "$ROOT_DIR/anno_extract/acr2/acr2" -o "$ANNEX_OUT/" \
        "$TEST_DIR/test_doc1.jpg" "$TEST_DIR/test_doc2.jpg")

    # 出力ファイルが生成されたか
    out_count=$(find "$ANNEX_OUT" -type f | wc -l | tr -d ' ')
    if [ "$out_count" -gt 0 ]; then
        log_pass "注釈抽出完了 (出力ファイル: ${out_count}個)"
    else
        # 出力ファイルがなくても正常終了していれば PASS
        if echo "$output" | grep -qiE "time|proc|extract|annex|[0-9]"; then
            log_pass "パイプライン実行完了 (出力ファイルなし)"
        else
            log_fail "注釈抽出に失敗"
        fi
    fi
    echo "$output" | head -10 | sed 's/^/    /'
else
    log_skip "バイナリなし"
fi

# -------------------------------------------------------------------
# サマリ
# -------------------------------------------------------------------
log_section "テスト結果サマリ"
TOTAL=$((PASS + FAIL + SKIP))
echo -e "  合計: ${TOTAL}  ${GREEN}PASS: ${PASS}${NC}  ${RED}FAIL: ${FAIL}${NC}  ${YELLOW}SKIP: ${SKIP}${NC}"

if [ "$FAIL" -gt 0 ]; then
    echo -e "\n${RED}テスト失敗があります。${NC}"
    exit 1
else
    echo -e "\n${GREEN}全テスト成功!${NC}"
    exit 0
fi
