#!/usr/bin/env python3
 
import os
import sys
import subprocess
import shutil

cfg_default_output_dir = 'build'

def remove_dir(tgt_dir):
    """Remove directory `tgt_dir` and all its contents recursively.
    
    Args:
        tgt_dir (str): Path to the directory to be removed.
        
    Raises:
        OSError: If the directory cannot be removed (e.g., permission issues).
    """
    if os.path.exists(tgt_dir):
        shutil.rmtree(tgt_dir)

def fetch_all_branches():
    subprocess.run(["git", "fetch", "--all"])
    
    branches = subprocess.check_output(["git", "branch", "-r"]).decode().split('\n')
    for branch in branches:
        branch = branch.strip()
        if branch and not branch.endswith('->') and branch.startswith('origin/'):
            local_branch = branch.replace('origin/', '')
            subprocess.run(["git", "branch", "--track", local_branch, branch])
    
    print("git fetch all branches done")

def run(args):

    fetch_all_branches()

    cpu = os.cpu_count()

    base_dir = os.path.abspath(os.path.dirname(__file__))
    output_dir = os.path.join(base_dir, cfg_default_output_dir, 'html')

    remove_dir(output_dir)

    print("Start build English version")
    src = os.path.join(base_dir, 'en')
    dst = os.path.join(output_dir, 'en')
    cmd_line = f'sphinx-multiversion "{src}" "{dst}" -j {cpu}'
    subprocess.run(cmd_line, shell=True)

    print("Start build Chinese version")
    src = os.path.join(base_dir, 'zh')
    dst = os.path.join(output_dir, 'zh')
    cmd_line = f'sphinx-multiversion "{src}" "{dst}" -j {cpu}'
    subprocess.run(cmd_line, shell=True)


if __name__ == '__main__':
    run(sys.argv[1:])