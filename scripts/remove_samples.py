import xmlrpclib
from Queue import Queue
from threading import Thread

uk = "USER_KEY"

q = Queue()

def worker():
	__server = xmlrpclib.Server("http://deepblue.mpi-inf.mpg.de/xmlrpc", allow_none=True)
	while True:
	    	_id = q.get()
        	print  _id
		print __server.remove(_id, uk)
        	q.task_done()

for i in xrange(16):
	t = Thread(target=worker)
	t.daemon = True
	t.start()


server = xmlrpclib.Server("http://deepblue.mpi-inf.mpg.de/xmlrpc", allow_none=True)
samples = server.list_samples("", {"source":"BLUEPRINT Progenitors"}, uk)[1]
s_id = [s[0] for s in samples]

print len(s_id)

for _id in s_id:
	q.put(_id)

q.join()


