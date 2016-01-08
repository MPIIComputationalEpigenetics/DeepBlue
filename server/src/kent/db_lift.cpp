
class ChainHash { };
class Chain { };
class ChromMap { };

/* Read map file into hashes. */
bool readLiftOverMap(std::istream &input,  ChainHash& chain, std::string &msg)
{
  ChromMap *map;
  int chainCount = 0;

  while (input) {
    Chain* chain = chainRead(input);
  }


  while ((chain = chainRead(lf)) != NULL) {
    if ((map = hashFindVal(chainHash, chain->tName)) == NULL) {
      AllocVar(map);
      map->bk = binKeeperNew(0, chain->tSize);
      hashAddSaveName(chainHash, chain->tName, map, &map->name);
    }
    binKeeperAdd(map->bk, chain->tStart, chain->tEnd, chain);
    ++chainCount;
  }
}

bool chain_read(std::istream &input, Chain* chain, std::string& msg)
{
  if (!chainReadChainLine(input, chain, msg)) {
    return false;
  }

  return chainReadBlocks(input, chain, msg);
}

bool chainReadChainLine(std::istream& input, Chain* chain, std::string& msg)
{
  std::vector<std::string> row;
  boost::split(sample_chars_strs, value, boost::is_any_of(" "));

  if (row[0] != "chain") {
    msg = "expecting 'chain' line...";
    return false;
  }

  if (!utils::string_to_score(row[1], chain->score)) {
    msg = "Parsing error: " + row[1];
    return false;
  }
  chain->tName = row[2];
  if (!utils::string_to_integer(row[3], chain->tSize)) {
    msg = "Parsing error: " + row[3];
    return false;
  }

  if (row.size() >= 13) {
    if (!utils::string_to_integer(row[12], chain->id)) {
      msg = "Parsing error: " + row[12];
      return false;
    }
  } else {
    chainIdNext(chain);
  }

  if (!utils::string_to_integer(row[5], chain->tStart)) {
    msg = "Parsing error: " + row[5];
    return false;
  }

  if (!utils::string_to_integer(row[6], chain->tEnd)) {
    msg = "Parsing error: " + row[5];
    return false;
  }

  chain->qName = row[7];

  if (!utils::string_to_integer(row[8], chain->qSize)) {
    msg = "Parsing error: " + row[8];
    return false;
  }

  if (!utils::string_to_integer(row[10], chain->qStart)) {
    msg = "Parsing error: " + row[10];
    return false;
  }
  if (!utils::string_to_integer(row[11], chain->qEnd)) {
    msg = "Parsing error: " + row[11];
    return false;
  }


  chain->qStrand = row[9][0];
  if (chain->qStart >= chain->qEnd || chain->tStart >= chain->tEnd) {
    msg = "End before start line %d of %s"; //, lf->lineIx, lf->fileName);
    return false;
  }

  if (chain->qStart < 0 || chain->tStart < 0) {
    msg = "Start before zero line %d of %s"; //, lf->lineIx, lf->fileName);
    return false;
  }

  if (chain->qEnd > chain->qSize || chain->tEnd > chain->tSize) {
    msg = "Past end of sequence line %d of %s"; // lf->lineIx, lf->fileName);
    return false;
  }
  return true;
}
