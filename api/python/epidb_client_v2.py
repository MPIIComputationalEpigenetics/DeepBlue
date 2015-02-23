import xmlrpclib

def key_required(func):
    def func_wrapper(self, *args, **kwargs):
        if self.key:
            return func(self, *args, **kwargs)
        else:
            raise AttributeError("To use this function a key is required. Set it using "
                                 "set_key or construct EpidbClient providing a key")
    return func_wrapper


class EpidbClient(object):
    """Conveniently access a DeepBlue server
    """
    def __init__(self, key=None, address="deepblue.mpi-inf.mpg.de/xmlrpc", port=None):
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

    def set_key(self, key):
        self.key = key

    # Status command
    def echo(self):
        return self.server.echo(self.key)

    def test_types(self, *args):
        return self.server.test_types(*args)

    def commands(self):
        return self.server.commands()

    @key_required
    def remove(self, _id):
        return self.server.remove(_id, self.key)

    @key_required
    def clone_dataset(self, dataset_id, new_name, new_epigenetic_mark, new_sample,
                      new_technique, new_project, description, _format, extra_metadata):
        return self.server.clone_dataset(dataset_id, new_name, new_epigenetic_mark,
                                         new_sample, new_technique, new_project, description,
                                         _format, extra_metadata, self.key)

    @key_required
    def change_extra_metadata(self, _id, key, value):
        return self.server.change_extra_metadata(_id, key, value, self.key)

    # Main Operations

    # General information
    @key_required
    def info(self, _id):
        return self.server.info(_id, self.key)

    @key_required
    def search(self, text, type_):
        return self.server.search(text, type_, self.key)

        # Genomes
    @key_required
    def add_genome(self, name, description, chromosomes):
        return self.server.add_genome(name, description, chromosomes, self.key)

    @key_required
    def list_genomes(self):
        return self.server.list_genomes(self.key)

    @key_required
    def list_similar_genomes(self, name):
        return self.server.list_similar_genomes(name, self.key)

    @key_required
    def chromosomes(self, genome):
        return self.server.chromosomes(genome, self.key)

    @key_required
    def upload_chromosome(self, genome, chromosome, sequence):
        return self.server.upload_chromosome(genome, chromosome,
                                             sequence, self.key)

        # Bio Sources
    @key_required
    def add_biosource(self, name, description, extra_metadata):
        return self.server.add_biosource(name, description,
                                         extra_metadata, self.key)

    @key_required
    def list_biosources(self):
        return self.server.list_biosources(self.key)

    @key_required
    def list_similar_biosources(self, biosource_name):
        return self.server.list_similar_biosources(biosource_name, self.key)

    @key_required
    def is_biosource(self, biosource_name):
        return self.server.is_biosource(biosource_name, self.key)

        # Samples
    @key_required
    def add_sample(self, biosource_name, fields):
        return self.server.add_sample(biosource_name, fields, self.key)

    @key_required
    def add_sample_from_gsm(self, biosource, gsm_id):
        return self.server.add_sample_from_gsm(biosource, gsm_id, self.key)

    @key_required
    def list_samples(self, biosource, metadata):
        return self.server.list_samples(biosource, metadata, self.key)

        # Epigenetic Marks
    @key_required
    def add_epigenetic_mark(self, name, description):
        return self.server.add_epigenetic_mark(name, description, self.key)

    @key_required
    def list_epigenetic_marks(self):
        return self.server.list_epigenetic_marks(self.key)

    @key_required
    def list_similar_epigenetic_marks(self, term_name):
        return self.server.list_similar_epigenetic_marks(term_name, self.key)

        # Column types
    @key_required
    def create_column_type_simple(self, name, description, type_):
        return self.server.create_column_type_simple(name, description,
                                                     type_, self.key)

    @key_required
    def create_column_type_range(self, name, description,
                                 minimum, maximum):
        return self.server.create_column_type_range(name, description,
                                                    minimum, maximum, self.key)

    @key_required
    def create_column_type_category(self, name, description,
                                    items):
        return self.server.create_column_type_category(name, description,
                                                       items, self.key)

    @key_required
    def create_column_type_calculated(self, name, description, code):
        return self.server.create_column_type_calculated(name, description, code, self.key)

    @key_required
    def list_column_types(self):
        return self.server.list_column_types(self.key)

        # Techniques
    @key_required
    def add_technique(self, name, description, metadata):
        return self.server.add_technique(name, description, metadata, self.key)

    @key_required
    def list_techniques(self):
        return self.server.list_techniques(self.key)

    @key_required
    def list_similar_techniques(self, name):
        return self.server.list_similar_techniques(name, self.key)

        # Projects
    @key_required
    def add_project(self, name, description):
        return self.server.add_project(name, description, self.key)

    @key_required
    def list_projects(self):
        return self.server.list_projects(self.key)

    @key_required
    def list_similar_projects(self, project_name):
        return self.server.list_similar_projects(project_name, self.key)

        # Bio Source names
    @key_required
    def set_biosource_synonym(self, biosource_name, synonym_name):
        return self.server.set_biosource_synonym(biosource_name,
                                                 synonym_name, self.key)

    @key_required
    def get_biosource_synonyms(self, biosource_name):
        return self.server.get_biosource_synonyms(biosource_name, self.key)

    @key_required
    def set_biosource_parent(self, biosource_name_more_embracing,
                             biosource_name_less_embracing):
        return self.server.set_biosource_parent(biosource_name_more_embracing,
                                                biosource_name_less_embracing,
                                                self.key)

    @key_required
    def get_biosource_children(self, biosource_name):
        return self.server.get_biosource_children(biosource_name, self.key)

    @key_required
    def get_biosource_parents(self, biosource_name):
        return self.server.get_biosource_parents(biosource_name, self.key)

    @key_required
    def get_biosource_related(self, biosource_name):
        return self.server.get_biosource_related(biosource_name, self.key)

        # Expermiments
    @key_required
    def add_experiment(self, name, genome, epigenetic_mark, sample, technique,
                       project, description, data, frmt, extra_metadata):
        return self.server.add_experiment(name, genome, epigenetic_mark,
                                          sample, technique, project,
                                          description, data,
                                          frmt, extra_metadata, self.key)

    @key_required
    def get_experiments_by_query(self, qid):
        return self.server.get_experiments_by_query(qid, self.key)

    @key_required
    def list_experiments(self, genome, epigenetic_mark, sample, technique,
                         project):
        return self.server.list_experiments(genome, epigenetic_mark, sample,
                                            technique, project, self.key)

    @key_required
    def list_similar_experiments(self, name, genome):
        return self.server.list_similar_experiments(name, genome, self.key)

    @key_required
    def list_recent_experiments(self, days, genome, epigenetic_mark, sample,
                                technique, project):
        return self.server.list_recent_experiments(days, genome,
                                                   epigenetic_mark, sample,
                                                   technique, project,
                                                   self.key)

    # Annotations
    @key_required
    def add_annotation(self, name, genome, description, data, frmt,
                       extra_metadata):
        return self.server.add_annotation(name, genome, description,
                                          data, frmt, extra_metadata,
                                          self.key)

    @key_required
    def list_annotations(self, genome):
        return self.server.list_annotations(genome, self.key)

    @key_required
    def find_pattern(self, pattern, genome, overlap):
        return self.server.find_pattern(pattern, genome, overlap, self.key)

    # Operations
    @key_required
    def select_regions(self, experiment_name, genome, epigenetic_mark,
                       sample, technique, project, chromosome, start, end):
        return self.server.select_regions(experiment_name, genome,
                                          epigenetic_mark, sample, technique,
                                          project, chromosome, start, end,
                                          self.key)

    @key_required
    def select_annotations(self, annotations_name, genome, chromosomes,
                           start, end):
        return self.server.select_annotations(annotations_name, genome,
                                              chromosomes, start, end, self.key)

    @key_required
    def intersection(self, query_a_id, query_b_id):
        return self.server.intersection(query_a_id, query_b_id, self.key)

    @key_required
    def aggregate(self, regions_data_id, regions_range_id, field):
        return self.server.aggregate(regions_data_id,
                                     regions_range_id, field, self.key)

    @key_required
    def merge_queries(self, query_a_id, query_b_id):
        return self.server.merge_queries(query_a_id, query_b_id, self.key)

    @key_required
    def filter_regions(self, query_id, field, operation, value,
                       type_):
        return self.server.filter_regions(query_id, field, operation,
                                          value, type_,  self.key)

    @key_required
    def tiling_regions(self, size, genome, chromosomes):
        return self.server.tiling_regions(size, genome, chromosomes, self.key)

    @key_required
    def count_regions(self, query_id):
        return self.server.count_regions(query_id, self.key)

    @key_required
    def get_regions(self, query_id, _format):
        return self.server.get_regions(query_id, _format, self.key)

    @key_required
    def score_matrix(self, experiments, operation, regions_id):
        return self.server.score_matrix(experiments, operation, regions_id, self.key)

    # Admin Operations

    def init_system(self, name, email, institution):
        print(self.key, self.server)
        print(self, name, email, institution)
        return self.server.init_system(name, email, institution)

    @key_required
    def add_user(self, name, email, institution):
        return self.server.add_user(name, email, institution, self.key)

    @key_required
    def list_users(self):
        return self.server.list_users(self.key)
