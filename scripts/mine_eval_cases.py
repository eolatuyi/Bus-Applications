#!/usr/bin/env python3
"""
mine_eval_cases.py — deterministic backend for the eval-case-miner skill.

Handles all the mechanical parts of mining eval cases from a repo's git
history, so the agent invoking this skill only has to do the judgment
parts: reading candidate summaries, drafting expected-findings.md prose,
and relaying the owner's accept/reject decisions.

Requires: PyYAML (pip install pyyaml --break-system-packages)

Three subcommands, matching the skill's Procedure steps:

  scan        Search history for new candidate commits, print a summary.
              Writes nothing to eval-cases/ — read-only.

  accept      Extract before/after for one accepted commit, scaffold
              metadata.yaml (with a TODO placeholder findings prompt) and
              a stub expected-findings.md marked as a mined candidate.
              Does NOT advance the mining checkpoint.

  checkpoint  Advance .mining-state.yaml's last_mined_commit once the
              owner has finished reviewing a full batch (accepted +
              rejected + skipped) — so the next scan doesn't re-surface
              anything already seen.

Usage:

  python mine_eval_cases.py scan --repo /path/to/repo --system bus-applications
  python mine_eval_cases.py scan --repo /path/to/repo --system bus-applications --since v2.1
  python mine_eval_cases.py scan --repo /path/to/repo --system bus-applications --category concurrency

  python mine_eval_cases.py accept --repo /path/to/repo --system bus-applications \\
      --commit 4efc865eef7964e56fcc6e090f56e83d930eb8ea \\
      --case-id 0001-lcd1602-gpiod-v2-line-offset \\
      --file src/lcd1602.cpp \\
      --category shared_resource --target-pass pass2_fault_posture

  python mine_eval_cases.py checkpoint --repo /path/to/repo --system bus-applications \\
      --commit eb01b58a7ff8a56d2ad4593b8625b7ab28c3df75
"""

import argparse
import subprocess
import sys
from datetime import date, datetime
from pathlib import Path

import yaml

DEFAULT_GREP_TERMS = [
    "race", "deadlock", "reentrant", "ISR", "interrupt",
    "priority inversion", "use-after-free", "stack overflow",
    "memory leak", "double free", "off-by-one", "overflow",
    "underflow", "watchdog", "brown-?out", "low.power", "sleep",
    "wake", "extern", "linkage", "gpiod", "offset",
]

CATEGORY_TERMS = {
    "concurrency": ["race", "deadlock", "reentrant", "ISR", "interrupt",
                    "priority inversion", "critical section", "atomic"],
    "shared_resource": ["gpiod", "offset", "bus", "i2c", "spi", "pool",
                         "exhaustion", "arbitration"],
    "cleanup": ["leak", "double free", "use-after-free", "teardown",
                "deinit", "cleanup"],
    "low_power": ["sleep", "wake", "low.power", "tickless", "clock gat"],
    "fault_tolerance": ["watchdog", "brown-?out", "redundan", "voting",
                         "safe-state", "escalat"],
    "state_variable": ["state machine", "transition", "illegal state"],
}


def run_git(repo, *args):
    result = subprocess.run(
        ["git", "-C", str(repo), *args],
        capture_output=True, text=True, check=True,
    )
    return result.stdout


def state_path(eval_cases_dir, system):
    return eval_cases_dir / system / ".mining-state.yaml"


def load_state(eval_cases_dir, system):
    p = state_path(eval_cases_dir, system)
    if not p.exists():
        return None
    with open(p) as f:
        return yaml.safe_load(f)


def existing_commit_hashes(eval_cases_dir, system):
    """Dedup set: every commit_hash already present in a metadata.yaml
    under eval-cases/<system>/*/metadata.yaml."""
    hashes = set()
    system_dir = eval_cases_dir / system
    if not system_dir.exists():
        return hashes
    for meta_path in system_dir.glob("*/metadata.yaml"):
        with open(meta_path) as f:
            meta = yaml.safe_load(f)
        if meta and meta.get("commit_hash"):
            hashes.add(meta["commit_hash"])
    return hashes


