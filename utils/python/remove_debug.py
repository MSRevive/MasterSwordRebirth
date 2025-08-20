#!/usr/bin/env python3
import re
import os
import sys

def remove_debug_from_file(filepath):
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            content = f.read()
        
        original_content = content
        
        # Remove dbg(...) calls
        content = re.sub(r'\s*dbg\([^)]*\);\s*\n', '', content)
        
        # Remove genitemdbg(...) calls
        content = re.sub(r'\s*genitemdbg\([^)]*\);\s*\n', '', content)
        
        # Remove #define genitemdbg line
        content = re.sub(r'#define genitemdbg\([^)]*\)[^\n]*\n', '', content)
        
        # Replace startdbg; with try {
        content = re.sub(r'(\s*)startdbg;', r'\1try {', content)
        
        # Replace enddbg; with }
        content = re.sub(r'(\s*)enddbg;', r'\1}', content)
        
        # Replace enddbgprt(...); with }
        content = re.sub(r'(\s*)enddbgprt\([^)]*\);', r'\1}', content)
        
        # Remove SetDebugProgress calls
        content = re.sub(r'\s*SetDebugProgress\([^)]*\);\s*\n', '', content)
        
        if content != original_content:
            with open(filepath, 'w', encoding='utf-8') as f:
                f.write(content)
            return True
        return False
        
    except Exception as e:
        print(f"Error processing {filepath}: {e}")
        return False

files_to_process = []

modified_files = []
for filepath in files_to_process:
    if os.path.exists(filepath):
        if remove_debug_from_file(filepath):
            modified_files.append(filepath)
            print(f"Modified: {filepath}")
        else:
            print(f"No changes needed: {filepath}")
    else:
        print(f"File not found: {filepath}")

print(f"\nTotal files modified: {len(modified_files)}")