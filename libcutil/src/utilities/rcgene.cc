#include "cutil.hpp"
#include "mxml.hpp"
#include <stdio.h>

using namespace utils;
using namespace model;

int main(int argc, char **argv)
{
  CmdeLine cmdeline(argc, argv);
  utils::init(cmdeline, "libcutil", "rcgene");

  MXml mx;

  if(mx.from_file(cmdeline.get_option("-i").c_str()))
  {
    printf("rggene: no version.xml file found.\n");
    return -1;
  }
  
  int maj, min, build;
  std::string company, copyright, file_desc, original_file;
  maj = mx.get_attribute("maj").to_int();
  min = mx.get_attribute("min").to_int();
  build = mx.get_attribute("build").to_int();
  company = mx.get_attribute("company").to_string();
  copyright = mx.get_attribute("copyright").to_string();
  file_desc = mx.get_attribute("file_desc").to_string();
  original_file = mx.get_attribute("original_file").to_string();
  
  printf("rcgene: revision = %d.%d...\n", maj, min);

  std::string name = cmdeline.get_option("-n", "version");

  std::string orc = cmdeline.get_option("-o") + PATH_SEP + name + ".rc";

  FILE *of = fopen(orc.c_str(), "wt");

  fprintf(of, 
	  "1 VERSIONINFO\n"
	  "FILEVERSION 0,%d,%d,%d\n"
	  "PRODUCTVERSION 0,%d,%d,%d\n"
	  "FILEFLAGSMASK 0x17L\n"
	  "FILEOS 0x4L\n"
	  "FILETYPE 0x01\n"
	  "FILESUBTYPE 0x0L\n"
	  "BEGIN\n"
	  " BLOCK \"StringFileInfo\"\n"
	  " BEGIN\n"
	  " BLOCK \"040c04b0\"\n"
	  "  BEGIN\n"
	  "  VALUE \"CompanyName\", \"%s\"\n"
	  "  VALUE \"FileDescription\", \"%s\"\n"
          "  VALUE \"FileVersion\", \"0, %d, %d, %d\"\n"
          "  VALUE \"InternalName\", \"%s\"\n"
          "  VALUE \"LegalCopyright\", \"%s\"\n"
          "  VALUE \"OriginalFilename\", \" %s\"\n"
          "  VALUE \"ProductName\", \"%s\"\n"
          "  VALUE \"ProductVersion\", \"0, %d, %d, %d\"\n"
          "  END\n"
          " END\n"
          " BLOCK \"VarFileInfo\"\n"
	  "  BEGIN\n"
	  "  VALUE \"Translation\", 0x40c, 1200\n"
	  "  END\n"
	  "END\n", maj, min, build, maj, min, build, company.c_str(), file_desc.c_str(), maj, min, build, file_desc.c_str(), 
	  copyright.c_str(), original_file.c_str(), original_file.c_str(), maj, min, build);


  fclose(of);

  std::string oh = cmdeline.get_option("-o") + PATH_SEP + name + ".h";
  of = fopen(oh.c_str(), "wt");
  fprintf(of, "#define VERSION_MAJ %d\n", maj);
  fprintf(of, "#define VERSION_MIN %d\n", min);
  fclose(of);
  
  return 0;
}
