
#include <fstream>
#include <string>
#include <iostream>
#include <cmath>
#include <sstream>
#include <vector>
#include "../src/system/utils/microlzw.h"
#include "../src/system/utils/microlzw.cpp"

using namespace std;


int main(int argc, char **argv)
{
  string sifile(argv[1]);
  string sofile(argv[2]);
  ifstream ifile(sifile.c_str());
  ofstream ofile(sofile.c_str());
  string buf;
  getline(ifile, buf);  // struct ...
  ofile << buf << endl;
  getline(ifile, buf);  // width
  ofile << buf << endl;
  getline(ifile, buf);  // height
  ofile << buf << endl;
  getline(ifile, buf);  // rgbData
  if(buf == "      static constexpr bool lzwCompress = false;")
  {
    getline(ifile, buf);  // lzwCompressSize
    getline(ifile, buf);  // rgbData
  }
  ofile << "      static constexpr bool lzwCompress = true;\n";
  ofile << buf << endl;
  bool conv_ended = false;
  vector<uint32_t> rgbbuf;
  while(!ifile.eof())
  {
    getline(ifile, buf);  // width
    if(!conv_ended)
    {
      if(buf.substr(buf.length() - 2, 2) == "};")
        conv_ended = true;
      else
      {
        int n = ceil(float(buf.length()) / 10);
        for(int j=0; j<n; ++j)
        {
          uint32_t rgb;
          stringstream s(buf.substr(j * 10, 8));
          s >> hex >> rgb;
          rgbbuf.push_back(rgb);
        }
      }
    }
    // ofile << buf << endl;
  }
  cout << "in len: " << rgbbuf.size() << endl;
  vector<uint32_t> swapped(rgbbuf.size());
  for(size_t i=0; i<rgbbuf.size(); ++i)
  {
    uint32_t rgb = rgbbuf[i];
//     swapped[i] = ((rgb & 0xff0000) << 8) | ((rgb & 0xff000000) >> 8) | ((rgb & 0xff) << 8) | ((rgb & 0xff00) >> 8);
    swapped[i] = rgb; //  + 0x01010101;
  }
  char *ibuf = reinterpret_cast<char *>(&swapped[0]);
  int compressed[rgbbuf.size()];
  size_t comp_size = 0;
  const int dict_size = 4096;
  mlzw_compress(ibuf, compressed, rgbbuf.size() * 4, &comp_size, dict_size);
  cout << "compressed size: " << comp_size << endl;
  bool newLine = true;
  unsigned lcount = 0;
  for(size_t i=0; i<comp_size; ++i)
  {
    if(newLine)
    {
      ofile << "        ";
      newLine = false;
    }
    ofile << "0x" << hex << compressed[i];
    if(i<comp_size - 1)
      ofile << ",";
    else
    {
      ofile << "\n";
      break;
    }
    if(++lcount == 16)
    {
      ofile << "\n";
      newLine = true;
      lcount = 0;
    }
    else
      ofile << " ";
  }
  ofile << "      };\n";
  ofile << "      static constexpr uint32_t lzwCompressSize = " << dec
        << comp_size << ";\n";
  ofile << "    };\n";

  // test uncompress
  vector<uint32_t> rgbbuf_unc(rgbbuf.size());
  mlzw_decompress(reinterpret_cast<const int *>(compressed),
                  comp_size,
                  reinterpret_cast<char *>(&rgbbuf_unc[0]),
                  4096);
  for(size_t i=0; i<rgbbuf.size(); ++i)
    if(rgbbuf[i] != rgbbuf_unc[i])
      cout << dec << i << ": " << hex << rgbbuf[i] << " != " << rgbbuf_unc[i] << endl;
}
