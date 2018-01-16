## Accessing DeepBlue
All access to DeepBlue is made through the XML-RPC protocol to the server located at [http://deepblue.mpi-inf.mpg.de/xmlrpc](http://deepblue.mpi-inf.mpg.de/xmlrpc).

Note that while the examples in this manual focus on the *Python 2.7* programming language, DeepBlue commands should work in nearly all programming languages. For XML-RPC-based  access to DeepBlue from other languages, please check the following resources:
 * Java - [Apache XML-RPC](http://ws.apache.org/xmlrpc/)
 * R - We developed an optimized XML-RPC library for accessing DeepBlue from R: [DeepBlue-R](https://raw.githubusercontent.com/MPIIComputationalEpigenetics/DeepBlue-R/master/py/deepblue.r)
 * PHP - [XML-RPC](http://php.net/manual/en/book.xmlrpc.php)
 * Other programming languages [google it](https://www.google.com/search?q=xml+rpc+%3Cyour%20programming%20language%3E)

We use the library [xmlrpclib](https://docs.python.org/2/library/xmlrpclib.html) to access the DeepBlue Server in *Python 2.7*.
In *Python 3*, the library [xmlrpc.client](https://docs.python.org/3.0/library/xmlrpc.client.html) must be used.
To access DeepBlue, we must instantiate a client object.
For example, the following snipped instantiated an object, called *server*, that will be used to access the DeepBlue server. This snipped should be used as a prefix for all future examples.

```python
import xmlrpclib
user_key = "anonymous_key"
url = "http://deepblue.mpi-inf.mpg.de/xmlrpc"
server = xmlrpclib.Server(url, encoding='UTF-8', allow_none=True)
```

The *user_key* stores a user's credentials for accessing the server. The section [Creating User](01-02-creating-user.md) describes how to obtain a *user_key*.
In the final line of the example above, we create an object for accessing the server. Be sure to use *UTF-8* encoding, and *allow_none=True*.

Execute the following code:
```python
import xmlrpclib
url = "http://deepblue.mpi-inf.mpg.de/xmlrpc"
server = xmlrpclib.Server(url, encoding='UTF-8', allow_none=True)
server.echo(None)
```

The previous code should print: ```['okay', 'Deep Blue (0.9.5) says hi to a Stranger']```

Some commands, for example [list_experiments](http://deepblue.mpi-inf.mpg.de/api.php#api-list_experiments), have some optional parameters. For these, it is possible to enter *None* (or the *null* value in any other programming language) or an empty string (```""```).
The last line of the previous code snippet can be rewritten as:
```python
server.echo("")
```

### Commands Response

The commands response is a tuple with two elements. The first element is the status: returning ```okay``` when the command is successfully executed or ```error``` when a error occurs.
When the command is executed successfully (```okay```), the second tuple element is the command result; otherwise, this element contains an error message.

It is recommendable to check the result status after each command execution:

```python
(status, result) = server.echo(None)
if status == "error":
  print "Error : " + result
else:
  print result
```