def guess_category(subject, body):
    text = (subject + " " + body).lower()
    scores = {}
    for cat, terms in CATEGORY_TERMS.items():
        score = sum(1 for t in terms if t.lower() in text)
        if score:
            scores[cat] = score
    if not scores:
        return "general"
    return max(scores, key=scores.get)


def cmd_scan(args):
    repo = Path(args.repo)
    eval_cases_dir = Path(args.eval_cases_dir)
    state = load_state(eval_cases_dir, args.system)

    since = args.since or (state["last_mined_commit"] if state else None)
    terms = CATEGORY_TERMS.get(args.category, DEFAULT_GREP_TERMS) if args.category else DEFAULT_GREP_TERMS
    grep_pattern = r"\|".join(terms)

    log_args = ["log", "--all", "--pretty=format:%H%x1f%s%x1f%b%x1e",
                "-i", f"--grep={grep_pattern}"]
    if since:
        log_args.insert(2, f"{since}..HEAD")

    try:
        raw = run_git(repo, *log_args)
    except subprocess.CalledProcessError as e:
        print(f"git log failed: {e.stderr}", file=sys.stderr)
        sys.exit(1)

    known_hashes = existing_commit_hashes(eval_cases_dir, args.system)

    candidates = []
    for entry in raw.split("\x1e"):
        entry = entry.strip()
        if not entry:
            continue
        parts = entry.split("\x1f")
        if len(parts) < 2:
            continue
        commit_hash, subject = parts[0], parts[1]
        body = parts[2] if len(parts) > 2 else ""
        if commit_hash in known_hashes:
            continue
        candidates.append({
            "commit_hash": commit_hash,
            "subject": subject.strip(),
            "guessed_category": guess_category(subject, body),
        })

    if not candidates:
        print(f"No new candidates for '{args.system}' since "
              f"{since or 'the beginning of history'}.")
        return

    print(f"# {len(candidates)} new candidate(s) for '{args.system}'")
    print(f"# (scanned since: {since or 'beginning of history'})\n")
    for c in candidates:
        print(f"- commit: {c['commit_hash']}")
        print(f"  subject: {c['subject']}")
        print(f"  guessed_category: {c['guessed_category']}")
        print()
    print("# Next: for each you want to keep, run `accept` with --commit,")
    print("# --case-id, --file, and the real category/target_pass once")
    print("# you've looked at the diff. Then run `checkpoint` once you've")
    print("# finished reviewing this whole batch (accepted + rejected).")


def cmd_accept(args):
    repo = Path(args.repo)
    eval_cases_dir = Path(args.eval_cases_dir)
    case_dir = eval_cases_dir / args.system / args.case_id

    if case_dir.exists():
        print(f"Case directory already exists: {case_dir}", file=sys.stderr)
        sys.exit(1)

    case_dir.mkdir(parents=True)

    try:
        before = run_git(repo, "show", f"{args.commit}^:{args.file}")
    except subprocess.CalledProcessError:
        before = None  # file may not have existed pre-commit
    after = run_git(repo, "show", f"{args.commit}:{args.file}")

    ext = Path(args.file).suffix or ".txt"
    if before is not None:
        (case_dir / f"before{ext}").write_text(before)
    (case_dir / f"after{ext}").write_text(after)

    metadata = {
        "case_id": args.case_id,
        "system": args.system,
        "source": "real",
        "commit_hash": args.commit,
        "bug_category": args.category or "TODO",
        "target_pass": args.target_pass or "TODO",
        "criticality": args.criticality or "medium",
        "expected_verdict_before": "TODO — fill in after reviewing the diff",
        "expected_verdict_after": "TODO — fill in after reviewing the diff",
        "notes": f"Mined {date.today().isoformat()}. Source file: {args.file}.",
    }
    with open(case_dir / "metadata.yaml", "w") as f:
        yaml.safe_dump(metadata, f, sort_keys=False, allow_unicode=True)

    findings_stub = f"""# Expected Findings — {args.case_id}

## ⚠️ MINED CANDIDATE — NOT YET VALIDATED
This was auto-drafted from commit {args.commit}. Verify the described
defect and consequence match reality before treating this as ground truth.

Mined from `{args.system}` commit `{args.commit}`.
Source file: `{args.file}`.

## Defect

TODO — describe the defect based on the diff.

## Consequence

TODO — describe what actually happens as a result.

## Findings the review must produce on `before{ext}`

TODO

## Findings the review must NOT produce on `after{ext}`

TODO
"""
    (case_dir / "expected-findings.md").write_text(findings_stub)

    print(f"Scaffolded case at {case_dir}")
    print(f"  - before{ext} {'(written)' if before is not None else '(SKIPPED — file did not exist pre-commit)'}")
    print(f"  - after{ext} (written)")
    print("  - metadata.yaml (TODO fields need filling)")
    print("  - expected-findings.md (MINED CANDIDATE stub — needs real content + validation)")
    print("\nCheckpoint NOT advanced. Run `checkpoint` once you've finished")
    print("reviewing the full batch this commit came from.")


