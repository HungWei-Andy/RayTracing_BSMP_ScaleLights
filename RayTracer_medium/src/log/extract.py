import sys

filename = sys.argv[1]

f = open(filename)
fout = open('time.csv', 'w')

lines = f.readlines()
fout.write('real,user\n')
for line in lines:
  if line.strip().startswith('real'):
    fout.write(line.split()[1]+',')
  if line.strip().startswith('user'):
    fout.write(line.split()[1]+'\n')

