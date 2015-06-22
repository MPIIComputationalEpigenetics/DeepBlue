import gzip
import inspect
import subprocess
import os.path
import time
import unittest
import time
import sys

from pymongo import MongoClient
from client import EpidbClient

import settings
import data_info as data

"""
setUp starts the complete MongoDB cluster and emptys the database.
"""
def setUp():
  # start mongodb cluster

  #print subprocess.call(["/opt/mongo-clang-libc++/bin/mongod", "--dbpath /Users/albrecht/epidb/data/single/","--setParameter textSearchEnabled=true", "--enableExperimentalIndexStatsCmd"])

  print "----- starting mongodb cluster -----"
  #subprocess.call([settings.MONGODB_SCRIPTS["config_server"], "start"])
  #subprocess.call([settings.MONGODB_SCRIPTS["mongos"], "start"])
  #subprocess.call([settings.MONGODB_SCRIPTS["shard"], "start"])
  print "\n"

  # clear database
  print "----- clearing database -----"
  mongo = MongoClient("localhost", 27017)
  mongo.drop_database(settings.DATABASE_NAME)
  print "\n"

  # start epidb
  #print "----- starting epidb -----"
  #epidb_process = subprocess.Popen([settings.EPIDB_BIN, "0.0.0.0", "31415", "1", "localhost"])
  print "\n"
  #time.sleep(2)

  # module scope seems to be destroyed before executing the `finally' statement
  # in test.py so we return the process directly.
  # TODO: init scripts for epidb?
  #return epidb_process

"""
tearDown shuts down the whole MongoDB cluster.
"""
def tearDown(epidb_process=None):
  pass
  # stop mongodb cluster
  #print "----- stopping mongodb cluster -----"
  #subprocess.call([settings.MONGODB_SCRIPTS["config_server"], "stop"])
  #subprocess.call([settings.MONGODB_SCRIPTS["mongos"], "stop"])
  #subprocess.call([settings.MONGODB_SCRIPTS["shard"], "stop"])
  #print "\n"

  #if epidb_process is not None:
  #  print "----- stopping epidb -----"
  #  epidb_process.terminate()
  #  print "\n"


def parse_result_file(f):
  results = {}
  current = None

  for l in f:
    if l.startswith('#') or l.strip() == "":
      continue

    if l.startswith('::'):
      current = l[2:].strip()
      results[current] = ""
    elif current:
      results[current] += l

  for res in results:
    results[res] = results[res].strip('\r\n')

  return results


def get_result(test, name=None):
  if not name:
    name = test

    frm = inspect.stack()[1]
    filename = os.path.basename(frm[1])
    test = filename.split('.')[0]

  with open("test_cases/result_data/%s.bed.res" % test, 'r') as f:
    results = parse_result_file(f)
    return results[name]


def load_bed(name):
  filename = "data/bed/%s.bed" % name
  with open(filename, 'r') as f:
    return f.read()

def load_wig(name):
  filename = "data/wig/%s.wig" % name
  with open(filename, 'r') as f:
    return f.read()


def load_bedgraph(name):
  filename = "data/bedgraph/%s.bg.gz" % name
  with gzip.open(filename, 'r') as f:
    return f.read()

