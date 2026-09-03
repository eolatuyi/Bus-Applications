#!/usr/bin/env bash
# Hardware-in-the-loop checks for the multi-bus dashboard (run on the Pi).
set -euo pipefail

APP="${APP:-./build/app}"
I2C_BUS="${I2C_BUS:-1}"
RUN_SECS="${RUN_SECS:-8}"

pass() { echo "PASS: $*"; }
fail() { echo "FAIL: $*" >&2; exit 1; }
warn() { echo "WARN: $*"; }

echo "=== HIL preflight ==="
command -v i2cdetect >/dev/null || fail "i2cdetect not found (install i2c-tools)"
[[ -e "/dev/i2c-${I2C_BUS}" ]] || fail "/dev/i2c-${I2C_BUS} missing — enable I2C in raspi-config"
[[ -e /dev/spidev0.0 ]] || fail "/dev/spidev0.0 missing — enable SPI in raspi-config"
[[ -x "$APP" ]] || fail "app not found at $APP (build first)"

echo "=== I2C device scan (expect 0x48 family ADC at 0x4b, IMU at 0x68) ==="
mapfile -t lines < <(i2cdetect -y "$I2C_BUS" 2>/dev/null | tail -n +2)
scan=$(printf '%s\n' "${lines[@]}")
echo "$scan"
echo "$scan" | grep -qE '\b4b\b' || fail "ADS7830 not seen at 0x4b"
echo "$scan" | grep -qE '\b68\b' || fail "MPU6050 not seen at 0x68"
pass "I2C addresses 0x4b and 0x68 present"

echo "=== HC595 SPI bring-up smoke ==="
spi_log=$(mktemp)
timeout 3 "$APP" --test-hc595 >"$spi_log" 2>&1 || [[ $? -eq 124 ]] || fail "--test-hc595 exited early"
grep -q "walk Q0" "$spi_log" || fail "no walk Q0 in --test-hc595 output"
pass "--test-hc595 produced walk pattern (visual check is still operator)"
rm -f "$spi_log"

echo "=== App smoke (MPU6050 + ADS7830 + SPI, no LCD) ==="
log=$(mktemp)
trap 'rm -f "$log"' EXIT
timeout "$RUN_SECS" "$APP" --no-lcd >"$log" 2>&1 || [[ $? -eq 124 ]] || fail "app exited early — see $log"
grep -q "ADS7830 @0x4b pot=CH2" "$log" || fail "startup banner missing (wrong build?)"
grep -q "Accel\[g\]=" "$log" || fail "no MPU6050 samples in log"
grep -q "Pot=" "$log" || fail "no ADS7830 samples in log"
pass "app ran ${RUN_SECS}s without fatal error"

echo "=== Operator checks (manual) ==="
echo "  0. LED bar first: $APP --test-hc595  (walk Q0-Q7, then bar fill; Ctrl-C)."
echo "  1. Turn the pot fully CCW then CW — Pot= should span roughly 0–255."
echo "  2. LED bar on 74HC595 should track Pot= monotonically."
echo "  3. Tilt the MPU6050 — Accel[g] axes should change smoothly."
echo "  4. Wire LCD per README, run: $APP (no --no-lcd)"
echo "     - Expect 'I2C/SPI Dashboard' then live Pot/ax lines."
echo "     - Adjust VO contrast if blank; run WITHOUT sudo if in gpio group."

if [[ "${STRICT_POT:-0}" == "1" ]]; then
  min=$(grep -o 'Pot=[0-9]*' "$log" | sed 's/Pot=//' | sort -n | head -1)
  max=$(grep -o 'Pot=[0-9]*' "$log" | sed 's/Pot=//' | sort -n | tail -1)
  if [[ "$min" == "$max" ]]; then
    fail "pot unchanged during run — turn knob during test or set STRICT_POT=0"
  fi
  pass "pot range ${min}..${max} observed during capture"
else
  warn "pot sweep not auto-verified — re-run with STRICT_POT=1 while turning the knob"
fi

echo "=== HIL complete ==="
