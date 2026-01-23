#!/bin/bash
# Download historical BTCUSDT trade data from Binance
# Supports date ranges and skips already-downloaded files

set -e  # Exit on error

# Help message
show_help() {
    cat << EOF
Usage: $0 [SYMBOL] [OPTIONS]

Download historical trade data from Binance.

ARGUMENTS:
    SYMBOL          Trading pair (default: BTCUSDT)

OPTIONS:
    -d, --days N          Download last N days (default: 7)
    -s, --start DATE      Start date (YYYY-MM-DD)
    -e, --end DATE        End date (YYYY-MM-DD, default: yesterday)
    -h, --help            Show this help message

EXAMPLES:
    # Download last 7 days of BTCUSDT
    $0 BTCUSDT --days 7

    # Download specific date range
    $0 BTCUSDT --start 2024-01-01 --end 2024-01-31

    # Download last 30 days of ETHUSDT
    $0 ETHUSDT -d 30

    # Download single day
    $0 BTCUSDT --start 2024-01-15 --end 2024-01-15

EOF
}

# Default values
SYMBOL="${1:-BTCUSDT}"
DATA_DIR="data"
DAYS=""
START_DATE=""
END_DATE=""

# Shift past the symbol argument if provided
if [[ "$1" != -* ]] && [[ -n "$1" ]]; then
    shift
fi

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -d|--days)
            DAYS="$2"
            shift 2
            ;;
        -s|--start)
            START_DATE="$2"
            shift 2
            ;;
        -e|--end)
            END_DATE="$2"
            shift 2
            ;;
        -h|--help)
            show_help
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            show_help
            exit 1
            ;;
    esac
done

# Function to convert date to timestamp (for comparison)
date_to_timestamp() {
    if [[ "$OSTYPE" == "darwin"* ]]; then
        date -j -f "%Y-%m-%d" "$1" "+%s" 2>/dev/null || echo 0
    else
        date -d "$1" "+%s" 2>/dev/null || echo 0
    fi
}

# Function to get yesterday's date
get_yesterday() {
    if [[ "$OSTYPE" == "darwin"* ]]; then
        date -v-1d +%Y-%m-%d
    else
        date -d "yesterday" +%Y-%m-%d
    fi
}

# Determine date range
if [[ -n "$START_DATE" ]]; then
    # Date range mode
    if [[ -z "$END_DATE" ]]; then
        END_DATE=$(get_yesterday)
    fi

    START_TS=$(date_to_timestamp "$START_DATE")
    END_TS=$(date_to_timestamp "$END_DATE")

    if [[ $START_TS -eq 0 ]] || [[ $END_TS -eq 0 ]]; then
        echo "Error: Invalid date format. Use YYYY-MM-DD"
        exit 1
    fi

    if [[ $START_TS -gt $END_TS ]]; then
        echo "Error: Start date must be before or equal to end date"
        exit 1
    fi

    MODE="range"
else
    # Days mode (default)
    if [[ -z "$DAYS" ]]; then
        DAYS=7
    fi
    MODE="days"
fi

echo "================================================"
echo "Binance Data Downloader"
echo "================================================"
echo "Symbol: $SYMBOL"

if [[ "$MODE" == "range" ]]; then
    echo "Date range: $START_DATE to $END_DATE"
else
    echo "Last N days: $DAYS"
fi

echo "Data directory: $DATA_DIR"
echo ""

# Create data directory
mkdir -p "$DATA_DIR/$SYMBOL"

# Count for progress
downloaded=0
skipped=0
failed=0

