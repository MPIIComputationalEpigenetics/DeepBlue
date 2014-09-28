/**

albrecht@guest-7:~/mpi/DeepBlue/server/src|master⚡
⇒  clang++ test_compress_lzo.cpp ../third_party/minilzo/minilzo.c ../third_party/drivers/macosx/lib/libmongoclient.a -I../third_party/minilzo/ -I../third_party/drivers/macosx/include/ -I/opt/boost/include  /opt/boost/lib/libboost_filesystem.a /opt/boost/lib/libboost_system.a /opt/boost/lib/libboost_thread.a  -O3 -Ofast extras/compress.cpp -I. /opt/boost/lib/libboost_log.a
clang: warning: treating 'c' input as 'c++' when in C++ mode, this behavior is deprecated
albrecht@guest-7:~/mpi/DeepBlue/server/src|master⚡
⇒  ./a.out
15888895
8
vai entrar
compressed 15888895 bytes into 6227956 bytes
6228046
{ S: 0, E: 1000, S0: 10, MD5: "522ad5072c69a8b2a121ec054e790884", SIZE: 15888895, data: BinData }
-------------
6227956
6227956
-------------
r == LZO_E_OK
15888895
15888895

./a.out  0.41s user 0.03s system 98% cpu 0.444 total
*/


#include <iostream>
#include <string>
#include <ctime>

#include <minilzo.h>

#include <boost/shared_ptr.hpp>

#include <mongo/bson/bson.h>

#include "extras/compress.hpp"

#include "datatypes/datatypes.hpp"

//bool compress()

int main()
{

  epidb::compress::init();

  mongo::BSONObjBuilder builder;
  builder.append("S", 0);
  builder.append("E", 1000);
  builder.append("S0", 10);

  mongo::BSONArrayBuilder ab;
  for (long long s(0) ; s < 1000; s++) {
    ab.append(s * 100);
    ab.append((s * 100) + 100);
    ab.append(s * 1234.456);
  }

  mongo::BSONObj o = ab.arr();
  std::string md5 = o.md5();

  builder.append("MD5", md5);
  builder.append("SIZE", (int) o.objsize());

  mongo::BSONObjBuilder builder2;
  builder2.append("S", 0);
  builder2.append("E", 1000);
  builder2.append("S0", 10);
  builder2.append("data", o);
  builder2.append("MD5", md5);
  builder2.append("SIZE", (int) o.objsize());
  mongo::BSONObj o2 = builder2.obj();

  std::cerr << "not compressed: " << o2.objsize() << std::endl;

  boost::shared_ptr<char> out;
  size_t out_len(0);
  const long double sysTime = clock();
  bool c = false;
  out = epidb::compress::compress(o.objdata(), o.objsize(), out_len, c);

  std::cerr << c << std::endl;
  std::cerr << "compress in " << (( ((float) clock()) - sysTime) / CLOCKS_PER_SEC) << std::endl;

  builder.appendBinData("data", out_len, mongo::BinDataGeneral, (void *) out.get());

  mongo::BSONObj obj = builder.obj();
  std::cerr << obj.objsize() << std::endl;
  std::cerr << obj.toString() << std::endl;

  int i_size;
  int prev_size = obj["SIZE"].Int();
  const char *data = obj["data"].binData(i_size);
  lzo_uint lzo_size = i_size;

  std::cerr << "-------------" << std::endl;
  std::cerr << i_size << std::endl;
  std::cerr << lzo_size << std::endl;
  std::cerr << "-------------" << std::endl;

  clock_t dsysTime = clock();
  char* uncompressed;
  size_t uncompressed_size;
  uncompressed = epidb::compress::decompress(data, lzo_size, prev_size, uncompressed_size);
  std::cerr << "decompress in " << (( ((float)  clock()) - dsysTime) / CLOCKS_PER_SEC) << std::endl;

  mongo::BSONObj new_obj(uncompressed);

  std::cerr << new_obj.toString() << std::endl;

  const size_t size = 40000;
  std::vector<int> v;

  for (int i = 0; i < size; i++) {
    v.push_back(i * 10);
    v.push_back((i * 10)+10);
    v.push_back(i*12345678901234567);
  }

  size_t v_len = sizeof(int) * v.size();
  size_t v_out_len;
  boost::shared_ptr<char> out_2;
  out_2 = epidb::compress::compress((char *) v.data(), v_len, v_out_len, c);
  std::cerr << c << std::endl;

  std::cerr << "v_len: " << v_len << " , v_out_len: " << v_out_len << std::endl;

}