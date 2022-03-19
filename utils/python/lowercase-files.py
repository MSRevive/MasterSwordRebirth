"""
Lowercase all of the files for the project.
"""
import os

def lowerFileCase(f):
    oldF = f
    os.rename(f, f.lower())
    print(f'Rename {oldF} to {f}')

def convertHeaderCase(f):
    should_update, content = False, []
    
    with open(f, 'r') as c:
        for line in c:
            if ('#include' in line) and (not '//' in line) and (not ';' in line) and (any(x.isupper() for x in line)):
                should_update = True
                line.lower()
                file = line.rsplit('/', 1)[-1]
                newFile = file.lower()
                line = line.replace(file, newFile)
            content.append(line)
     
    if should_update:
        print("Writing: ", f)
        with open(f, 'w') as c:
            c.write(''.join(content))

if __name__ == "__main__":
    for root, dirs, files in os.walk(r'D:\github\msrevive\MasterSwordRebirth\cl_dll'):
        types = set(['cpp', 'h', 'c'])
        for f in files:
            f = '{}\\{}'.format(root, f)
    
            if not (f.lower().split('.')[-1] in types):
                continue
    
            try:
                lowerFileCase(f)
            except Exception as e:
                print('ERROR:', f, '-->', e)
    
            try:
                convertHeaderCase(f)
            except Exception as e:
                print('ERROR:', f, '-->', e)
