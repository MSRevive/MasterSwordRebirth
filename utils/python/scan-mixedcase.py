"""
Scan which files for mixed case #include
"""

import os

def isValid(line):
    if ('SDL' in line) or ('VGUI' in line):
        return False

    return True

def checkFile(f):
    with open(f, 'r') as c:        
        items = [line.strip() for line in c if (('#include' in line) and isValid(line.strip()) and (line.lower() != line))]
        if len(items) > 0:
            print("File:", f)
            for l in items:
                print('\t', l)

def scan():
    types = set(['cpp', 'hpp', 'h', 'c'])
    for root, subdirs, files in os.walk(r'E:\Cloud\GIT\ms-rebirth'):
        if '.git' in root:
            continue

        for f in files:
            f = '{}\\{}'.format(root, f)
            if not (f.lower().split('.')[-1] in types):
                continue
            try:
                checkFile(f)
            except Exception as e:
                print('ERROR:', f, '-->', e)

if __name__ == "__main__":
    scan()
