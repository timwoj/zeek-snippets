import matplotlib.pyplot as plt
import csv
import sys

x = []
y = []

count = 0
total = 0
max_time = 0;
with open(sys.argv[1], 'r') as csvfile:
    plots = csv.reader(csvfile, delimiter=',')
    for row in plots:
        t = int(row[1]) / 1000
        if t > 100:
            continue
        x.append(int(row[0]))
        y.append(t)

        count += 1
        total += t
        if t > max_time:
            max_time = t

print("Average operation time (us): %f" % (float(total) / float(count)))
print("Max operation time (us): %d" % max_time)

plt.figure(figsize=(8,6), dpi=100)
plt.plot(y, 'bo')
plt.xlabel('Elements')
plt.ylabel('Time (us)')
plt.show()
