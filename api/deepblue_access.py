# Python helper for For DeepBlue version 1.7.19

import xmlrpclib

deepblue_URL = "http://deepblue.mpi-inf.mpg.de/xmlrpc"
deepblue_USER_KEY = "anonymous_key"
deepblue_debug_VERBOSE = False

def key_required(func):
    def func_wrapper(self, *args, **kwargs):
        if self.key:
            return func(self, *args, **kwargs)
        else:
            raise AttributeError("To use this function a key is required. Set it using "
                                 "set_key or construct DeepBlueClient providing a key")
    return func_wrapper

class DeepBlueClient(object):
  """Conveniently access a DeepBlue server
  """
  def __init__(self, key=None, address=DEEPBLUE_url, port=None):
    """
    :param key: Authentication key to be used for all commands
    :param address: Address of the server DeepBlue runs on
    :param port: Port on the server DeepBlue listens on
    """
    self.key = key
    if port is not None:
      url = "http://%s:%d" % (address, port)
    else:
      url = "http://%s" % address
    self.server = xmlrpclib.Server(url, encoding='UTF-8', allow_none=True, verbose=False)



  def add_annotation(self, name=None, genome=None, description=None, data=None, format=None, extra_metadata=None, user_key=deepblue_USER_KEY):
    return self.server.add_annotation(name=None, genome=None, description=None, data=None, format=None, extra_metadata=None, user_key=deepblue_USER_KEY)

  def add_biosource(self, name=None, description=None, extra_metadata=None, user_key=deepblue_USER_KEY):
    return self.server.add_biosource(name=None, description=None, extra_metadata=None, user_key=deepblue_USER_KEY)

  def add_epigenetic_mark(self, name=None, description=None, extra_metadata=None, user_key=deepblue_USER_KEY):
    return self.server.add_epigenetic_mark(name=None, description=None, extra_metadata=None, user_key=deepblue_USER_KEY)

  def add_experiment(self, name=None, genome=None, epigenetic_mark=None, sample=None, technique=None, project=None, description=None, data=None, format=None, extra_metadata=None, user_key=deepblue_USER_KEY):
    return self.server.add_experiment(name=None, genome=None, epigenetic_mark=None, sample=None, technique=None, project=None, description=None, data=None, format=None, extra_metadata=None, user_key=deepblue_USER_KEY)

  def add_gene_model(self, name=None, description=None, data=None, format=None, extra_metadata=None, user_key=deepblue_USER_KEY):
    return self.server.add_gene_model(name=None, description=None, data=None, format=None, extra_metadata=None, user_key=deepblue_USER_KEY)

  def add_genome(self, name=None, description=None, data=None, user_key=deepblue_USER_KEY):
    return self.server.add_genome(name=None, description=None, data=None, user_key=deepblue_USER_KEY)

  def add_project(self, name=None, description=None, user_key=deepblue_USER_KEY):
    return self.server.add_project(name=None, description=None, user_key=deepblue_USER_KEY)

  def add_sample(self, biosource=None, extra_metadata=None, user_key=deepblue_USER_KEY):
    return self.server.add_sample(biosource=None, extra_metadata=None, user_key=deepblue_USER_KEY)

  def add_sample_from_gsm(self, biosource=None, gsm_id=None, user_key=deepblue_USER_KEY):
    return self.server.add_sample_from_gsm(biosource=None, gsm_id=None, user_key=deepblue_USER_KEY)

  def add_technique(self, name=None, description=None, extra_metadata=None, user_key=deepblue_USER_KEY):
    return self.server.add_technique(name=None, description=None, extra_metadata=None, user_key=deepblue_USER_KEY)

  def add_user_to_project(self, user=None, project=None, set=None, user_key=deepblue_USER_KEY):
    return self.server.add_user_to_project(user=None, project=None, set=None, user_key=deepblue_USER_KEY)

  def aggregate(self, data_id=None, ranges_id=None, column=None, user_key=deepblue_USER_KEY):
    return self.server.aggregate(data_id=None, ranges_id=None, column=None, user_key=deepblue_USER_KEY)

  def cancel_request(self, id=None, user_key=deepblue_USER_KEY):
    return self.server.cancel_request(id=None, user_key=deepblue_USER_KEY)

  def change_extra_metadata(self, id=None, key=None, value=None, user_key=deepblue_USER_KEY):
    return self.server.change_extra_metadata(id=None, key=None, value=None, user_key=deepblue_USER_KEY)

  def chromosomes(self, genome=None, user_key=deepblue_USER_KEY):
    return self.server.chromosomes(genome=None, user_key=deepblue_USER_KEY)

  def clone_dataset(self, dataset_id=None, new_name=None, new_epigenetic_mark=None, new_sample=None, new_technique=None, new_project=None, description=None, format=None, extra_metadata=None, user_key=deepblue_USER_KEY):
    return self.server.clone_dataset(dataset_id=None, new_name=None, new_epigenetic_mark=None, new_sample=None, new_technique=None, new_project=None, description=None, format=None, extra_metadata=None, user_key=deepblue_USER_KEY)

  def collection_experiments_count(self, controlled_vocabulary=None, genome=None, type=None, epigenetic_mark=None, biosource=None, sample=None, technique=None, project=None, user_key=deepblue_USER_KEY):
    return self.server.collection_experiments_count(controlled_vocabulary=None, genome=None, type=None, epigenetic_mark=None, biosource=None, sample=None, technique=None, project=None, user_key=deepblue_USER_KEY)

  def commands(self, ):
    return self.server.commands()

  def count_regions(self, query_id=None, user_key=deepblue_USER_KEY):
    return self.server.count_regions(query_id=None, user_key=deepblue_USER_KEY)

  def coverage(self, query_id=None, genome=None, user_key=deepblue_USER_KEY):
    return self.server.coverage(query_id=None, genome=None, user_key=deepblue_USER_KEY)

  def create_column_type_calculated(self, name=None, description=None, code=None, user_key=deepblue_USER_KEY):
    return self.server.create_column_type_calculated(name=None, description=None, code=None, user_key=deepblue_USER_KEY)

  def create_column_type_category(self, name=None, description=None, items=None, user_key=deepblue_USER_KEY):
    return self.server.create_column_type_category(name=None, description=None, items=None, user_key=deepblue_USER_KEY)

  def create_column_type_range(self, name=None, description=None, minimum=None, maximum=None, user_key=deepblue_USER_KEY):
    return self.server.create_column_type_range(name=None, description=None, minimum=None, maximum=None, user_key=deepblue_USER_KEY)

  def create_column_type_simple(self, name=None, description=None, type=None, user_key=deepblue_USER_KEY):
    return self.server.create_column_type_simple(name=None, description=None, type=None, user_key=deepblue_USER_KEY)

  def echo(self, user_key=deepblue_USER_KEY):
    return self.server.echo(user_key=deepblue_USER_KEY)

  def extend(self, query_id=None, length=None, direction=None, use_strand=None, user_key=deepblue_USER_KEY):
    return self.server.extend(query_id=None, length=None, direction=None, use_strand=None, user_key=deepblue_USER_KEY)

  def extract_ids(self, list=None):
    return self.server.extract_ids(list=None)

  def extract_names(self, list=None):
    return self.server.extract_names(list=None)

  def faceting_experiments(self, genome=None, type=None, epigenetic_mark=None, biosource=None, sample=None, technique=None, project=None, user_key=deepblue_USER_KEY):
    return self.server.faceting_experiments(genome=None, type=None, epigenetic_mark=None, biosource=None, sample=None, technique=None, project=None, user_key=deepblue_USER_KEY)

  def filter_regions(self, query_id=None, field=None, operation=None, value=None, type=None, user_key=deepblue_USER_KEY):
    return self.server.filter_regions(query_id=None, field=None, operation=None, value=None, type=None, user_key=deepblue_USER_KEY)

  def find_pattern(self, pattern=None, genome=None, overlap=None, user_key=deepblue_USER_KEY):
    return self.server.find_pattern(pattern=None, genome=None, overlap=None, user_key=deepblue_USER_KEY)

  def flank(self, query_id=None, start=None, length=None, use_strand=None, user_key=deepblue_USER_KEY):
    return self.server.flank(query_id=None, start=None, length=None, use_strand=None, user_key=deepblue_USER_KEY)

  def get_biosource_children(self, biosource=None, user_key=deepblue_USER_KEY):
    return self.server.get_biosource_children(biosource=None, user_key=deepblue_USER_KEY)

  def get_biosource_parents(self, biosource=None, user_key=deepblue_USER_KEY):
    return self.server.get_biosource_parents(biosource=None, user_key=deepblue_USER_KEY)

  def get_biosource_related(self, biosource=None, user_key=deepblue_USER_KEY):
    return self.server.get_biosource_related(biosource=None, user_key=deepblue_USER_KEY)

  def get_biosource_synonyms(self, biosource=None, user_key=deepblue_USER_KEY):
    return self.server.get_biosource_synonyms(biosource=None, user_key=deepblue_USER_KEY)

  def get_experiments_by_query(self, query_id=None, user_key=deepblue_USER_KEY):
    return self.server.get_experiments_by_query(query_id=None, user_key=deepblue_USER_KEY)

  def get_regions(self, query_id=None, output_format=None, user_key=deepblue_USER_KEY):
    return self.server.get_regions(query_id=None, output_format=None, user_key=deepblue_USER_KEY)

  def get_request_data(self, request_id=None, user_key=deepblue_USER_KEY):
    return self.server.get_request_data(request_id=None, user_key=deepblue_USER_KEY)

  def info(self, id=None, user_key=deepblue_USER_KEY):
    return self.server.info(id=None, user_key=deepblue_USER_KEY)

  def input_regions(self, genome=None, region_set=None, user_key=deepblue_USER_KEY):
    return self.server.input_regions(genome=None, region_set=None, user_key=deepblue_USER_KEY)

  def intersection(self, query_data_id=None, query_filter_id=None, user_key=deepblue_USER_KEY):
    return self.server.intersection(query_data_id=None, query_filter_id=None, user_key=deepblue_USER_KEY)

  def is_biosource(self, biosource=None, user_key=deepblue_USER_KEY):
    return self.server.is_biosource(biosource=None, user_key=deepblue_USER_KEY)

  def list_annotations(self, genome=None, user_key=deepblue_USER_KEY):
    return self.server.list_annotations(genome=None, user_key=deepblue_USER_KEY)

  def list_biosources(self, extra_metadata=None, user_key=deepblue_USER_KEY):
    return self.server.list_biosources(extra_metadata=None, user_key=deepblue_USER_KEY)

  def list_column_types(self, user_key=deepblue_USER_KEY):
    return self.server.list_column_types(user_key=deepblue_USER_KEY)

  def list_epigenetic_marks(self, extra_metadata=None, user_key=deepblue_USER_KEY):
    return self.server.list_epigenetic_marks(extra_metadata=None, user_key=deepblue_USER_KEY)

  def list_experiments(self, genome=None, type=None, epigenetic_mark=None, biosource=None, sample=None, technique=None, project=None, user_key=deepblue_USER_KEY):
    return self.server.list_experiments(genome=None, type=None, epigenetic_mark=None, biosource=None, sample=None, technique=None, project=None, user_key=deepblue_USER_KEY)

  def list_gene_models(self, user_key=deepblue_USER_KEY):
    return self.server.list_gene_models(user_key=deepblue_USER_KEY)

  def list_genes(self, gene_id_or_name=None, chromosome=None, start=None, end=None, gene_models=None, user_key=deepblue_USER_KEY):
    return self.server.list_genes(gene_id_or_name=None, chromosome=None, start=None, end=None, gene_models=None, user_key=deepblue_USER_KEY)

  def list_genomes(self, user_key=deepblue_USER_KEY):
    return self.server.list_genomes(user_key=deepblue_USER_KEY)

  def list_in_use(self, controlled_vocabulary=None, user_key=deepblue_USER_KEY):
    return self.server.list_in_use(controlled_vocabulary=None, user_key=deepblue_USER_KEY)

  def list_projects(self, user_key=deepblue_USER_KEY):
    return self.server.list_projects(user_key=deepblue_USER_KEY)

  def list_recent_experiments(self, days=None, genome=None, epigenetic_mark=None, sample=None, technique=None, project=None, user_key=deepblue_USER_KEY):
    return self.server.list_recent_experiments(days=None, genome=None, epigenetic_mark=None, sample=None, technique=None, project=None, user_key=deepblue_USER_KEY)

  def list_requests(self, request_state=None, user_key=deepblue_USER_KEY):
    return self.server.list_requests(request_state=None, user_key=deepblue_USER_KEY)

  def list_samples(self, biosource=None, extra_metadata=None, user_key=deepblue_USER_KEY):
    return self.server.list_samples(biosource=None, extra_metadata=None, user_key=deepblue_USER_KEY)

  def list_similar_biosources(self, name=None, user_key=deepblue_USER_KEY):
    return self.server.list_similar_biosources(name=None, user_key=deepblue_USER_KEY)

  def list_similar_epigenetic_marks(self, name=None, user_key=deepblue_USER_KEY):
    return self.server.list_similar_epigenetic_marks(name=None, user_key=deepblue_USER_KEY)

  def list_similar_experiments(self, name=None, genome=None, user_key=deepblue_USER_KEY):
    return self.server.list_similar_experiments(name=None, genome=None, user_key=deepblue_USER_KEY)

  def list_similar_genomes(self, name=None, user_key=deepblue_USER_KEY):
    return self.server.list_similar_genomes(name=None, user_key=deepblue_USER_KEY)

  def list_similar_projects(self, name=None, user_key=deepblue_USER_KEY):
    return self.server.list_similar_projects(name=None, user_key=deepblue_USER_KEY)

  def list_similar_techniques(self, name=None, user_key=deepblue_USER_KEY):
    return self.server.list_similar_techniques(name=None, user_key=deepblue_USER_KEY)

  def list_techniques(self, user_key=deepblue_USER_KEY):
    return self.server.list_techniques(user_key=deepblue_USER_KEY)

  def merge_queries(self, query_a_id=None, query_b_id=None, user_key=deepblue_USER_KEY):
    return self.server.merge_queries(query_a_id=None, query_b_id=None, user_key=deepblue_USER_KEY)

  def name_to_id(self, name=None, collection=None, user_key=deepblue_USER_KEY):
    return self.server.name_to_id(name=None, collection=None, user_key=deepblue_USER_KEY)

  def preview_experiment(self, experiment_name=None, user_key=deepblue_USER_KEY):
    return self.server.preview_experiment(experiment_name=None, user_key=deepblue_USER_KEY)

  def query_cache(self, query_id=None, cache=None, user_key=deepblue_USER_KEY):
    return self.server.query_cache(query_id=None, cache=None, user_key=deepblue_USER_KEY)

  def query_experiment_type(self, query_id=None, type=None, user_key=deepblue_USER_KEY):
    return self.server.query_experiment_type(query_id=None, type=None, user_key=deepblue_USER_KEY)

  def remove(self, id=None, user_key=deepblue_USER_KEY):
    return self.server.remove(id=None, user_key=deepblue_USER_KEY)

  def score_matrix(self, experiments_columns=None, aggregation_function=None, aggregation_regions_id=None, user_key=deepblue_USER_KEY):
    return self.server.score_matrix(experiments_columns=None, aggregation_function=None, aggregation_regions_id=None, user_key=deepblue_USER_KEY)

  def search(self, keyword=None, type=None, user_key=deepblue_USER_KEY):
    return self.server.search(keyword=None, type=None, user_key=deepblue_USER_KEY)

  def select_annotations(self, annotation_name=None, genome=None, chromosome=None, start=None, end=None, user_key=deepblue_USER_KEY):
    return self.server.select_annotations(annotation_name=None, genome=None, chromosome=None, start=None, end=None, user_key=deepblue_USER_KEY)

  def select_experiments(self, experiment_name=None, chromosome=None, start=None, end=None, user_key=deepblue_USER_KEY):
    return self.server.select_experiments(experiment_name=None, chromosome=None, start=None, end=None, user_key=deepblue_USER_KEY)

  def select_genes(self, genes_name=None, gene_model=None, chromosome=None, start=None, end=None, user_key=deepblue_USER_KEY):
    return self.server.select_genes(genes_name=None, gene_model=None, chromosome=None, start=None, end=None, user_key=deepblue_USER_KEY)

  def select_regions(self, experiment_name=None, genome=None, epigenetic_mark=None, sample_id=None, technique=None, project=None, chromosome=None, start=None, end=None, user_key=deepblue_USER_KEY):
    return self.server.select_regions(experiment_name=None, genome=None, epigenetic_mark=None, sample_id=None, technique=None, project=None, chromosome=None, start=None, end=None, user_key=deepblue_USER_KEY)

  def set_biosource_parent(self, parent=None, child=None, user_key=deepblue_USER_KEY):
    return self.server.set_biosource_parent(parent=None, child=None, user_key=deepblue_USER_KEY)

  def set_biosource_synonym(self, biosource=None, synonym_name=None, user_key=deepblue_USER_KEY):
    return self.server.set_biosource_synonym(biosource=None, synonym_name=None, user_key=deepblue_USER_KEY)

  def set_project_public(self, project=None, set=None, user_key=deepblue_USER_KEY):
    return self.server.set_project_public(project=None, set=None, user_key=deepblue_USER_KEY)

  def tiling_regions(self, size=None, genome=None, chromosome=None, user_key=deepblue_USER_KEY):
    return self.server.tiling_regions(size=None, genome=None, chromosome=None, user_key=deepblue_USER_KEY)

  def upload_chromosome(self, genome=None, chromosome=None, data=None, user_key=deepblue_USER_KEY):
    return self.server.upload_chromosome(genome=None, chromosome=None, data=None, user_key=deepblue_USER_KEY)


if __name__ == "__main__":
  server = DeepBlueClient()
  print server.echo(deepblue_USER_KEY)