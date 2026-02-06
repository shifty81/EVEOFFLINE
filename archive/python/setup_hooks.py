#!/usr/bin/env python3
"""
Setup Git hooks for EVE OFFLINE
Installs pre-commit and pre-push hooks for automated checks
"""

import os
import sys
import stat
from pathlib import Path

HOOK_DIR = Path(".git/hooks")

PRE_COMMIT_HOOK = """#!/usr/bin/env python3
# EVE OFFLINE - Pre-commit hook
# Runs quick checks before allowing commit

import sys
import subprocess

print("Running pre-commit checks...")
result = subprocess.run([sys.executable, "build_and_test.py", "--quick"], 
                       capture_output=False)

if result.returncode != 0:
    print("\\n❌ Pre-commit checks failed!")
    print("Fix the issues and try again, or use 'git commit --no-verify' to skip")
    sys.exit(1)

print("\\n✅ Pre-commit checks passed!")
sys.exit(0)
"""

PRE_PUSH_HOOK = """#!/usr/bin/env python3
# EVE OFFLINE - Pre-push hook
# Runs full tests before allowing push

import sys
import subprocess

print("Running pre-push checks...")
result = subprocess.run([sys.executable, "build_and_test.py"], 
                       capture_output=False)

if result.returncode != 0:
    print("\\n❌ Pre-push checks failed!")
    print("Fix the issues and try again, or use 'git push --no-verify' to skip")
    sys.exit(1)

print("\\n✅ Pre-push checks passed!")
sys.exit(0)
"""


def install_hook(hook_name: str, hook_content: str) -> bool:
    """Install a Git hook"""
    if not HOOK_DIR.exists():
        print(f"Error: {HOOK_DIR} not found. Are you in a Git repository?")
        return False
    
    hook_path = HOOK_DIR / hook_name
    
    # Backup existing hook
    if hook_path.exists():
        backup_path = hook_path.with_suffix('.backup')
        print(f"Backing up existing {hook_name} to {backup_path}")
        hook_path.rename(backup_path)
    
    # Write new hook
    print(f"Installing {hook_name}...")
    hook_path.write_text(hook_content)
    
    # Make executable
    st = os.stat(hook_path)
    os.chmod(hook_path, st.st_mode | stat.S_IEXEC)
    
    print(f"✓ {hook_name} installed")
    return True


def main():
    """Main entry point"""
    print("=" * 60)
    print("EVE OFFLINE - Git Hook Setup")
    print("=" * 60)
    print()
    
    success = True
    
    # Install pre-commit hook
    if install_hook("pre-commit", PRE_COMMIT_HOOK):
        print()
    else:
        success = False
    
    # Install pre-push hook
    if install_hook("pre-push", PRE_PUSH_HOOK):
        print()
    else:
        success = False
    
    if success:
        print("=" * 60)
        print("✅ Git hooks installed successfully!")
        print("=" * 60)
        print()
        print("Hooks installed:")
        print("  • pre-commit: Runs quick checks before commit")
        print("  • pre-push: Runs full tests before push")
        print()
        print("To skip hooks, use:")
        print("  git commit --no-verify")
        print("  git push --no-verify")
        return 0
    else:
        print("=" * 60)
        print("❌ Failed to install some hooks")
        print("=" * 60)
        return 1


if __name__ == "__main__":
    sys.exit(main())
