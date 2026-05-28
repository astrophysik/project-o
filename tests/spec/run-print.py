#!/usr/bin/env python3
"""Test runner for the project-o compiler.

Runs all .po test files (excluding 'negative' directories) and validates:
- Test must compile successfully (empty stderr)
- Output of the compiled program must not contain 'false'
"""

import subprocess
import sys
import tempfile
import os
from pathlib import Path
from dataclasses import dataclass

GREEN = "\033[92m"
RED   = "\033[91m"
DIM   = "\033[90m"
RESET = "\033[0m"


@dataclass
class TestResult:
    path: Path
    passed: bool
    fail_reason: str  # empty string if passed
    stderr: str
    run_output: str


def find_test_files(tests_dir: Path) -> list[Path]:
    return sorted(
        f for f in tests_dir.rglob("*.po")
        if "negative" not in f.parts
    )


def run_test(compiler_path: Path, test_file: Path) -> TestResult:
    # --- compile ---
    with tempfile.TemporaryDirectory() as tmp:
        out_bin = Path(tmp) / "a.out"
        try:
            comp = subprocess.run(
                [str(compiler_path), str(test_file), "-o", str(out_bin)],
                capture_output=True, text=True, timeout=30
            )
        except subprocess.TimeoutExpired:
            return TestResult(test_file, False, "TIMEOUT during compilation", "", "")
        except Exception as e:
            return TestResult(test_file, False, f"ERROR: {e}", "", "")

        if comp.stderr.strip() or comp.returncode != 0:
            return TestResult(test_file, False, "Compilation failed", comp.stderr, "")

        # --- run ---
        if not out_bin.exists():
            # Some compilers write output next to source; try stem with no ext
            out_bin = test_file.with_suffix("")

        try:
            run = subprocess.run(
                [str(out_bin)],
                capture_output=True, text=True, timeout=30
            )
        except subprocess.TimeoutExpired:
            return TestResult(test_file, False, "TIMEOUT during execution", "", "")
        except Exception as e:
            return TestResult(test_file, False, f"ERROR running binary: {e}", "", run.stdout if 'run' in dir() else "")

        output = run.stdout + run.stderr
        if "false" in output:
            return TestResult(test_file, False, "Output contains 'false'", comp.stderr, output)

        return TestResult(test_file, True, "", comp.stderr, output)


def print_result(result: TestResult, verbose: bool = False) -> None:
    status = "PASS" if result.passed else "FAIL"
    color  = GREEN if result.passed else RED
    try:
        rel_path = result.path.relative_to(Path.cwd())
    except ValueError:
        rel_path = result.path

    print(f"{color}{status}{RESET} {rel_path}", end="")
    if not result.passed:
        print(f"  — {result.fail_reason}", end="")
    print()

    if verbose:
        if result.stderr.strip():
            print(f"{DIM}  stderr: {result.stderr.strip()[:200]}{RESET}")
        if result.run_output.strip():
            print(f"{DIM}  output: {result.run_output.strip()[:200]}{RESET}")


def run_all_tests(compiler_path: Path, tests_dir: Path) -> tuple[int, int]:
    test_files = find_test_files(tests_dir)

    if not test_files:
        print(f"No test files found in {tests_dir}")
        return 0, 0

    print(f"Found {len(test_files)} test files")
    print(f"Compiler: {compiler_path}")
    print("-" * 60)

    passed_count = 0
    failed_results: list[TestResult] = []

    for test_file in test_files:
        result = run_test(compiler_path, test_file)
        if result.passed:
            passed_count += 1
        else:
            failed_results.append(result)
        print_result(result, verbose=True)

    print("-" * 60)
    total = len(test_files)

    if passed_count == total:
        print(f"{GREEN}All {total} tests passed!{RESET}")
    else:
        print(f"{RED}{passed_count}/{total} tests passed, {total - passed_count} failed{RESET}")

    if failed_results:
        print(f"\n{RED}Failed tests:{RESET}")
        for r in failed_results:
            try:
                rel_path = r.path.relative_to(Path.cwd())
            except ValueError:
                rel_path = r.path
            print(f"  {rel_path} — {r.fail_reason}")

    return passed_count, total


def main() -> None:
    if len(sys.argv) < 2:
        print("Usage: python run_tests.py <compiler> [tests_dir]")
        print("Example: python run_tests.py ./build/compiler")
        print("         python run_tests.py ./build/compiler ./tests")
        sys.exit(1)

    compiler_path = Path(sys.argv[1])
    tests_dir     = Path(sys.argv[2]) if len(sys.argv) > 2 else Path(__file__).parent

    if not compiler_path.exists():
        print(f"Error: Compiler not found at {compiler_path}")
        sys.exit(1)

    if not tests_dir.exists():
        print(f"Error: Tests directory not found at {tests_dir}")
        sys.exit(1)

    passed, total = run_all_tests(compiler_path, tests_dir)
    sys.exit(0 if passed == total else 1)


if __name__ == "__main__":
    main()