class TestCase(unittest.TestCase):

  admin_key = None

  sample_ids = []
  experiment_ids = []

  @classmethod
  def setUpClass(cls):
    pass
    #cls._epidb_process = subprocess.Popen([settings.EPIDB_BIN, "0.0.0.0", "31415", "1", "localhost"],  stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    #time.sleep(2)

  @classmethod
  def tearDownClass(cls):
    pass
    #cls._epidb_process.terminate()

  def setUp(self):
    mongo = MongoClient("localhost", 27017)
    mongo.drop_database("tests_suite")
    mongo.drop_database("tests_suite_queue")
    self.startTime = time.time()


  def tearDown(self):
    t = time.time() - self.startTime
    elapsed = "\t%.3f\t" % (t)
    sys.stderr.write(elapsed)
    #print "--- Stoping Server ---"
    #self.epidb_process.terminate()

  def assertSuccess(self, result, data=None):
    if isinstance(result, tuple) or isinstance(result, list):
      status = result[0]
    else:
      status = result

    if status != 'okay':
      if data == None:
        msg = "Command failed: %s" % (result)
      else:
        msg = "Command failed: %s %s" % (result, data)
      raise self.failureException, msg

  def assertFailure(self, result, data=None):
    if data is None:
      data = result[1]
      result = result[0]

    if result != 'error':
      msg = "Command did not fail"
      raise self.failureException, msg


  def insert_genome(self, epidb, name):
    genome = data.GENOMES[name]
    genome_info = None
    with open(genome["info_file"], 'r') as f:
      genome_info = f.read().replace(",", "")

    res, gid = epidb.add_genome(name, genome["description"], genome_info, self.admin_key)
    self.assertSuccess(res, gid)

    return gid


  def insert_column(self, epidb, column):
    res, cid =epidb.create_column_type_simple(column[0], column[1], column[2], self.admin_key)
    self.assertSuccess(res, cid)
    return cid

  def insert_annotation(self, epidb, name):
    ann = data.ANNOTATIONS[name]
    annotation_data = None
    with open(ann["data_file"], 'r') as f:
      annotation_data = f.read()

    res, aid = epidb.add_annotation(name, ann["genome"], ann["description"],
               annotation_data, ann["format"], ann["metadata"],
              self.admin_key)
    self.assertSuccess(res, aid)

    return aid


  def insert_samples(self, epidb, name):
    sam = data.SAMPLES[name]
    res, sid = epidb.add_sample(name, sam["metadata"], self.admin_key)
    self.assertSuccess(res, sid)

    self.sample_ids.append(sid)

    return sid


  def insert_epigenetic_mark(self, epidb, name):
    mark = data.EPIGENETIC_MARKS[name]
    res, emid = epidb.add_epigenetic_mark(name, mark["description"], self.admin_key)
    self.assertSuccess(res, emid)

    return emid


  def insert_biosource(self, epidb, name):
    bs = data.BIOSOURCES[name]
    res, bsid = epidb.add_biosource(name, bs["description"], bs["metadata"], self.admin_key)
    self.assertSuccess(res, bsid)

    return bsid


  def insert_technique(self, epidb, name):
    tech = data.TECHNIQUES[name]
    res, tid = epidb.add_technique(name, tech["description"], tech["metadata"], self.admin_key)
    self.assertSuccess(res, tid)

    return tid


  def insert_project(self, epidb, name):
    project = data.PROJECTS[name]
    res, pid = epidb.add_project(name, project["description"], self.admin_key)
    self.assertSuccess(res, pid)

    return pid


  def insert_experiment(self, epidb, exp, sample_id=None):
    bed_data = load_bed(exp)
    info = data.EXPERIMENTS[exp]

    if not sample_id and not info["sample_id"] and len(self.sample_ids) > 0:
      sample_id = self.sample_ids[0]

    res, eid = epidb.add_experiment(exp, info["genome"], info["epigenetic_mark"], sample_id,
                info["technique"], info["project"], info["description"], bed_data, info["format"],
                None, self.admin_key)

    self.assertSuccess(res, eid)
    self.experiment_ids.append(eid)

    return eid

  def insert_user(self, epidb, name):
    user = data.USERS[name]
    res, user = epidb.add_user(name, user["email"], user["institution"], self.admin_key)
    self.assertSuccess(res, user)

    return user



  def insert_experiments(self, epidb, experiments):
    return [insert_experiment(epidb, exp) for exp in experiments]


  def init(self, epidb=None):
    if not epidb:
      epidb = EpidbClient()

    res, key = epidb.init_system(*settings.EPIDB_TEST_ADMIN)
    self.assertSuccess(res, key)

    self.admin_key = key


  def init_base(self, epidb=None):
    if not epidb:
      epidb = EpidbClient()

    self.init(epidb)

    for genome in data.GENOMES:
      self.insert_genome(epidb, genome)

    for bsource in data.BIOSOURCES:
      self.insert_biosource(epidb, bsource)

    for tech in data.TECHNIQUES:
      self.insert_technique(epidb, tech)

    for epimark in data.EPIGENETIC_MARKS:
      self.insert_epigenetic_mark(epidb, epimark)

    for project in data.PROJECTS:
      self.insert_project(epidb, project)

    for sample in data.SAMPLES:
      self.insert_samples(epidb, sample)

    for column in data.COLUMNS:
      self.insert_column(epidb, column)
    res, cid = epidb.create_column_type_category("STRAND", "Region strand: +, -, .", ["+", "-", "."], self.admin_key)
    self.assertSuccess(res, cid)

  def init_full(self, epidb=None):
    if not epidb:
      epidb = EpidbClient()
    self.init_base(epidb)

    for user in data.USERS:
      self.insert_user(epidb, user)

    for ann in data.ANNOTATIONS:
      self.insert_annotation(epidb, ann)

    for exp in data.EXPERIMENTS:
      if not "big" in exp:
        self.insert_experiment(epidb, exp)

  def count_request(self, req):
    if req[0] is not 'r':
      print "Invalid request " + req
      return

    epidb = EpidbClient()
    sleep = 0.1
    (s, ss) = epidb.info(req, self.admin_key)
    while ss[0]["state"] != "done":
      time.sleep(sleep)
      (s, ss) = epidb.info(req, self.admin_key)
      sleep += sleep

    (s, data) = epidb.get_request_data(req, self.admin_key)

    self.assertSuccess(s, data)

    return data["count"]

  def __get_regions_request(self, req, status=["done"]):
    if req[0] is not 'r':
      print "Invalid request " + req
      return

    epidb = EpidbClient()
    sleep = 0.1
    (s, ss) = epidb.info(req, self.admin_key)
    while ss[0]["state"] not in status:
      time.sleep(sleep)
      (s, ss) = epidb.info(req, self.admin_key)
      sleep += sleep

    (s, data) = epidb.get_request_data(req, self.admin_key)
    return (s, data)

  def get_regions_request(self, req):
    (s, data) = self.__get_regions_request(req)
    self.assertSuccess(s, data)
    return data

  def get_regions_request_error(self, req):
    (s, data) = self.__get_regions_request(req, ["failed"])
    self.assertFailure(s, data)
    return data
