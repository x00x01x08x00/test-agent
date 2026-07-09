# fanotify-agent-benchmark

A tiny Linux C repository for comparing PR review quality across AI coding agents.

The program is supposed to watch one directory and print file activity such as open,
modify, and close-after-write events.

## Important

This repository is intentionally imperfect.

- It should compile cleanly.
- It contains a small number of planted correctness/resource-management issues.
- Do not tell the reviewers how many issues exist.
- Let every agent review the same commit before applying any fixes.

## Requirements

- Linux with `CONFIG_FANOTIFY`
- C compiler (`gcc` or `clang`)
- `make`
- Root or equivalent capability for this fd-based sample

`fanotify` is Linux-specific.

## Build

```bash
make
```

## Run

```bash
mkdir -p /tmp/fanwatch-demo
sudo ./bin/fanwatch /tmp/fanwatch-demo
```

In another terminal:

```bash
echo hello > /tmp/fanwatch-demo/a.txt
echo world >> /tmp/fanwatch-demo/a.txt
cat /tmp/fanwatch-demo/a.txt
```

Stop the watcher with `Ctrl+C`.

## Smoke test

```bash
make
sudo ./tests/smoke.sh
```

The benchmark starts with the current implementation. Do not fix failures before all
agents have reviewed the PR.

## Suggested benchmark procedure

1. Push this repository.
2. Create a branch named `benchmark/fanotify-status`.
3. Make one harmless edit, such as changing the startup text.
4. Open a PR into `main`.
5. Ask Codex, Gemini, and Claude to review the exact same PR.
6. Save each review before allowing any agent to modify code.
7. Score the reviews with `BENCHMARK.md`.

## Review prompts

Post these as separate PR comments:

```text
@codex review for correctness, Linux fanotify API misuse, resource leaks, undefined behavior, and missing tests. Do not modify code.
```

```text
/gemini review
```

```text
@claude review this PR for correctness, Linux fanotify API misuse, resource leaks, undefined behavior, and missing tests. Do not modify code.
```

After all reviews are saved, choose only one agent to implement fixes at a time.
