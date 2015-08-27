import xmlrpclib


class EpidbClient:
 # def __init__(self, address="deepblue.mpi-inf.mpg.de/xmlrpc", port=None):
    def __init__(self, address="localhost", port=31415):
        if port is not None:
            url = "http://%s:%d" % (address, port)
        else:
            url = "http://%s" % (address)
        self.server = xmlrpclib.Server(url, encoding='UTF-8',
                                       allow_none=True, verbose=False)

## Status command
    def echo(self, user_key):
        return self.server.echo(user_key)

    def get_state(self, vocab, key):
        return self.server.get_state(vocab, key)

    def test_types(self, *args):
        return self.server.test_types(*args)

    def commands(self):
        return self.server.commands()

    def remove(self, _id, user_key):
      return self.server.remove(_id, user_key)

    def clone_dataset(self, dataset_id, new_name, new_epigenetic_mark, new_sample, new_technique, new_project, description, format, extra_metadata, user_key):
      return self.server.clone_dataset(dataset_id, new_name, new_epigenetic_mark, new_sample, new_technique, new_project, description, format, extra_metadata, user_key)

    def change_extra_metadata(self, _id, key, value, user_key):
      return self.server.change_extra_metadata(_id, key, value, user_key)


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
        return self.server.upload_chromosome(genome, chromosome,
                                             sequence, user_key)

  ## Bio Sources
    def add_biosource(self, name, description, extra_metadata, user_key):
        return self.server.add_biosource(name, description,
                                          extra_metadata, user_key)

    def list_biosources(self, extra_metadata, user_key):
        return self.server.list_biosources(extra_metadata, user_key)

    def list_similar_biosources(self, biosource_name, user_key):
        return self.server.list_similar_biosources(biosource_name, user_key)

    def is_biosource(self, biosource_name, user_key):
        return self.server.is_biosource(biosource_name, user_key)

  ## Samples
    def add_sample(self, biosource_name, fields, user_key):
        return self.server.add_sample(biosource_name, fields, user_key)

    def add_sample_from_gsm(self, biosource, gsm_id, user_key):
      return self.server.add_sample_from_gsm(biosource, gsm_id, user_key)

    def list_samples(self, biosource, metadata, user_key):
        return self.server.list_samples(biosource, metadata, user_key)

  ## Epigenetic Marks
    def add_epigenetic_mark(self, name, description, user_key):
        return self.server.add_epigenetic_mark(name, description, user_key)

    def list_epigenetic_marks(self, user_key):
        return self.server.list_epigenetic_marks(user_key)

    def list_similar_epigenetic_marks(self, term_name, user_key):
        return self.server.list_similar_epigenetic_marks(term_name, user_key)

  ## Column types
    def create_column_type_simple(self, name, description,
                                  type_, user_key):
        return self.server.create_column_type_simple(name, description,
                                                     type_, user_key)

    def create_column_type_range(self, name, description,
                                 minimum, maximum, user_key):
        return self.server.create_column_type_range(name, description,
                                                    minimum, maximum, user_key)

    def create_column_type_category(self, name, description,
                                    items, user_key):
        return self.server.create_column_type_category(name, description,
                                                       items, user_key)

    def create_column_type_calculated(self, name, description, code,
                                      user_key):
        return self.server.create_column_type_calculated(name, description,
                                                       code,
                                                       user_key)

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

    def set_project_public(self, project, set, user_key):
        return self.server.set_project_public(project, set, user_key)

  ## Bio Source names
    def set_biosource_synonym(self, biosource_name, synonym_name, user_key):
        return self.server.set_biosource_synonym(biosource_name,
                                                  synonym_name, user_key)

    def get_biosource_synonyms(self, biosource_name, user_key):
        return self.server.get_biosource_synonyms(biosource_name, user_key)

    def set_biosource_parent(self, biosource_name_more_embracing,
                             biosource_name_less_embracing, user_key):
        return self.server.set_biosource_parent(biosource_name_more_embracing,
                                                biosource_name_less_embracing,
                                                user_key)

    def get_biosource_children(self, biosource_name, user_key):
        return self.server.get_biosource_children(biosource_name, user_key)

    def get_biosource_parents(self, biosource_name, user_key):
        return self.server.get_biosource_parents(biosource_name, user_key)

    def get_biosource_related(self, biosource_name, user_key):
        return self.server.get_biosource_related(biosource_name, user_key)

  ## Expermiments
    def add_experiment(self, name, genome, epigenetic_mark, sample, technique,
                       project, description, data, frmt, extra_metadata,
                       user_key):
        return self.server.add_experiment(name, genome, epigenetic_mark,
                                          sample, technique, project,
                                          description, data,
                                          frmt, extra_metadata, user_key)

    def get_experiments_by_query(self, qid, user_key):
        return self.server.get_experiments_by_query(qid, user_key)

    def list_experiments(self, genome, epigenetic_mark, sample, technique,
                         project, user_key):
        return self.server.list_experiments(genome, epigenetic_mark, sample,
                                            technique, project, user_key)

    def list_similar_experiments(self, name, genome, user_key):
        return self.server.list_similar_experiments(name, genome, user_key)

    def list_recent_experiments(self, days, genome, epigenetic_mark, sample,
                                technique, project, user_key):
        return self.server.list_recent_experiments(days, genome,
                                                   epigenetic_mark, sample,
                                                   technique, project,
                                                   user_key)

    ## Annotations
    def add_annotation(self, name, genome, description, data, frmt,
                       extra_metadata, user_key):
        return self.server.add_annotation(name, genome, description,
                                          data, frmt, extra_metadata,
                                          user_key)

    def list_annotations(self, genome, user_key):
        return self.server.list_annotations(genome, user_key)

    def find_pattern(self, pattern, genome, overlap, user_key):
        return self.server.find_pattern(pattern, genome, overlap, user_key)

    ## Operations
    def select_regions(self, experiment_name, genome, epigenetic_mark,
                       sample, technique, project, chromosome, start, end,
                       user):
        return self.server.select_regions(experiment_name, genome,
                                          epigenetic_mark, sample, technique,
                                          project, chromosome, start, end,
                                          user)

    def select_annotations(self, annotations_name, genome, chromosomes,
                           start, end, user):
        return self.server.select_annotations(annotations_name, genome,
                                              chromosomes, start, end, user)

    def intersection(self, query_a_id, query_b_id, user_key):
        return self.server.intersection(query_a_id, query_b_id, user_key)

    def aggregate(self, regions_data_id, regions_range_id, field, user_key):
        return self.server.aggregate(regions_data_id,
                                     regions_range_id, field, user_key)

    def merge_queries(self, query_a_id, query_b_id, user_key):
        return self.server.merge_queries(query_a_id, query_b_id, user_key)

    def filter_regions(self, query_id, field, operation, value,
                       type_, user_key):
        return self.server.filter_regions(query_id, field, operation,
                                          value, type_,  user_key)

    def input_regions(self, genome, regions, user_key):
        return self.server.input_regions(genome, regions, user_key)

    def tiling_regions(self, size, genome, chromosomes, user_key):
        return self.server.tiling_regions(size, genome, chromosomes, user_key)

    def count_regions(self, query_id, user_key):
        return self.server.count_regions(query_id, user_key)

    def get_regions(self, query_id, format, user_key):
        return self.server.get_regions(query_id, format, user_key)

    def score_matrix(self, experiments, operation, regions_id, user_key):
        return self.server.score_matrix(experiments, operation, regions_id, user_key)

## Requests
    def list_requests(self, status, user_key):
        return self.server.list_requests(status, user_key)

    def get_request_data(self, request_id, user_key):
        return self.server.get_request_data(request_id, user_key)

## Admin Operations

    def init_system(self, name, email, institution):
        return self.server.init_system(name, email, institution)

    def add_user(self, name, email, institution, admin_key):
        return self.server.add_user(name, email, institution, admin_key)

    def list_users(self, admin_key):
        return self.server.list_users(admin_key)

    def user_auth(self, email, password):
        return self.server.user_auth(email, password)

    def modify_user(self, field, value, key):
        return self.server.modify_user(field, value, key)

    def modify_user_admin(self, user_key, field, value, admin_key):
        return self.server.modify_user_admin(user_key, field, value, admin_key)