# Generate list of dates to download
if [[ "$MODE" == "range" ]]; then
    # Range mode: iterate from start to end
    current_ts=$START_TS
    dates=()

    while [[ $current_ts -le $END_TS ]]; do
        if [[ "$OSTYPE" == "darwin"* ]]; then
            current_date=$(date -r $current_ts +%Y-%m-%d)
        else
            current_date=$(date -d "@$current_ts" +%Y-%m-%d)
        fi
        dates+=("$current_date")
        current_ts=$((current_ts + 86400))  # Add 1 day in seconds
    done

    total=${#dates[@]}

else
    # Days mode: last N days
    total=$DAYS
    dates=()

    for i in $(seq 1 $DAYS); do
        if [[ "$OSTYPE" == "darwin"* ]]; then
            date_str=$(date -v-${i}d +%Y-%m-%d)
        else
            date_str=$(date -d "${i} days ago" +%Y-%m-%d)
        fi
        dates+=("$date_str")
    done
fi

# Sort dates chronologically (oldest first)
IFS=$'\n' dates=($(sort <<<"${dates[*]}"))
unset IFS

# Download each date
count=0
for DATE in "${dates[@]}"; do
    count=$((count + 1))

    FILENAME="${SYMBOL}-trades-${DATE}"
    URL="https://data.binance.vision/data/spot/daily/trades/${SYMBOL}/${FILENAME}.zip"

    echo "[$count/$total] Processing $DATE..."

    # Check if already exists
    if [ -f "$DATA_DIR/$SYMBOL/trades-${DATE}.csv" ]; then
        echo "  ✓ Already exists, skipping"
        skipped=$((skipped + 1))
        continue
    fi

    # Download with wget or curl
    download_success=0

    if command -v wget &> /dev/null; then
        if wget -q --show-progress -P "$DATA_DIR/$SYMBOL" "$URL" 2>/dev/null; then
            download_success=1
        fi
    elif command -v curl &> /dev/null; then
        if curl -f -L -o "$DATA_DIR/$SYMBOL/${FILENAME}.zip" "$URL" 2>/dev/null; then
            download_success=1
        fi
    else
        echo "Error: Neither wget nor curl found. Please install one of them."
        exit 1
    fi

    if [[ $download_success -eq 0 ]]; then
        echo "  ✗ Failed to download (file may not exist yet)"
        failed=$((failed + 1))
        continue
    fi

    # Unzip
    if [ -f "$DATA_DIR/$SYMBOL/${FILENAME}.zip" ]; then
        if unzip -q -o "$DATA_DIR/$SYMBOL/${FILENAME}.zip" -d "$DATA_DIR/$SYMBOL" 2>/dev/null; then
            mv "$DATA_DIR/$SYMBOL/${FILENAME}.csv" "$DATA_DIR/$SYMBOL/trades-${DATE}.csv" 2>/dev/null || true
            rm "$DATA_DIR/$SYMBOL/${FILENAME}.zip"
            echo "  ✓ Downloaded and extracted"
            downloaded=$((downloaded + 1))
        else
            echo "  ✗ Failed to extract"
            rm -f "$DATA_DIR/$SYMBOL/${FILENAME}.zip"
            failed=$((failed + 1))
        fi
    fi
done

echo ""
echo "================================================"
echo "Download Summary"
echo "================================================"
echo "Total requested: $total"
echo "Downloaded: $downloaded"
echo "Skipped (already exist): $skipped"
echo "Failed: $failed"
echo ""

# List downloaded files in chronological order
if [ -d "$DATA_DIR/$SYMBOL" ]; then
    csv_count=$(ls -1 "$DATA_DIR/$SYMBOL/"*.csv 2>/dev/null | wc -l | xargs)
    echo "Total CSV files: $csv_count"

    if [ $csv_count -gt 0 ]; then
        echo ""
        echo "Available dates:"
        ls -1 "$DATA_DIR/$SYMBOL/trades-"*.csv 2>/dev/null | \
            sed 's/.*trades-/  /' | \
            sed 's/.csv$//' | \
            sort
    fi
fi

echo ""
echo "Usage in C++:"
echo "  DataManager dm(\"$DATA_DIR\");"
echo "  auto trades = dm.load_day(\"$SYMBOL\", \"YYYY-MM-DD\");"
