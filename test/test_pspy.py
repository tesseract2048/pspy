import pspy
import os
import time

ps = pspy.pspy(100000000, 5000000, 4)
ts = time.time()
for i in range(0, 100000):
    infohash = os.urandom(20)
    for j in range(0, 100):
        peerid = os.urandom(6)
        ps.put(infohash, peerid)
print 'put done: %s' % (time.time() - ts)
print 'size: %s' % ps.size()
ts = time.time()
ps.collect()
print 'collect done: %s' % (time.time() - ts)