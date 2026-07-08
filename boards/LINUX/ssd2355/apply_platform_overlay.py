#!/usr/bin/env python3
# coding=utf-8
"""
Apply board-specific patches to the downloaded LINUX platform tree.

TuyaOpen downloads platform/LINUX from the upstream TuyaOpen-ubuntu repository.
SSD2355 support lives in boards/LINUX/ssd2355/platform_overlay/ and is copied
into platform/LINUX before platform_prepare.py runs.
"""

import os
import shutil
import sys


def apply(open_root, platform="LINUX", chip="ssd2355"):
    """
    Copy overlay files into platform/LINUX.

    @param[in] open_root TuyaOpen repository root
    @param[in] platform platform name (LINUX)
    @param[in] chip board chip name (ssd2355)
    @return True on success
    """
    overlay_dir = os.path.join(open_root, "boards", platform, chip, "platform_overlay")
    platform_dir = os.path.join(open_root, "platform", platform)

    if not os.path.isdir(overlay_dir):
        return True

    if not os.path.isdir(platform_dir):
        print(f"Error: platform directory not found: {platform_dir}")
        print("Run tos.py build once to download the LINUX platform.")
        return False

    copied = 0
    for root, _dirs, files in os.walk(overlay_dir):
        for name in files:
            src = os.path.join(root, name)
            rel = os.path.relpath(src, overlay_dir)
            dst = os.path.join(platform_dir, rel)
            os.makedirs(os.path.dirname(dst), exist_ok=True)
            shutil.copy2(src, dst)
            copied += 1

    print(f"SSD2355: applied {copied} file(s) to {platform_dir}")
    return True


def main():
    if len(sys.argv) >= 2:
        open_root = os.path.abspath(sys.argv[1])
    else:
        open_root = os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))

    platform = sys.argv[2] if len(sys.argv) >= 3 else "LINUX"
    chip = sys.argv[3] if len(sys.argv) >= 4 else "ssd2355"

    if not apply(open_root, platform, chip):
        sys.exit(1)
    sys.exit(0)


if __name__ == "__main__":
    main()
