# Expected Findings — 0001-lcd1602-gpiod-v2-line-offset

Validated 2026-09-08 by owner.

Mined from `bus-applications` commit `4efc865eef7964e56fcc6e090f56e83d930eb8ea`.
Source file: `src/lcd1602.cpp`.

## Defect

Each LCD GPIO line is requested from libgpiod v2 as a single-line request whose
offset is the chip line number (BCM 17/27/22/23/24/25 by default). Every later
write then calls `gpiod_line_request_set_value` with offset `0` instead of that
chip line number:

- `pulseEnable` drives `e_` at offset `0`
- `write4` drives `rs_` / `d4_` / `d5_` / `d6_` / `d7_` at offset `0`

In libgpiod v2 the offset argument is the GPIO chip line number used at request
time, not an index into the request. Offset `0` is not in any of these
requests. The return value of `set_value` is discarded, so the failed writes
are silent.

## Consequence

RS, E, and the four data pins never change. The HD44780 never sees a nibble or
an enable pulse, so init, clear, and character writes all no-op. The LCD stays
blank. Because `set_value` failures are ignored, the driver reports success.

## Findings the review must produce on `before.cpp`

Pass 2 (fault posture) / shared-resource — **blocking** (or equivalent
severity under this project's non-critical defaults, still required as a
finding):

1. `gpiod_line_request_set_value(..., 0, ...)` in `pulseEnable` and `write4`
   uses the wrong offset for a request created with `offsets[1] = {gpio}`
   where `gpio` is the chip line number. The review must name the call sites
   and state that the offset must be the requested chip line, not `0`.
2. Failed GPIO writes are undetected: `set_value`'s return code is ignored, so
   the driver cannot tell that every RS/E/data write is rejected.

A Pass 1 (intent) finding that the LCD path cannot display anything is
acceptable as a companion, but it is not a substitute for (1).

The review is **not** required to flag the HD44780 init-timing / `pulseEnable`
pulse-width differences that landed in the same commit. Those are bring-up
margins, not this defect.

## Findings the review must NOT produce on `after.cpp`

Must not re-file the offset-`0` GPIO write bug: `setLine` passes the stored
chip line (`rs_off_`, `e_off_`, `d4_off_`, …) into
`gpiod_line_request_set_value`, matching the offset used in `request_out`.

Must not re-file ignored `set_value` failures: `setLine` checks `rc < 0` and
throws.

May still note unrelated follow-ups (e.g. `delay_us` implemented as
`sleep_for`, no HIL for the LCD path). Those are out of scope for this case's
required findings.
