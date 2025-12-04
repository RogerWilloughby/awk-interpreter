#!/usr/bin/env python3
"""
Integration test runner for AWK Interpreter
Provides cross-platform test execution with detailed reporting
"""

import os
import sys
import subprocess
import glob
import argparse
from pathlib import Path
from typing import List, Tuple


class Colors:
    """ANSI color codes for terminal output"""
    GREEN = '\033[92m'
    RED = '\033[91m'
    YELLOW = '\033[93m'
    BLUE = '\033[94m'
    RESET = '\033[0m'
    BOLD = '\033[1m'

    @classmethod
    def disable(cls):
        cls.GREEN = cls.RED = cls.YELLOW = cls.BLUE = cls.RESET = cls.BOLD = ''


def find_awk_executable() -> str:
    """Find the AWK executable in common build locations"""
    project_dir = Path(__file__).parent.parent

    candidates = [
        project_dir / 'build' / 'bin' / 'awk',
        project_dir / 'build' / 'bin' / 'awk.exe',
        project_dir / 'build' / 'bin' / 'Release' / 'awk.exe',
        project_dir / 'build' / 'bin' / 'Debug' / 'awk.exe',
        project_dir / 'build' / 'Release' / 'awk.exe',
        project_dir / 'build' / 'Debug' / 'awk.exe',
    ]

    for candidate in candidates:
        if candidate.exists():
            return str(candidate)

    raise FileNotFoundError("AWK executable not found. Please build the project first.")


def run_test(awk_exe: str, script: Path, input_file: Path, expected: Path) -> Tuple[bool, str, str]:
    """Run a single test and return (passed, actual_output, expected_output)"""
    try:
        cmd = [awk_exe, '-f', str(script)]
        if input_file.exists():
            cmd.append(str(input_file))

        result = subprocess.run(
            cmd,
            capture_output=True,
            text=True,
            timeout=30
        )

        actual = result.stdout

        if expected.exists():
            with open(expected, 'r') as f:
                expected_output = f.read()
        else:
            expected_output = ""

        # Normalize line endings
        actual = actual.replace('\r\n', '\n')
        expected_output = expected_output.replace('\r\n', '\n')

        passed = actual == expected_output
        return passed, actual, expected_output

    except subprocess.TimeoutExpired:
        return False, "TIMEOUT", ""
    except Exception as e:
        return False, f"ERROR: {e}", ""


def main():
    parser = argparse.ArgumentParser(description='Run AWK integration tests')
    parser.add_argument('--awk', help='Path to AWK executable')
    parser.add_argument('--verbose', '-v', action='store_true', help='Show detailed output')
    parser.add_argument('--no-color', action='store_true', help='Disable colored output')
    parser.add_argument('--filter', help='Only run tests matching this pattern')
    args = parser.parse_args()

    if args.no_color or not sys.stdout.isatty():
        Colors.disable()

    # Find paths
    project_dir = Path(__file__).parent.parent
    test_dir = project_dir / 'integration_tests'
    scripts_dir = test_dir / 'scripts'
    input_dir = test_dir / 'input'
    expected_dir = test_dir / 'expected'

    # Find AWK executable
    try:
        awk_exe = args.awk if args.awk else find_awk_executable()
    except FileNotFoundError as e:
        print(f"{Colors.RED}Error: {e}{Colors.RESET}")
        return 1

    print(f"{Colors.BOLD}AWK Integration Tests{Colors.RESET}")
    print(f"Executable: {awk_exe}")
    print("=" * 60)

    # Find test scripts
    scripts = sorted(scripts_dir.glob('*.awk'))

    if args.filter:
        scripts = [s for s in scripts if args.filter in s.name]

    if not scripts:
        print(f"{Colors.YELLOW}No test scripts found{Colors.RESET}")
        return 1

    # Run tests
    passed = 0
    failed = 0
    results = []

    for script in scripts:
        test_name = script.stem

        # Find corresponding input and expected files
        input_file = input_dir / f"{test_name}.txt"
        if not input_file.exists():
            # Try generic input files
            for generic in ['data.txt', 'numbers.txt', 'input.txt']:
                candidate = input_dir / generic
                if candidate.exists():
                    input_file = candidate
                    break

        expected_file = expected_dir / f"{test_name}.txt"

        # Run test
        success, actual, expected_output = run_test(awk_exe, script, input_file, expected_file)

        if success:
            passed += 1
            status = f"{Colors.GREEN}PASS{Colors.RESET}"
        else:
            failed += 1
            status = f"{Colors.RED}FAIL{Colors.RESET}"

        print(f"[{status}] {test_name}")

        if args.verbose and not success:
            print(f"  Expected: {repr(expected_output[:100])}")
            print(f"  Actual:   {repr(actual[:100])}")

        results.append((test_name, success, actual, expected_output))

    # Summary
    print("=" * 60)
    total = passed + failed
    print(f"Results: {Colors.GREEN}{passed} passed{Colors.RESET}, ", end='')
    print(f"{Colors.RED}{failed} failed{Colors.RESET} ({total} total)")

    if failed > 0:
        print(f"\n{Colors.RED}Failed tests:{Colors.RESET}")
        for name, success, _, _ in results:
            if not success:
                print(f"  - {name}")

    return 0 if failed == 0 else 1


if __name__ == '__main__':
    sys.exit(main())
