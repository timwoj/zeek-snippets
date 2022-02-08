import sys

df = sys.argv[1]
with open(df, 'r') as f:

    class_name = ''
    for line in f.readlines():

        if line.startswith('struct') or line.startswith('class'):
            class_name = line.split(' ')[1]
            lt = class_name.find('<')
            if lt != -1:
                class_name = class_name[:lt]
            if len(class_name) > 40:
                class_name = class_name[:10]
        elif class_name and line.startswith('	/* size:'):
            size = line.split(' ')[2][:-1]
            print('{:40s} {:s}'.format(class_name, size))
            class_name = ''
