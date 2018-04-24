import xmlrpclib
from Queue import Queue
from threading import Thread

uk = "1kbgOVXjTJuQ4v8s"

q = Queue()

def worker():
	__server = xmlrpclib.Server("http://localhost:31415", allow_none=True)
	while True:
	    	_id = q.get()
        	print "removing " +  _id
		try:
			(s, _id) = __server.remove(_id, uk)
                        print _id + " removed"
		except Exception as e:
			print e
		finally:
			q.task_done()

for i in xrange(4):
	t = Thread(target=worker)
	t.daemon = True
	t.start()


server = xmlrpclib.Server("http://localhost:31415", allow_none=True)
experiments = server.list_experiments("", "peaks", "flrna", None, None, None,	"BLUEPRINT Epigenome", uk)[1]

print experiments

exp_ids = server.extract_ids(experiments)[1]
for _id in exp_ids:
	q.put(_id)
q.join()
