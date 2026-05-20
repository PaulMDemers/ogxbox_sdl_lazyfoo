#!/usr/bin/env sh
set -eu

root_dir="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
out_dir="${1:-$root_dir/dist/release/lazyfoo}"
iso_count=0
xbe_count=0

rm -rf "$out_dir"
mkdir -p "$out_dir/isos" "$out_dir/xbes"

for app in $(make -s -C "$root_dir" print-apps); do
    iso="$(ls -t "$root_dir/$app"/*.iso 2>/dev/null | sed -n '1p')"
    xbe="$root_dir/$app/bin/default.xbe"
    if [ -n "$iso" ]; then
        cp "$iso" "$out_dir/isos/"
        iso_count=$((iso_count + 1))
    fi
    if [ -f "$xbe" ]; then
        mkdir -p "$out_dir/xbes/$app"
        cp "$xbe" "$out_dir/xbes/$app/default.xbe"
        xbe_count=$((xbe_count + 1))
    fi
done

printf 'Copied %d ISO(s) and %d XBE(s) to %s\n' "$iso_count" "$xbe_count" "$out_dir"
