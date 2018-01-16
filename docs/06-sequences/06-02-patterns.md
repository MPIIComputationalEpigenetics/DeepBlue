## Patterns

The command [find_pattern](http://deepblue.mpi-inf.mpg.de/api.php#api-find_pattern) creates an annotation based on a given sequence pattern.
This command has three parameters: the pattern, in [Perl Regular Expression Syntax](http://www.boost.org/doc/libs/1_44_0/libs/regex/doc/php/boost_regex/syntax/perl_syntax.php), the ```genome```, and a boolean, to specify if the pattern matching will be overlapping or non-overlapping.

The difference between overlapping and non-overlapping is as follows: when a non-overlapping pattern is found, the pattern searching continues in the position after the end end of the match. In the case of an  overlapping pattern, the searching process continues one position after the beginning of the match:

```python
status, _id = server.find_pattern("TATA", "hg19_only_chr19", True, user_key)
print server.info(_id, user_key)
```

### Motif Metafield

It is possible to count how many times a pattern occurs in a region with the meta-field ``@COUNT.MOTIF(PATTERN)``.

```python
(status, ann) = server.tiling_regions(50000, "hg19", "chr1", user_key)

fmt = "CHROMOSOME,START,END,@NAME,@LENGTH,@COUNT.MOTIF(TATA),\
       @COUNT.MOTIF(CG),@COUNT.MOTIF((TATA|CG))"
(status, regions_request_id) = server.get_regions(ann, fmt, user_key)

# Wait for the server processing
(status, info) = server.info(regions_request_id, user_key)
request_status = info[0]["state"]
while request_status != "done" and request_status != "failed":
  time.sleep(1)
  (status, info) = server.info(regions_request_id, user_key)
  request_status = info[0]["state"]
  print request_status


(status, regions) = server.get_request_data(regions_request_id, user_key)
print regions
```
