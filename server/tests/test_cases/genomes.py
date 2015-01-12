import helpers

from client import EpidbClient


class TestGenomeCommands(helpers.TestCase):


  def test_genome_hg19(self):
    epidb = EpidbClient()
    self.init(epidb)

    genome_info = None
    with open("data/genomes/hg19", 'r') as f:
      genome_info = f.read().replace(",", "")

    res = epidb.add_genome("hg19", "Human genome 19", genome_info, self.admin_key)
    self.assertSuccess(res)

    (res, genomes) = epidb.list_genomes(self.admin_key)
    self.assertEqual(len(genomes), 1)
    self.assertEqual(genomes[0][1], "hg19")

  def test_chromosomes(self):
    epidb = EpidbClient()
    self.init(epidb)

    genome_info = None
    with open("data/genomes/hg19", 'r') as f:
        genome_info = f.read().replace(",", "")

    res = epidb.add_genome("hg19", "Human genome 19", genome_info, self.admin_key)
    self.assertSuccess(res)
    (r, chroms) = epidb.chromosomes("hg19", self.admin_key)
    expected = [['chr1', 249250621], ['chr10', 135534747], ['chr11', 135006516], ['chr11_gl000202_random', 40103], ['chr12', 133851895], ['chr13', 115169878], ['chr14', 107349540], ['chr15', 102531392], ['chr16', 90354753], ['chr17', 81195210], ['chr17_ctg5_hap1', 1680828], ['chr17_gl000203_random', 37498], ['chr17_gl000204_random', 81310], ['chr17_gl000205_random', 174588], ['chr17_gl000206_random', 41001], ['chr18', 78077248], ['chr18_gl000207_random', 4262], ['chr19', 59128983], ['chr19_gl000208_random', 92689], ['chr19_gl000209_random', 159169], ['chr1_gl000191_random', 106433], ['chr1_gl000192_random', 547496], ['chr2', 243199373], ['chr20', 63025520], ['chr21', 48129895], ['chr21_gl000210_random', 27682], ['chr22', 51304566], ['chr3', 198022430], ['chr4', 191154276], ['chr4_ctg9_hap1', 590426], ['chr4_gl000193_random', 189789], ['chr4_gl000194_random', 191469], ['chr5', 180915260], ['chr6', 171115067], ['chr6_apd_hap1', 4622290], ['chr6_cox_hap2', 4795371], ['chr6_dbb_hap3', 4610396], ['chr6_mann_hap4', 4683263], ['chr6_mcf_hap5', 4833398], ['chr6_qbl_hap6', 4611984], ['chr6_ssto_hap7', 4928567], ['chr7', 159138663], ['chr7_gl000195_random', 182896], ['chr8', 146364022], ['chr8_gl000196_random', 38914], ['chr8_gl000197_random', 37175], ['chr9', 141213431], ['chr9_gl000198_random', 90085], ['chr9_gl000199_random', 169874], ['chr9_gl000200_random', 187035], ['chr9_gl000201_random', 36148], ['chrM', 16571], ['chrUn_gl000211', 166566], ['chrUn_gl000212', 186858], ['chrUn_gl000213', 164239], ['chrUn_gl000214', 137718], ['chrUn_gl000215', 172545], ['chrUn_gl000216', 172294], ['chrUn_gl000217', 172149], ['chrUn_gl000218', 161147], ['chrUn_gl000219', 179198], ['chrUn_gl000220', 161802], ['chrUn_gl000221', 155397], ['chrUn_gl000222', 186861], ['chrUn_gl000223', 180455], ['chrUn_gl000224', 179693], ['chrUn_gl000225', 211173], ['chrUn_gl000226', 15008], ['chrUn_gl000227', 128374], ['chrUn_gl000228', 129120], ['chrUn_gl000229', 19913], ['chrUn_gl000230', 43691], ['chrUn_gl000231', 27386], ['chrUn_gl000232', 40652], ['chrUn_gl000233', 45941], ['chrUn_gl000234', 40531], ['chrUn_gl000235', 34474], ['chrUn_gl000236', 41934], ['chrUn_gl000237', 45867], ['chrUn_gl000238', 39939], ['chrUn_gl000239', 33824], ['chrUn_gl000240', 41933], ['chrUn_gl000241', 42152], ['chrUn_gl000242', 43523], ['chrUn_gl000243', 43341], ['chrUn_gl000244', 39929], ['chrUn_gl000245', 36651], ['chrUn_gl000246', 38154], ['chrUn_gl000247', 36422], ['chrUn_gl000248', 39786], ['chrUn_gl000249', 38502], ['chrX', 155270560], ['chrY', 59373566]]
    self.assertEqual(chroms, expected)

    hg18_genome_info = None
    with open("data/genomes/hg18", 'r') as f:
        hg18_genome_info = f.read().replace(",", "")
    res = epidb.add_genome("hg18", "Human genome 18", hg18_genome_info, self.admin_key)
    self.assertSuccess(res)
    (r, chroms) = epidb.chromosomes("hg18", self.admin_key)
    expected = [['chr1', 247249719], ['chr10', 135374737], ['chr10_random', 113275], ['chr11', 134452384], ['chr11_random', 215294], ['chr12', 132349534], ['chr13', 114142980], ['chr13_random', 186858], ['chr14', 106368585], ['chr15', 100338915], ['chr15_random', 784346], ['chr16', 88827254], ['chr16_random', 105485], ['chr17', 78774742], ['chr17_random', 2617613], ['chr18', 76117153], ['chr18_random', 4262], ['chr19', 63811651], ['chr19_random', 301858], ['chr1_random', 1663265], ['chr2', 242951149], ['chr20', 62435964], ['chr21', 46944323], ['chr21_random', 1679693], ['chr22', 49691432], ['chr22_h2_hap1', 63661], ['chr22_random', 257318], ['chr2_random', 185571], ['chr3', 199501827], ['chr3_random', 749256], ['chr4', 191273063], ['chr4_random', 842648], ['chr5', 180857866], ['chr5_h2_hap1', 1794870], ['chr5_random', 143687], ['chr6', 170899992], ['chr6_cox_hap1', 4731698], ['chr6_qbl_hap2', 4565931], ['chr6_random', 1875562], ['chr7', 158821424], ['chr7_random', 549659], ['chr8', 146274826], ['chr8_random', 943810], ['chr9', 140273252], ['chr9_random', 1146434], ['chrM', 16571], ['chrX', 154913754], ['chrX_random', 1719168], ['chrY', 57772954]]

    self.assertEqual(chroms, expected)

    mm9_genome_info = None
    with open("data/genomes/mm9", 'r') as f:
        mm9_genome_info = f.read().replace(",", "")
    res = epidb.add_genome("mm9", "Mouse genome 9", mm9_genome_info, self.admin_key)
    self.assertSuccess(res)
    (r, chroms) = epidb.chromosomes("mm9", self.admin_key)
    expected = [['chr1', 197195432], ['chr10', 129993255], ['chr11', 121843856], ['chr12', 121257530], ['chr13', 120284312], ['chr13_random', 400311], ['chr14', 125194864], ['chr15', 103494974], ['chr16', 98319150], ['chr16_random', 3994], ['chr17', 95272651], ['chr17_random', 628739], ['chr18', 90772031], ['chr19', 61342430], ['chr1_random', 1231697], ['chr2', 181748087], ['chr3', 159599783], ['chr3_random', 41899], ['chr4', 155630120], ['chr4_random', 160594], ['chr5', 152537259], ['chr5_random', 357350], ['chr6', 149517037], ['chr7', 152524553], ['chr7_random', 362490], ['chr8', 131738871], ['chr8_random', 849593], ['chr9', 124076172], ['chr9_random', 449403], ['chrM', 16299], ['chrUn_random', 5900358], ['chrX', 166650296], ['chrX_random', 1785075], ['chrY', 15902555], ['chrY_random', 58682461]]
    self.assertEqual(chroms, expected)

  def test_genome_empty(self):
    epidb = EpidbClient()
    self.init(epidb)

    hg18_genome_info = None
    with open("data/genomes/hg18", 'r') as f:
        hg18_genome_info = f.read().replace(",", "")

    mm9_genome_info = None
    with open("data/genomes/mm9", 'r') as f:
        mm9_genome_info = f.read().replace(",", "")

    res = epidb.add_genome("hg18", "Human genome 18", hg18_genome_info, self.admin_key)
    self.assertSuccess(res)
    res = epidb.add_genome("mm9", "Mouse genome 9", mm9_genome_info, self.admin_key)
    self.assertSuccess(res)

    (res, genomes) = epidb.list_genomes(self.admin_key)
    self.assertEqual(len(genomes), 2)

    genome_names = [x[1] for x in genomes]
    self.assertTrue("hg18" in genome_names)
    self.assertTrue("mm9" in genome_names)

  def test_genome_info(self):
    epidb = EpidbClient()
    self.init(epidb)

    hg18_genome_info = None
    with open("data/genomes/hg18", 'r') as f:
        hg18_genome_info = f.read().replace(",", "")

    res = epidb.add_genome("hg18", "Human genome 18", hg18_genome_info, self.admin_key)
    self.assertSuccess(res)

    info = [{'chromosomes': '{ name: "chr1", size: 247249719 },{ name: "chr1_random", size: 1663265 },{ name: "chr10", size: 135374737 },{ name: "chr10_random", size: 113275 },{ name: "chr11", size: 134452384 },{ name: "chr11_random", size: 215294 },{ name: "chr12", size: 132349534 },{ name: "chr13", size: 114142980 },{ name: "chr13_random", size: 186858 },{ name: "chr14", size: 106368585 },{ name: "chr15", size: 100338915 },{ name: "chr15_random", size: 784346 },{ name: "chr16", size: 88827254 },{ name: "chr16_random", size: 105485 },{ name: "chr17", size: 78774742 },{ name: "chr17_random", size: 2617613 },{ name: "chr18", size: 76117153 },{ name: "chr18_random", size: 4262 },{ name: "chr19", size: 63811651 },{ name: "chr19_random", size: 301858 },{ name: "chr2", size: 242951149 },{ name: "chr2_random", size: 185571 },{ name: "chr20", size: 62435964 },{ name: "chr21", size: 46944323 },{ name: "chr21_random", size: 1679693 },{ name: "chr22", size: 49691432 },{ name: "chr22_random", size: 257318 },{ name: "chr22_h2_hap1", size: 63661 },{ name: "chr3", size: 199501827 },{ name: "chr3_random", size: 749256 },{ name: "chr4", size: 191273063 },{ name: "chr4_random", size: 842648 },{ name: "chr5", size: 180857866 },{ name: "chr5_random", size: 143687 },{ name: "chr5_h2_hap1", size: 1794870 },{ name: "chr6", size: 170899992 },{ name: "chr6_random", size: 1875562 },{ name: "chr6_cox_hap1", size: 4731698 },{ name: "chr6_qbl_hap2", size: 4565931 },{ name: "chr7", size: 158821424 },{ name: "chr7_random", size: 549659 },{ name: "chr8", size: 146274826 },{ name: "chr8_random", size: 943810 },{ name: "chr9", size: 140273252 },{ name: "chr9_random", size: 1146434 },{ name: "chrM", size: 16571 },{ name: "chrX", size: 154913754 },{ name: "chrX_random", size: 1719168 },{ name: "chrY", size: 57772954 }', 'description': 'Human genome 18', 'user': 'test_admin', '_id': 'g1', 'type': 'genome', 'name': 'hg18'}]

    s, info_answer = epidb.info("g1", self.admin_key)

    self.assertEqual(info_answer, info)

  def test_genome_duplicate(self):
    epidb = EpidbClient()
    self.init(epidb)

    hg18_genome_info = None
    with open("data/genomes/hg18", 'r') as f:
        hg18_genome_info = f.read().replace(",", "")

    res = epidb.add_genome("hg18", "Human genome 18", hg18_genome_info, self.admin_key)
    self.assertSuccess(res)
    res = epidb.add_genome("hg18", "Human genome 18 #2", hg18_genome_info, self.admin_key)
    self.assertFailure(res)