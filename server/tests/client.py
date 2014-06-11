import xmlrpclib

class EpidbClient:
 # def __init__(self, address="deepblue.mpi-inf.mpg.de/xmlrpc", port=None):
  def __init__(self, address="localhost", port=31415):
    if port != None:
      url = "http://%s:%d" % (address, port)
    else:
      url = "http://%s" % (address)
    self.server = xmlrpclib.Server(url, encoding='UTF-8', allow_none=True, verbose=False)

## Status command

  def echo(self, user_key):
    return self.server.echo(user_key)

  def test_types(self, *args):
    return self.server.test_types(*args)

  def commands(self):
    return self.server.commands()


## Main Operations

  ## General information
  def info(self, id, user_key):
    return self.server.info(id, user_key)

  def search(self, text, type_, user_key):
    return self.server.search(text, type_, user_key)


  ## Genomes
  def add_genome(self, name, description, chromosomes, user_key):
    return self.server.add_genome(name, description, chromosomes, user_key)

  def list_genomes(self, user_key):
    return self.server.list_genomes(user_key)

  def list_similar_genomes(self, name, user_key):
    return self.server.list_similar_genomes(name, user_key)

  def chromosomes(self, genome, user_key):
    return self.server.chromosomes(genome, user_key)

  def upload_chromosome(self, genome, chromosome, sequence, user_key):
    return self.server.upload_chromosome(genome, chromosome, sequence, user_key)


  ## Bio Sources
  def add_bio_source(self, name, description, extra_metadata, user_key):
    return self.server.add_bio_source(name, description, extra_metadata, user_key)

  def list_bio_sources(self, user_key):
    return self.server.list_bio_sources(user_key)

  def list_similar_bio_sources(self, bio_source_name, user_key):
    return self.server.list_similar_bio_sources(name, user_key)


  ## Samples
  def add_sample(self, bio_source_name, fields, user_key):
    return self.server.add_sample(bio_source_name, fields, user_key)

  def add_sample_field(self, name, type_, description, user_key):
    return self.server.add_sample_field(name, type_, description, user_key)

  def list_samples(self, bio_source, metadata, user_key):
    return self.server.list_samples(bio_source, metadata, user_key)


  ## Epigenetic Marks
  def add_epigenetic_mark(self, name, description, user_key):
    return self.server.add_epigenetic_mark(name, description, user_key)

  def list_epigenetic_marks(self, user_key):
    return self.server.list_epigenetic_marks(user_key)

  def list_similar_epigenetic_marks(self, term_name, user_key):
    return self.server.list_similar_epigenetic_marks(term_name, user_key)


  ## Column types
  def create_column_type_simple(self, name, description, ignore_if, type_, user_key):
    return self.server.create_column_type_simple(name, description, ignore_if, type_, user_key)

  def create_column_type_range(self, name, description, ignore_if, minimu, maximum, user_key):
    return self.server.create_column_type_range(name, description, ignore_if, minimu, maximum, user_key)

  def create_column_type_category(self, name, description, ignore_if, items, user_key):
    return self.server.create_column_type_category(name, description, ignore_if, items, user_key)

  def list_column_types(self, user_key):
    return self.server.list_column_types(user_key)


  ## Techniques
  def add_technique(self, name, description, metadata, user_key):
    return self.server.add_technique(name, description, metadata, user_key)

  def list_techniques(self, user_key):
    return self.server.list_techniques(user_key)

  def list_similar_techniques(self, name, user_key):
    return self.server.list_similar_techniques(name, user_key)


  ## Projects
  def add_project(self, name, description, user_key):
    return self.server.add_project(name, description, user_key)

  def list_projects(self, user_key):
    return self.server.list_projects(user_key)

  def list_similar_projects(self, project_name, user_key):
    return self.server.list_similar_projects(project_name, user_key)


  ## Bio Source names
  def set_bio_source_synonym(self, bio_source_name, synonym_name, user_key):
    return self.server.set_bio_source_synonym(bio_source_name, synonym_name, user_key)

  def get_bio_source_synonyms(self, bio_source_name, user_key):
    return self.server.get_bio_source_synonyms(bio_source_name, user_key)

  def set_bio_source_scope(self, bio_source_name_more_embracing, bio_source_name_less_embracing, user_key):
    return self.server.set_bio_source_scope(bio_source_name_more_embracing, bio_source_name_less_embracing, user_key)

  def get_bio_source_scope(self, bio_source_name, user_key):
    return self.server.get_bio_source_scope(bio_source_name, user_key)

  def get_bio_source_related(self, bio_source_name, user_key):
    return self.server.get_bio_source_related(bio_source_name, user_key)


  ## Expermiments
  def add_experiment(self, name, genome, epigenetic_mark, sample, technique, project, description, data, frmt, extra_metadata, user_key):
    return self.server.add_experiment(name, genome, epigenetic_mark, sample, technique, project, description, data, frmt, extra_metadata, user_key)

  def get_experiments_by_query(self, qid, user_key):
    return self.server.get_experiments_by_query(qid, user_key)

  def list_experiments(self, genome, epigenetic_mark, sample, technique, project, user_key):
    return self.server.list_experiments(genome, epigenetic_mark, sample, technique, project, user_key)

  def list_similar_experiments(self, name, genome, user_key):
    return self.server.list_similar_experiments(name, genome, user_key)

  def list_recent_experiments(self, days, genome, epigenetic_mark, sample, technique, project, user_key):
    return self.server.list_recent_experiments(days, genome, epigenetic_mark, sample, technique, project, user_key)


  ## Annotations
  def add_annotation(self, name, genome, description, data, frmt, extra_metadata, user_key):
    return self.server.add_annotation(name, genome, description, data, frmt, extra_metadata, user_key)

  def list_annotations(self, genome, user_key):
    return self.server.list_annotations(genome, user_key)

  def find_pattern(self, pattern, genome, overlap, user_key):
    return self.server.find_pattern(pattern, genome, overlap, user_key)

  ## Operations
  def select_regions(self, experiment_name, genome, epigenetic_mark, sample, technique, project, chromosome, start, end, user):
    return self.server.select_regions(experiment_name, genome, epigenetic_mark, sample, technique, project, chromosome, start, end, user)

  def select_annotations(self, annotations_name, genome, chromosomes, start, end, user):
    return self.server.select_annotations(annotations_name, genome, chromosomes, start, end, user)

  def intersection(self, query_a_id, query_b_id, user_key):
    return self.server.intersection(query_a_id, query_b_id, user_key)

  def aggregate(self, regions_data_id, regions_range_id, field, user_key):
      return self.server.aggregate(regions_data_id, regions_range_id, field, user_key)

  def merge_queries(self, query_a_id, query_b_id, user_key):
    return self.server.merge_queries(query_a_id, query_b_id, user_key)

  def filter_regions(self, query_id, field, operation, value, type_, user_key):
    return self.server.filter_regions(query_id, field, operation, value, type_,  user_key)

  def tiling_regions(self, size, genome, chromosomes, user_key):
    return self.server.tiling_regions(size, genome, chromosomes, user_key)

  def count_regions(self, query_id, user_key):
    return self.server.count_regions(query_id, user_key)

  def get_regions(self, query_id, format, user_key):
    return self.server.get_regions(query_id, format, user_key)


## Admin Operations

  def init_system(self, name, email, institution):
    return self.server.init_system(name, email, institution)

  def add_user(self, name, email, institution, admin_key):
    return self.server.add_user(name, email, institution, admin_key)

  def list_users(self, admin_key):
    return self.server.list_users(admin_key)
