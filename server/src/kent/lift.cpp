
typedef std::unordered_map<std::string, std::string> Chain;

struct chain
/* A chain of blocks.  Used for output of chainBlocks. */
{
  struct chain *next;	  	  /* Next in list. */
  struct cBlock *blockList;      /* List of blocks. */
  double score;	  	  /* Total score for chain. */
  char *tName;		  /* target name, allocated here. */
  int tSize;			  /* Overall size of target. */
  /* tStrand always + */
  int tStart, tEnd;		 /* Range covered in target. */
  char *qName;		  /* query name, allocated here. */
  int qSize;			  /* Overall size of query. */
  char qStrand;		  /* Query strand. */
  int qStart, qEnd;		 /* Range covered in query. */
  int id;			  /* ID of chain in file. */
} Chain;

static int nextId = 1;

void chainIdNext(struct chain *chain)
/* Add an id to a chain if it doesn't have one already */
{
	chain->id = nextId++;
}


struct chain *chainRead(struct lineFile *lf)
/* Read next chain from file.  Return NULL at EOF.
 * Note that chain block scores are not filled in by
 * this. */
{
struct chain *chain = chainReadChainLine(lf);
if (chain != NULL)
    chainReadBlocks(lf, chain);
return chain;
}


void readLiftOverMap(char *fileName, struct hash *chainHash)
/* Read map file into hashes. */
{

struct lineFile *lf;
struct netParsedUrl *npu;
if (udcIsLocal(fileName))
    lf = lineFileOpen(fileName, TRUE);
else
    lf = netHttpLineFileMayOpen(fileName, &npu);

struct chain *chain;
struct chromMap *map;
int chainCount = 0;

while ((chain = chainRead(lf)) != NULL)
    {
    if ((map = hashFindVal(chainHash, chain->tName)) == NULL)
	{
	AllocVar(map);
	map->bk = binKeeperNew(0, chain->tSize);
	hashAddSaveName(chainHash, chain->tName, map, &map->name);
	}
    binKeeperAdd(map->bk, chain->tStart, chain->tEnd, chain);
    ++chainCount;
    }
}


bool parse_chain(std::unique_ptr<std::istream> &&input, Chain& chain)
{


  strtk::for_each_line_conditional(*input_, [&](const std::string & line) -> bool {
  	actual_line_++;

  	if (line.empty() || line[0] == '#') {
    	return true;
    }

    std::string chain_str;
    std::shared_ptr<Chain> chain = std::shared_ptr<Chain>(new Chain());
    if (!strtk::parse(line, "\t ",chain_str, chain->score, chain->tName, chain->tSize)) {
		msg = "Failed to parse line : " + line_str() + " " + line;
		return false;
    }

    if (chain_str != "chain") {
    	msg = "error chain";
    	return false;
    }
  }
}

struct chain *chainReadChainLine(struct lineFile *lf)
/* Read line that starts with chain.  Allocate memory
 * and fill in values.  However don't read link lines. */
{
char *row[13];
int wordCount;
struct chain *chain;

wordCount = lineFileChop(lf, row);
if (wordCount == 0)
    return NULL;
if (wordCount < 12)
    errAbort("Expecting at least 12 words line %d of %s",
    	lf->lineIx, lf->fileName);
if (!sameString(row[0], "chain"))
    errAbort("Expecting 'chain' line %d of %s", lf->lineIx, lf->fileName);
AllocVar(chain);
chain->score = atof(row[1]);
chain->tName = cloneString(row[2]);
chain->tSize = lineFileNeedNum(lf, row, 3);
if (wordCount >= 13)
    chain->id = lineFileNeedNum(lf, row, 12);
else
    chainIdNext(chain);

/* skip tStrand for now, always implicitly + */
chain->tStart = lineFileNeedNum(lf, row, 5);
chain->tEnd = lineFileNeedNum(lf, row, 6);
chain->qName = cloneString(row[7]);
chain->qSize = lineFileNeedNum(lf, row, 8);
chain->qStrand = row[9][0];
chain->qStart = lineFileNeedNum(lf, row, 10);
chain->qEnd = lineFileNeedNum(lf, row, 11);
if (chain->qStart >= chain->qEnd || chain->tStart >= chain->tEnd)
    errAbort("End before start line %d of %s", lf->lineIx, lf->fileName);
if (chain->qStart < 0 || chain->tStart < 0)
    errAbort("Start before zero line %d of %s", lf->lineIx, lf->fileName);
if (chain->qEnd > chain->qSize || chain->tEnd > chain->tSize)
    errAbort("Past end of sequence line %d of %s", lf->lineIx, lf->fileName);
return chain;
}


void chainReadBlocks(struct lineFile *lf, struct chain *chain)
/* Read in chain blocks from file. */
{
char *row[3];
int q,t;

/* Now read in block list. */
q = chain->qStart;
t = chain->tStart;
for (;;)
    {
    int wordCount = lineFileChop(lf, row);
    int size = lineFileNeedNum(lf, row, 0);
    struct cBlock *b;
    AllocVar(b);
    slAddHead(&chain->blockList, b);
    b->qStart = q;
    b->tStart = t;
    q += size;
    t += size;
    b->qEnd = q;
    b->tEnd = t;
    if (wordCount == 1)
        break;
    else if (wordCount < 3)
        errAbort("Expecting 1 or 3 words line %d of %s\n",
		lf->lineIx, lf->fileName);
    t += lineFileNeedNum(lf, row, 1);
    q += lineFileNeedNum(lf, row, 2);
    }
if (q != chain->qEnd)
    errAbort("q end mismatch %d vs %d line %d of %s\n",
    	q, chain->qEnd, lf->lineIx, lf->fileName);
if (t != chain->tEnd)
    errAbort("t end mismatch %d vs %d line %d of %s\n",
    	t, chain->tEnd, lf->lineIx, lf->fileName);
slReverse(&chain->blockList);
}

void slReverse(void *listPt)
/* Reverse order of a list.
 * Usage:
 *    slReverse(&list);
 */
{
struct slList **ppt = (struct slList **)listPt;
struct slList *newList = NULL;
struct slList *el, *next;

next = *ppt;
while (next != NULL)
    {
    el = next;
    next = el->next;
    el->next = newList;
    newList = el;
    }
*ppt = newList;
}

struct chain *chain;
struct chromMap *map;
int chainCount = 0;

while ((chain = chainRead(lf)) != NULL)
{
  if ((map = hashFindVal(chainHash, chain->tName)) == NULL) {
    AllocVar(map);
    map->bk = binKeeperNew(0, chain->tSize);
    hashAddSaveName(chainHash, chain->tName, map, &map->name);
  }
  binKeeperAdd(map->bk, chain->tStart, chain->tEnd, chain);
  ++chainCount;
}
}