def cmd_checkpoint(args):
    eval_cases_dir = Path(args.eval_cases_dir)
    system_dir = eval_cases_dir / args.system
    system_dir.mkdir(parents=True, exist_ok=True)
    p = state_path(eval_cases_dir, args.system)

    state = load_state(eval_cases_dir, args.system) or {}
    state["system"] = args.system
    state["last_mined_commit"] = args.commit
    state["last_mined_date"] = date.today().isoformat()
    if args.source_repo:
        state["source_repo"] = args.source_repo
    if args.notes:
        state["notes"] = args.notes

    with open(p, "w") as f:
        yaml.safe_dump(state, f, sort_keys=False, allow_unicode=True)

    print(f"Checkpoint updated: {p}")
    print(f"  last_mined_commit: {args.commit}")


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                      formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = parser.add_subparsers(dest="cmd", required=True)

    common = dict(
        repo=("--repo", {"required": True, "help": "Path to the repo to mine"}),
        system=("--system", {"required": True, "help": "System name, e.g. bus-applications"}),
        eval_cases_dir=("--eval-cases-dir", {"default": "eval-cases", "help": "Path to eval-cases/ root"}),
    )

    p_scan = sub.add_parser("scan", help="Search history for new candidates (read-only)")
    p_scan.add_argument(common["repo"][0], **common["repo"][1])
    p_scan.add_argument(common["system"][0], **common["system"][1])
    p_scan.add_argument(common["eval_cases_dir"][0], **common["eval_cases_dir"][1])
    p_scan.add_argument("--since", help="Commit/tag to scan since (default: last checkpoint, or full history)")
    p_scan.add_argument("--category", choices=list(CATEGORY_TERMS), help="Narrow the grep terms to one category")
    p_scan.set_defaults(func=cmd_scan)

    p_accept = sub.add_parser("accept", help="Extract before/after and scaffold a case for one accepted commit")
    p_accept.add_argument(common["repo"][0], **common["repo"][1])
    p_accept.add_argument(common["system"][0], **common["system"][1])
    p_accept.add_argument(common["eval_cases_dir"][0], **common["eval_cases_dir"][1])
    p_accept.add_argument("--commit", required=True)
    p_accept.add_argument("--case-id", required=True)
    p_accept.add_argument("--file", required=True, help="Path (within repo) to isolate before/after for")
    p_accept.add_argument("--category", choices=list(CATEGORY_TERMS) + ["general"])
    p_accept.add_argument("--target-pass", choices=["pass1_intent", "pass2_fault_posture",
                                                      "pass3_fit_placement", "pass4_regression_coverage"])
    p_accept.add_argument("--criticality", choices=["low", "medium", "high"])
    p_accept.set_defaults(func=cmd_accept)

    p_checkpoint = sub.add_parser("checkpoint", help="Advance the mining checkpoint after reviewing a batch")
    p_checkpoint.add_argument(common["repo"][0], **common["repo"][1])
    p_checkpoint.add_argument(common["system"][0], **common["system"][1])
    p_checkpoint.add_argument(common["eval_cases_dir"][0], **common["eval_cases_dir"][1])
    p_checkpoint.add_argument("--commit", required=True)
    p_checkpoint.add_argument("--source-repo", help="Repo name to record in the state file")
    p_checkpoint.add_argument("--notes", help="Free-text notes about this mining batch")
    p_checkpoint.set_defaults(func=cmd_checkpoint)

    args = parser.parse_args()
    args.func(args)


if __name__ == "__main__":
    main()
