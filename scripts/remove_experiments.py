import xmlrpclib
from Queue import Queue
from threading import Thread

URL = "http://deepblue.mpi-inf.mpg.de/xmlrpc"

URL = "http://localhost:31415"
uk = "USER_KEY"

uk = "MqcV4PSCNuz73ZSw"

project = "KNIH"

q = Queue()

def worker():
	__server = xmlrpclib.Server(URL, allow_none=True)
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


server = xmlrpclib.Server(URL, allow_none=True)
experiments = server.list_experiments("", "", "", "", "", "", project, uk)[1]

exp_ids = server.extract_ids(experiments)[1]
for _id in exp_ids:
	q.put(_id)


gene_expressions = server.list_expressions("gene", "", None, project, uk)[1]
print gene_expressions

gx_ids = server.extract_ids(gene_expressions)[1]
for _id in gx_ids:
       q.put(_id)

q.join()


print server.info("me", uk)
