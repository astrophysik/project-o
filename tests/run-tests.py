#!/usr/bin/env python3
"""Test runner for the project-o compiler.

Runs all .po test files and validates:
- Tests in 'negative' directories should produce compilation errors (non-empty stderr)
- All other tests should compile successfully (empty stderr)
"""

import os
import subprocess
import sys
from pathlib import Path
from dataclasses import dataclass


@dataclass
class TestResult:
    path: Path
    passed: bool
    expected_error: bool
    had_error: bool
    stderr: str


def find_test_files(tests_dir: Path) -> list[Path]:
    """Find all .po test files in the tests directory."""
    return sorted(tests_dir.rglob("*.po"))


def is_negative_test(file_path: Path) -> bool:
    """Check if a test file is in a 'negative' directory."""
    return "negative" in file_path.parts


def run_compiler(compiler_path: Path, test_file: Path) -> tuple[str, str]:
    """Run the compiler on a test file and return stdout and stderr."""
    result = subprocess.run(
        [str(compiler_path), str(test_file)],
        capture_output=True,
        text=True,
        timeout=30
    )
    return result.stdout, result.stderr


def run_test(compiler_path: Path, test_file: Path) -> TestResult:
    """Run a single test and return the result."""
    expected_error = is_negative_test(test_file)

    try:
        _, stderr = run_compiler(compiler_path, test_file)
        had_error = bool(stderr.strip())
    except subprocess.TimeoutExpired:
        return TestResult(
            path=test_file,
            passed=False,
            expected_error=expected_error,
            had_error=False,
            stderr="TIMEOUT: Compiler took too long"
        )
    except Exception as e:
        return TestResult(
            path=test_file,
            passed=False,
            expected_error=expected_error,
            had_error=False,
            stderr=f"ERROR: {e}"
        )

    passed = (expected_error == had_error)

    return TestResult(
        path=test_file,
        passed=passed,
        expected_error=expected_error,
        had_error=had_error,
        stderr=stderr
    )


def print_result(result: TestResult, verbose: bool = False):
    """Print a single test result."""
    status = "PASS" if result.passed else "FAIL"

    # Color codes
    GREEN = "\033[92m"
    RED = "\033[91m"
    RESET = "\033[0m"
    DIM = "\033[90m"

    color = GREEN if result.passed else RED

    # Relative path from tests directory
    rel_path = result.path
    try:
        rel_path = result.path.relative_to(Path.cwd())
    except ValueError:
        pass

    test_type = "negative" if result.expected_error else "positive"
    print(f"{color}{status}{RESET} [{test_type}] {rel_path}")

    if verbose and result.stderr.strip():
        print(f"{DIM}  stderr: {result.stderr.strip()[:200]}{RESET}")


def run_all_tests(compiler_path: Path, tests_dir: Path) -> tuple[int, int]:
    """Run all tests and return (passed_count, total_count)."""
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
            print_result(result, verbose=True)
        else:
            failed_results.append(result)
            print_result(result, verbose=True)

    # Print summary
    print("-" * 60)
    total = len(test_files)

    GREEN = "\033[92m"
    RED = "\033[91m"
    RESET = "\033[0m"

    if passed_count == total:
        print(f"{GREEN}All {total} tests passed!{RESET}")
    else:
        print(f"{RED}{passed_count}/{total} tests passed, {total - passed_count} failed{RESET}")

        if failed_results:
            print(f"\n{RED}Failed tests:{RESET}")
            for r in failed_results:
                rel_path = r.path
                try:
                    rel_path = r.path.relative_to(Path.cwd())
                except ValueError:
                    pass

                if r.expected_error and not r.had_error:
                    print(f"  {rel_path} - Expected error but compilation succeeded")
                elif not r.expected_error and r.had_error:
                    print(f"  {rel_path} - Unexpected compilation error")

    return passed_count, total


def main():
    if len(sys.argv) < 2:
        print("Usage: python run_tests.py <compiler_path> [tests_dir]")
        print("Example: python run_tests.py ./build/compiler")
        print("         python run_tests.py ./build/compiler ./tests")
        sys.exit(1)

    compiler_path = Path(sys.argv[1])
    tests_dir = Path(sys.argv[2]) if len(sys.argv) > 2 else Path(__file__).parent

    # Check if compiler exists
    if not compiler_path.exists():
        print(f"Error: Compiler not found at {compiler_path}")
        sys.exit(1)

    # Check if tests directory exists
    if not tests_dir.exists():
        print(f"Error: Tests directory not found at {tests_dir}")
        sys.exit(1)

    passed, total = run_all_tests(compiler_path, tests_dir)

    sys.exit(0 if passed == total else 1)


if __name__ == "__main__":
    main()
