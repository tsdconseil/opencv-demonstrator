#include <sys/stat.h>

#include "cutil.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <ctime>
#include <sstream>
#include <unistd.h> // for unlink
#include <cmath>
#include <dirent.h>

#ifdef WIN
# include <windows.h>
# include <winbase.h>
# include <stdio.h>
# include <Shlobj.h>
#else
# include <sys/stat.h> 
# include <limits.h>
#endif
#include <string.h>
#include <malloc.h>
#include "modele.hpp"

using namespace std;

namespace utils
{

using namespace model;


#if 0
void VoidEventProvider::remove_all_listeners()
{
  functors.clear();
}

void VoidEventProvider::dispatch()
{
  std::vector<void *> copy;

  std::vector<VoidEventFunctor *> copy2;
  for(unsigned int i = 0; i < functors.size(); i++)
    copy2.push_back(functors[i]);

  for(unsigned int i = 0; i < copy2.size(); i++)
  {
    VoidEventFunctor *ef = copy2[i];
    ef->call();
  }
}
#endif

////////////////////////////////////////////////
//// CONSTANT DATA
////////////////////////////////////////////////
#define NLANG 5
static std::string lglist[NLANG] = 
  {"fr", "en", "de", "ru", "es"};

////////////////////////////////////////////////
//// LOCAL FUNCTIONS    
////////////////////////////////////////////////

static std::string get_self_path(const char *argv0);


////////////////////////////////////////////////
//// LOCAL DATA    
////////////////////////////////////////////////

class AppData
{
public:
  std::string nom_appli;
  std::string nom_projet;
  std::string dossier_executable;
  std::string chemin_donnees_fixes;
  std::string chemin_images;
};


static AppData appdata;
Localized::Language Localized::current_language = Localized::LANG_FR;


Section langue;


#ifdef WIN
static int vasprintf(char **sptr, const char *fmt, va_list argv )
{
  int wanted = vsnprintf( *sptr = nullptr, 0, fmt, argv );
  if( (wanted > 0) && ((*sptr = (char *) malloc( 1 + wanted )) != nullptr) )
    return vsprintf( *sptr, fmt, argv );
  return wanted;
}
#endif


#ifdef WIN
extern "C"
{
  char *strdup(const char *s)
  {
    char *res = (char *) malloc(strlen(s) + 1);
    strcpy(res, s);
    return res;
  }
}
#endif


static std::string get_self_path(const char *argv0)
{
# ifdef WIN
  char actualpath[300];
  HMODULE hmod = GetModuleHandle(NULL);//GetCurrentModule();
  if(hmod == NULL)
  {
    cerr << "get_self_path: Failed to retrieve module handle!" << endl;
  }
  GetModuleFileName(hmod,/*nullptr,*/ /*(LPWSTR)*/ actualpath, 300);
# else
  char actualpath[PATH_MAX + 1];
  if(realpath(argv0, actualpath) == nullptr)
  {
    perror("realpath.");
    return "";
  }
# endif
  return std::string(actualpath);
}

int files::copy_file(std::string target, std::string source)
{
# ifndef LINUX

  infos("Copie fichier [%s] <- [%s]...", target.c_str(), source.c_str());
  if(!(::CopyFile(source.c_str(), target.c_str(), false)))
  {
    int err = ::GetLastError();
    avertissement("Echec copyfile, code d'erreur = %d (0x%x).", err, err);
    return -1;
  }
  return 0;
# else
  return utils::proceed_syscmde("cp \"%s\" \"%s\"", source.c_str(), target.c_str());
# endif
}

int proceed_syscmde_bg(std::string cmde, ...)
{
  char *full_cmde;
  va_list ap;
  int result;

  va_start(ap, cmde);
  result = vasprintf(&full_cmde, cmde.c_str(), ap);
  va_end(ap);

  if(result == -1)
  {
    fprintf(stderr, "vasprintf failure: %d.\n", result);
    return result;
  }

  /*TraceManager::infos(TraceManager::TRACE_LEVEL_NORMAL,
          "util",
          "System command: '%s'..", full_cmde);*/


# ifdef LINUX

  std::string s = std::string(full_cmde) + " &";

  result = system(s.c_str());
# else

  STARTUPINFO si;
  PROCESS_INFORMATION pi;

  ZeroMemory( &si, sizeof(si) );
  si.cb = sizeof(si);
  ZeroMemory( &pi, sizeof(pi) );


  //SECURITY_ATTRIBUTES sa;
  if(!::CreateProcess(nullptr,
      /*(LPWSTR)*/ full_cmde,
      nullptr, /* security attributes for process */
      nullptr, /* security attributes for thread */
      0,    /* inherits handles ? */
      CREATE_NO_WINDOW,    /* creation flags */
      nullptr, /* environment */
      nullptr, /* initial path */
      &si,  /* startup infos */
      &pi   /* process infos */
          ))
  {
    result = 1;
  }
  else
  {
    result = 0;
    /*
    // Wait until child process exits.
    ::WaitForSingleObject( pi.hProcess, INFINITE );

    // Close process and thread handles.
    ::CloseHandle(pi.hProcess);
    ::CloseHandle(pi.hThread);*/
  }



# endif

  if(result != 0)
  {
    fprintf(stderr, "System command failure (%d): '%s'.\n", result, full_cmde);
    erreur("System command failure(%d): '%s'..\n", result, full_cmde);
  }

  free(full_cmde);

  return result;
}



int proceed_syscmde(std::string cmde, ...)
{
  char *full_cmde;
  va_list ap;
  int result;

  va_start(ap, cmde);
  result = vasprintf(&full_cmde, cmde.c_str(), ap);
  va_end(ap);

  if(result == -1)
  {
    fprintf(stderr, "vasprintf failure: %d.\n", result);
    return result;
  }

  if(full_cmde != nullptr)
    infos("System command: '%s'..", full_cmde);


# ifdef LINUX
  result = system(full_cmde);
# else

  STARTUPINFO si;
  PROCESS_INFORMATION pi;

  ZeroMemory( &si, sizeof(si) );
  si.cb = sizeof(si);
  ZeroMemory( &pi, sizeof(pi) );


  //SECURITY_ATTRIBUTES sa;
  if(!::CreateProcess(nullptr, 
		  /*(LPWSTR)*/ full_cmde,
		  nullptr, /* security attributes for process */
		  nullptr, /* security attributes for thread */
		  1,    /* inherits handles ? */
		  /*CREATE_NO_WINDOW*/0,    /* creation flags */
		  nullptr, /* environment */
		  nullptr, /* initial path */
		  &si,  /* startup infos */
		  &pi   /* process infos */
		      ))
  {
    result = 1;
  }
  else
  {
    result = 0;
    // Wait until child process exits.
    ::WaitForSingleObject( pi.hProcess, INFINITE );
  
    // Close process and thread handles. 
    ::CloseHandle(pi.hProcess);
    ::CloseHandle(pi.hThread);
  }



# endif

  if(result != 0)
  {
    fprintf(stderr, "System command failure (%d): '%s'.\n", result, full_cmde);
    erreur("System command failure(%d): '%s'..\n", result, full_cmde);
  }

  free(full_cmde);

  return result;
}

std::string get_execution_path()
{
  return appdata.dossier_executable;
}

void init(int argc, const char **argv, const std::string &projet, const std::string &app,
          unsigned int vmaj,
          unsigned int vmin,
          unsigned int vpatch)
{
  CmdeLine cl(argc, argv);
  init(cl, projet, app, vmaj, vmin, vpatch);
}

void init(CmdeLine &cmdeline,
          const std::string &projet,
          const std::string &app,
          unsigned int vmaj,
          unsigned int vmin,
          unsigned int vpatch)
{
  std::string fullpath, unused;
  appdata.nom_appli              = app;
  appdata.nom_projet          = projet;

  if(app.size() == 0)
  {
    std::string dummy, fn;
    utils::files::split_path_and_filename(cmdeline.argv0, dummy, fn);
    appdata.nom_appli = utils::files::remove_extension(fn);
  }

  if(appdata.nom_appli.size() == 0)
    appdata.nom_appli = projet;

  //if(cmdeline.argv != nullptr)
    fullpath = get_self_path(cmdeline.argv0.c_str());
  files::split_path_and_filename(fullpath, appdata.dossier_executable, unused);
  Localized::current_language = Localized::LANG_FR;


  //std::cout << "fullpath = " << fullpath << std::endl;
  //std::cout << "Exec dir = " << appdata.exec_dir << std::endl;

  if(cmdeline.has_option("-l"))
    Localized::current_language = Localized::parse_language(cmdeline.get_option("-l", "fr"));

  if(appdata.dossier_executable.size() == 0)
    appdata.dossier_executable = ".";

  appdata.chemin_donnees_fixes = appdata.dossier_executable + PATH_SEP + "data";

  //std::cout << "Fixed data path = " << appdata.fixed_data_path << std::endl;

  if(!files::dir_exists(appdata.chemin_donnees_fixes))// + PATH_SEP + "std-lang.xml"))
  {
#   ifdef WIN
    // SHOULD EXIST ALL TIME!
    fprintf(stderr, "Fixed data path not found: [%s].\n", appdata.chemin_donnees_fixes.c_str());
#   else
    appdata.chemin_donnees_fixes = "/usr/share/" + projet + "/data";
#   endif
  }

  appdata.chemin_images = appdata.chemin_donnees_fixes + PATH_SEP + "img";

  std::string cup = utils::get_current_user_path();
  // Check if writable data path exists
  if(!files::dir_exists(cup))
  {
    files::creation_dossier(cup);
    //fprintf(stdout, "created directory %s.\n", cup.c_str());
  }

  std::string cudp = utils::get_current_user_doc_path();
  if(!files::dir_exists(cudp))
    files::creation_dossier(cudp);

  std::string log_file = cup + PATH_SEP + appdata.nom_appli + "-log.txt";

  journal::TraceLevel tl = journal::AL_NONE;
  journal::TraceLevel tlf = journal::AL_NONE;

  if(cmdeline.has_option("-v"))
    tl = journal::AL_NORMAL;

  if(cmdeline.has_option("-vv"))
    tl = journal::AL_VERBOSE;

  if(cmdeline.has_option("--ftrace-level"))
  {
    int level = cmdeline.get_int_option("--ftrace-level", 7);
    if(level < 6)
      tlf = (journal::TraceLevel) level;
  }

  if(cmdeline.has_option("--infos-file-name"))
  {
    log_file = cmdeline.get_option("--infos-file-name");
  }

  if(cmdeline.has_option("--infos-level"))
  {
    int level = cmdeline.get_int_option("--infos-level", 7);
    if(level < 6)
      tl = (journal::TraceLevel) level;
  }

  //printf("tl = %d\n", (int) tl);

  /* Setup STDOUT traces level */
  journal::set_global_min_level(journal::TRACE_TARGET_STD, tl);

  /* Setup FILE traces level */
  journal::set_global_min_level(journal::TRACE_TARGET_FILE, tlf);

  journal::set_log_file(log_file);

  {
    string fs = utils::get_fixed_data_path() + PATH_SEP + "std-lang.xml";
    if(files::file_exists(fs))
      langue.load(fs);
  }

  {
    string fs = utils::get_fixed_data_path() + PATH_SEP + "lang.xml";
    if(files::file_exists(fs))
      langue.load(fs);
  }

# ifdef DEBUG_MODE
  journal::set_abort_on_anomaly(true);
# else
  if(cmdeline.has_option("--abrt"))
    journal::set_abort_on_anomaly(true);
# endif


  ////////////////////////////////////////////////
  /// VÃ©rification / crÃ©ation du dossier utilisateur/MGC
  ////////////////////////////////////////////////
  {
    std::string chem = utils::get_current_user_doc_path();
    if(!utils::files::dir_exists(chem))
    {
      infos("Creation du dossier utilisateur (%s).", chem.c_str());
      utils::files::creation_dossier(chem);
    }
    if(!utils::files::dir_exists(chem))
      erreur("Impossible de crÃ©er le dossier utilisateur [%s].", chem.c_str());
  }



  std::string dts = utils::get_current_date_time();
  infos("Fichier journal pour l'application %s, version %d.%d.%d\nDate / heure lancement application : %s\n**************************************\n**************************************\n**************************************",
      appdata.nom_appli.c_str(), vmaj, vmin, vpatch, dts.c_str());
  infos("Initialisation libcutil faite.");
}

std::string get_fixed_data_path()
{
/*# ifdef WIN
  return exec_dir;
# else
  return exec_dir;
# endif*/
  /*if(fixed_data_path.size() == 0)
  {
    if(exec_dir.size() == 0)
      return ".";
    return exec_dir;
  }
  return fixed_data_path;*/

  //printf("get_fixed_data_path: %s\n", appdata.fixed_data_path.c_str());fflush(0);

  return appdata.chemin_donnees_fixes;
}

std::string get_img_path()
{
  /*if(img_path.size() == 0)
    img_path = exec_dir + PATH_SEP + "img";
  return img_path;*/
  //return get_fixed_data_path() + PATH_SEP + "img";
  return appdata.chemin_images;
}

/*void Util::set_fixed_data_path(std::string s)
{
  fixed_data_path = s;
}*/





std::string str::unix_path_to_win_path(std::string s_)
{
  //s_ = replace_template(s_);
  const char *s = s_.c_str();
  char buffer[1000];
  unsigned int j = 0;
  for(unsigned int i = 0; i < strlen(s); i++)
  {
    if(s[i] == '/')
      buffer[j++] = '\\';
    else
      buffer[j++] = s[i];
  }
  buffer[j] = 0;
  return std::string(buffer);
}





void str::encode_str(std::string str, std::vector<unsigned char> vec)
{
  const char *s = str.c_str();
  for(unsigned int i = 0; i < strlen(s); i++)
  {
    unsigned char c = (unsigned char) s[i];
    vec.push_back(c);
  }
  vec.push_back(0x00);
}



void str::encode_byte_array_deci(std::string str, std::vector<unsigned char> vec)
{
  const char *s = str.c_str();
  unsigned char current = 0;
  if(!is_deci(s[0]))
  {
    erreur("Encoding decimal byte array : this string cannot be encoded: %s", str.c_str());
    return;
  }
  while(strlen(s) > 0)
  {
    if(is_deci(s[0]))
    {
      current = current * 10 + (s[0] - '0');
    }
    else
    {
      if(strlen(s) > 1)
      {
        vec.push_back(current);
        current = 0;
      }
    }
    s++;
  }
  vec.push_back(current);
}

bool str::is_deci(char c)
{
  return ((c >= '0') && (c <= '9'));
}

bool str::is_hexa(char c)
{
  return (((c >= '0') && (c <= '9')) || ((c >= 'a') && (c <= 'f')) || ((c >= 'A') && (c <= 'F')));
}


int str::parse_hexa_list(const std::string str, std::vector<unsigned char> &res)
{
  const char *s = str.c_str();
  res.clear();
  int current = 0;
  if(!str::is_hexa(s[0]))
    return -1;
  while(strlen(s) > 0)
  {
    if(str::is_deci(s[0]))
    {
      current = (current << 4) + (s[0] - '0');
    }
    else if((s[0] >= 'a') && (s[0] <= 'f'))
    {
      current = (current << 4) + (s[0] - 'a' + 10);
    }
    else if((s[0] >= 'A') && (s[0] <= 'F'))
    {
      current = (current << 4) + (s[0] - 'a' + 10);
    }
    else
    {
      if(strlen(s) > 1)
      {
        res.push_back((unsigned char) current);
        current = 0;
      }
    }
    s++;
  }
  res.push_back((unsigned char) current);
  return 0;
}



int str::parse_string_list(const std::string str, std::vector<std::string> &res, char separator)
{
  const char *s = str.c_str();
  res.clear();
  std::string current = "";

  while(strlen(s) > 0)
  {
    if(s[0] != separator)
    {
      char tmp[2];
      tmp[0] = s[0];
      tmp[1] = 0;
      current = current + std::string(tmp);
    }
    else
    {
      if(strlen(s) > 1)
      {
        res.push_back(current);
        current = "";
      }
    }
    s++;
  }
  res.push_back(current);
  return 0;
}

int str::parse_int_list(const std::string str, std::vector<int> &res)
{
  const char *s = str.c_str();
  res.clear();
  int current = 0;
  if(!str::is_deci(s[0]))
    return -1;
  while(strlen(s) > 0)
  {
    if(str::is_deci(s[0]))
    {
      current = current * 10 + (s[0] - '0');
    }
    else
    {
      if(((s[0] == '.') || ((s[0] == ','))) && (strlen(s) > 1))
      {
        res.push_back(current);
        current = 0;
      }
      else
      {
        return -1;
      }
    }
    s++;
  }
  res.push_back(current);
  return 0;
}

void str::encode_byte_array_hexa(std::string str, std::vector<unsigned char> vec)
{
  const char *s = str.c_str();
  unsigned char current = 0;
  if(!is_hexa(s[0]))
  {
    return;
  }

  while(strlen(s) > 0)
  {
    if(is_deci(s[0]))
    {
      current = (current << 4) + (s[0] - '0');
    }
    else if((s[0] >= 'a') && (s[0] <= 'f'))
    {
      current = (current << 4) + (s[0] - 'a' + 10);
    }
    else if((s[0] >= 'A') && (s[0] <= 'F'))
    {
      current = (current << 4) + (s[0] - 'a' + 10);
    }
    else
    {
      if(strlen(s) > 1)
      {
        vec.push_back(current);
        current = 0;
      }
    }
    s++;
  }
  vec.push_back(current);
}




std::string str::xmlAtt(std::string name, std::string val)
{
  const char *s = val.c_str();
  char buf[1000];
  unsigned int n = strlen(s);
  unsigned int j = 0;
  for(unsigned int i = 0; i < n; i++)
  {
    if(s[i] == '"')
    {
      buf[j++] = '\\';
      buf[j++] = 'G';
    }
    else
    {
      buf[j++] = s[i];
    }
  }
  buf[j] = 0;
  return std::string(" ") + name + "=\"" + std::string(buf) + "\"";
}

std::string str::xmlAtt(std::string name, int val)
{
  return std::string(" ") + name + "=\"" + str::int2str(val) + "\"";
}

std::string str::xmlAtt(std::string name, bool val)
{
  return std::string(" ") + name + "=\"" + (val ? "true" : "false") + "\"";
}

std::string str::int2strasm(int i)
{
  return std::string("d'") + int2str(i) + "'";
}

std::string str::uint2strhexa(int i)
{
  char buf[100];
  if(i < 0x10)
    sprintf(buf, "0x0%x", i);
  else
    sprintf(buf, "0x%x", i);
  return std::string(buf);
}

std::string str::int2strhexa(int i, int nbits)
{
  unsigned int v = (unsigned int) i;
  char buf[100];
  if(nbits == 32)
    sprintf(buf, "%.8x", v);
  else if(nbits == 8)
    sprintf(buf, "%.2x", v);
  else
    sprintf(buf, "%.4x", v);
  return std::string(buf);
}

std::string str::int2strhexa(int i)
{
  char buf[100];
  sprintf(buf, "%x", i);
  return std::string(buf);
}

std::string str::int2str_capacity(uint64_t val, bool truncate)
{
  if(val < 1024)
  {
    return str::int2str(val) + " bytes";
  }
  else if(val < 1024*1024)
  {
    if(((val & 1023) == 0) || truncate)
      return str::int2str(val >> 10) + " kbi";
    else
      return str::int2str(val) + " bytes";
  }

  if(((val & (1024*1024-1)) == 0) || truncate)
  {
    return str::int2str(val >> 20) + " mbi";
  }
  else   if(((val & 1023) == 0) || truncate)
  {
    return str::int2str(val >> 10) + " kbi";
  }
  else
  {
    return str::int2str(val) + " bytes";
  }
}

std::string str::int2str(int i, int nb_digits)
{
  char buf[100];
  char format[100];
  sprintf(format, "%%%dd", nb_digits);
  sprintf(buf, format, i);
  return std::string(buf);
}

std::string str::int2str(int i)
{
  char buf[100];
  sprintf(buf, "%d", i);
  return std::string(buf);
}

int files::save_txt_file(std::string filename, std::string content)
{
  FILE *f = fopen(filename.c_str(), "wt");
  if(f == nullptr)
  {
#     ifndef WIN
    perror("fopen error.\n");
#     endif
    printf("Fatal error: Cannot open %s for writing.\n", filename.c_str());
    fflush(0);
    return -1;
  }
  fprintf(f, "%s", content.c_str());
  fclose(f);
  return 0;
}

std::string get_current_date_time()
{
  time_t tim;
  time(&tim);
  std::string res = std::string(ctime(&tim));
  if(res[res.size() - 1] == '\n')
    res = res.substr(0, res.size() - 1);
  return res;
}

string str::to_latex(const string s)
{
  string res = "";

  for(uint32_t i = 0; i < s.size(); i++)
  {
    if(s[i] == '%')
    {
      res += "\\%";
    }
    else
    {
      char c[2];
      c[0] = s[i];
      c[1] = 0;
      res += string(&c[0]);
    }
  }

  return res;
}

std::string str::str_to_file(std::string name)
{
  std::string tmp = str::lowercase(name);
  const char *s = tmp.c_str();
  char *buf = (char *) malloc(strlen(s)*2+2);
  unsigned int i, j = 0;


  for(i = 0; i < strlen(s); i++)
  {
	char c = s[i];
	if((c == ' ') || (c == '-') || (c == '_'))
	{
	  c = '-';
	}
	buf[j++] = c;
  }
  buf[j] = 0;
  tmp = std::string(buf);
  free(buf);
  return tmp;
}

std::string str::str_to_class(std::string name)
{
  std::string tmp = str::lowercase(name);
  const char *s = tmp.c_str();
  char *buf = (char *) malloc(strlen(s)*2+2);
  unsigned int i, j = 0;
  bool next_maj = true;


  for(i = 0; i < strlen(s); i++)
  {
    char c = s[i];
    if((c == ' ') || (c == '-') || (c == '_'))
    {
      next_maj = true;
      continue;
    }
    if(next_maj)
    {
      next_maj = false;
      if((c >= 'a') && (c <= 'z'))
        c = c + ('A' - 'a');
    }

    buf[j++] = c;
  }
  buf[j] = 0;
  tmp = std::string(buf);
  free(buf);
  return tmp;
}

std::string str::str_to_var(std::string name)
{
  if((name.size() > 0) && (name[0] >= '0') && (name[0] <= '9'))
    name = "_" + name;

  std::string tmp = str::lowercase(name);
  const char *s = tmp.c_str();
  char buf[500];
  unsigned int i;

  for(i = 0; i < strlen(s); i++)
  {
    char c = s[i];
    if(c == ' ')
      c = '_';
    if(c == '-')
      c = '_';
    buf[i] = c;
  }
  buf[i] = 0;
  return std::string(buf);
}


std::string str::str_to_cst(std::string s)
{
  std::string buf = s;
  unsigned int i;
  for(i = 0; i < s.size(); i++)
  {
    char c = s[i];
    if((c >= 'a')  && (c <= 'z'))
      c = c - 'a' + 'A';

    if(c == '+')
      c = 'P';

    if(c == '/')
      c = 'N';

    /* Forbid every special character except '_' */
    if(((c < 'A') || (c > 'Z')) && (!str::is_deci(c)))
      c = '_';

    buf[i] = c;
  }
  return buf;
}


bool files::dir_exists(string name)
{
  //char *myDir = dirname(myPath);
  struct stat my_stat;
  if ((stat(name.c_str(), &my_stat) == 0) && (((my_stat.st_mode) & S_IFMT) == S_IFDIR))
  {
    return true;
  }

  return false;
  //return file_exists(name);
}

bool files::file_exists(std::string name)
{
  FILE *f = fopen(name.c_str(), "r");
  if(f == nullptr)
    return false;
  fclose(f);
  return true;
}

std::string get_current_user_doc_path()
{
# ifdef WIN
  TCHAR tmp[MAX_PATH]={0};
  if(S_OK == ::SHGetFolderPath(nullptr, CSIDL_PERSONAL, nullptr, 0, tmp))
  {
    std::ostringstream tmp2;
    tmp2 << tmp;
    return tmp2.str() + PATH_SEP + appdata.nom_projet;
  }
  return "";
# else
  std::string s = "";
  {
    char *temp = getenv("HOME");
    if(temp == nullptr)
    {
      perror("getenv");
      return s;
    }
    return std::string(temp) + "/" + appdata.nom_projet;
  }
# endif
}

std::string get_current_user_path()
{
# ifdef WIN
  TCHAR tmp[MAX_PATH]={0};
  if(S_OK == ::SHGetFolderPath(nullptr, CSIDL_APPDATA, nullptr, 0, tmp))
  {
    std::ostringstream tmp2;
    tmp2 << tmp;
    return tmp2.str() + PATH_SEP + appdata.nom_projet;
  }
  return "";
# else
  std::string s = "";
  {
    char *temp = getenv("HOME");
    if(temp == nullptr)
    {
      perror("getenv");
      return s;
    }
    return std::string(temp) + "/." + appdata.nom_projet;
  }
# endif
}

std::string get_all_user_path()
{
# ifdef WIN
  TCHAR tmp[MAX_PATH]={0};
  if(S_OK == ::SHGetFolderPath(nullptr, CSIDL_COMMON_APPDATA, nullptr, 0, tmp))
  {
    std::ostringstream tmp2;
    tmp2 << tmp;
    return tmp2.str() + PATH_SEP + appdata.nom_appli;
  }
  return "";
# else

  // TODO
  return "";
# endif
}




std::string get_env_variable(const std::string &name,
                                   const std::string &default_value)
{
# ifdef WIN
  char bf[500];
#ifdef VSTUDIO
  WCHAR bf2[500];
  GetEnvironmentVariable((LPCWSTR) name.c_str(), bf2, 500);
  wcstombs(bf, bf2, 250);
#else
  GetEnvironmentVariable(name.c_str(), bf, 500);
#endif
  return std::string(bf);
# else
  char *res = getenv(name.c_str());
  if((res == nullptr) || (strlen(res) == 0))
    return default_value;
  return std::string(res);
# endif
}




int files::check_and_build_directory(std::string path)
{
  if(!files::file_exists(path))
  {
    avertissement("%s: output path [%s] does not exist.", __func__, path.c_str());
    if(files::creation_dossier(path))
    {
      erreur("Failed to create output path.");
      return -1;
    }
  }
  return 0;
}

int files::explore_dossier(std::string chemin, std::vector<std::string> &fichiers)
{
# ifdef WIN
  DIR *dir;
  struct dirent *ent;
  if((dir = opendir(chemin.c_str())) == NULL)
  {
    fprintf(stderr, "Chemin non trouve : [%s].\n", chemin.c_str());
    return -1;
  }

  while ((ent = readdir (dir)) != NULL)
  {
    std::string f = std::string(ent->d_name);
    if(f.size() > 4)
      fichiers.push_back(chemin + '/' + f);
  }
  closedir (dir);
  return 0;
# else
  return -1;
# endif
}

int files::creation_dossier(std::string path)
{
# ifdef WIN
# ifdef VSTUDIO
  CreateDirectory((LPCWSTR) path.c_str(), nullptr);
  return 0;
#else
  CreateDirectory(path.c_str(), nullptr);
  return 0;
#endif
# else
  int res = mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  return res;
# endif
}



string str::get_filename_resume(const string &filename, unsigned int max_chars)
{
  if(filename.size() < max_chars)
    return filename;

  int i;
  unsigned int count = 0;
  for(i = filename.size() - 1; i > 0; i--)
  {
    if((filename[i] == '/') || (filename[i] == '\\'))
    {
      count++;
      if(count == 3)
        break;
    }
  }

  if(i > 3)
    return filename.substr(0, 3) + "..." + filename.substr(i, filename.size() - i);

  return filename;
}


string files::get_extension(const string &filepath)
{
  unsigned int i, n = filepath.size();
  for(i = 0; i < n; i++)
  {
    if(filepath[n-i-1] == '.')
      break;
  }

  /* no extension? */
  if((i == n) || (i == 0))
    return "";

  return filepath.substr(n - i, i);
}

string files::remove_extension(const string &filepath)
{
  unsigned int i, n = filepath.size();
  for(i = 0; i < n; i++)
  {
    if(filepath[n-i-1] == '.')
      break;
  }

  /* no extension? */
  if(i == 0)
    return filepath;

  return filepath.substr(0, n - i - 1);
}








int files::parse_filepath(const std::string &path,
                             std::vector<std::string> &items)
{
  //string sep = files::get_path_separator();

  items.clear();

  string accu = "";

  for(unsigned int i = 0; i < path.size(); i++)
  {
    if((path[i] == '/') || (path[i] == '\\'))
    {
      items.push_back(accu);
      accu = "";
    }
    else
    {
      string s = " ";
      s[0] = path[i];
      accu += s;
    }
  }

  if(accu.size() > 0)
    items.push_back(accu);

  return 0;
}

int files::abs2rel(const std::string &ref,
                      const std::string &abs,
                      std::string &result)
{
  unsigned int i, ncommon;
  vector<string> vref, vabs;

  parse_filepath(abs, vabs);
  parse_filepath(ref, vref);

  printf("vabs = ");
  for(i = 0; i < vabs.size(); i++)
    printf("[%s] ", vabs[i].c_str());
  printf("\nvref = ");
  for(i = 0; i < vref.size(); i++)
    printf("[%s] ", vref[i].c_str());
  printf("\n");

  for(i = 0; (i < vabs.size()) && (i < vref.size()); i++)
  {
#   ifdef WIN
    if(str::lowercase(vref[i]).compare(str::lowercase(vabs[i])) != 0)
#   else
    if(vref[i].compare(vabs[i]) != 0)
#   endif
      break;
  }

  ncommon = i;
  // ncommon = index of the first different item.

  result = "";



  for(i = ncommon; i < vref.size(); i++)
  {
    if(i > ncommon)
      result += PATH_SEP;
    result += "..";
  }

  // Target path is inside the reference path
  if(ncommon == vref.size())
    result += ".";

  for(i = ncommon; i < vabs.size(); i++)
    result += PATH_SEP + vabs[i];

  return 0;
}

void files::remplacement_motif(std::string &chemin)
{
  if(chemin.substr(0, 5) == "$DATA")
  {
    chemin = utils::get_fixed_data_path() + chemin.substr(5, chemin.size() - 5);
  }
}

int files::rel2abs(const std::string &ref,
                      const std::string &rel,
                      std::string &result)
{
  /* Not a relative path ? */
  if(rel[0] != '.')
    result = rel;
  else
  {
    char psep = get_path_separator()[0];
    string r = rel;
    if((rel.size() == 1) && (rel[0] == '.'))
      r = "";
    // Supprime le "./" de "./*"
    if((r.size() >= 2) && (r[0] == '.') && (r[1] == psep))
      r = r.substr(2, r.size() - 2);

    result = ref + PATH_SEP + r;
  }
    //result = build_absolute_path(ref, rel);
  return 0;
}

std::string files::build_absolute_path(const std::string absolute_origin,
                                          const std::string relative_path)
{
  std::string res, abs_path, abs_fn, rel;
  char psep = get_path_separator()[0];

  if(relative_path[0] != '.')
  {
    return relative_path;
  }

  //printf("Building absolute path from '%s' and '%s'...\n", 
  //       absolute_origin.c_str(),
  //       relative_path.c_str());

  split_path_and_filename(absolute_origin, abs_path, abs_fn);
  abs_path = files::correct_path_separators(abs_path);
  rel      = files::correct_path_separators(relative_path);
  //printf("Absolute root path = '%s'.\n", abs_path.c_str());
  // Supprime le "." de "."
  if((rel.size() == 1) && (rel[0] == '.'))
    rel = "";
  // Supprime le "./" de "./*"
  if((rel.size() >= 2) && (rel[0] == '.') && (rel[1] == psep))
    rel = rel.substr(2, rel.size() - 2);
  res = abs_path + get_path_separator() + rel;
  //printf("Result = %s.\n", res.c_str());
  return res;
}

std::string files::get_path_separator()
{
# ifdef WIN
  return "\\";
# else
  return "/";
# endif
}

std::string files::correct_path_separators(std::string s_)
{
  //s_ = replace_template(s_);
  const char *s = s_.c_str();
  char buffer[1000];
  unsigned int j = 0;
  for(unsigned int i = 0; i < strlen(s); i++)
  {
    if((s[i] == '/') || (s[i] == '\\'))
      buffer[j++] = get_path_separator()[0];
    else
      buffer[j++] = s[i];
  }
  buffer[j] = 0;
  return std::string(buffer);
}

void files::split_path_and_filename(const std::string complete_filename,
    std::string &path,
    std::string &filename)
{
  const char *s = complete_filename.c_str();
  unsigned int n = strlen(s);
  unsigned int i, j;
  j = 0;
  for(i = 0; i < n; i++)
  {
    if((s[i] == '/') || (s[i] == '\\'))
      j = i+1;
  }
  char buf[500];
  for(i = 0; i < j; i++)
    buf[i] = s[i];
  buf[j] = 0;
  // Remove last '/' or '\'
  if(j > 0)
    buf[j-1] = 0;
  path = std::string(buf);
  char buf2[500];
  for(i = j; i < n; i++)
    buf2[i-j] = s[i];
  buf2[n-j] = 0;
  filename = std::string(buf2);
}

void files::delete_file(std::string name)
{
# ifdef WIN
  ::DeleteFile((LPTSTR) name.c_str());
# else
  if(unlink(name.c_str()) != 0)
    perror("unlink");
# endif

  /*_chmod(nf, _S_IWRITE);
  if (DeleteFile((LPTSTR)nf) == 0) 
  {
  }*/
}



std::string str::lowercase(std::string s)
{
  char buf[1000];
  unsigned int i;

  if(s.size() == 0)
    return "";

  sprintf(buf, "%s", s.c_str());
  for(i = 0; i < s.size(); i++)
  {
    if((buf[i] >= 'A') && (buf[i] <= 'Z'))
      buf[i] = 'a' + (buf[i] - 'A');
  }
  buf[i] = 0;

  //TraceManager::infos(TraceManager::TRACE_LEVEL_MAJOR, "util", "lowercase: in = %s -> %s", s.c_str(), buf);
  return std::string(buf);
}

TextMatrix::TextMatrix(uint32_t ncols)
{
  this->ncols = ncols;
}

void TextMatrix::add(std::string s)
{
  current_row.push_back(s);
}

void TextMatrix::add_unformatted_line(std::string s)
{
  next_line();
  current_row.push_back(s);
  lst.push_back(current_row);
  unformatted.push_back(true);
  current_row.clear();
}

void TextMatrix::next_line()
{
  if(current_row.size() > 0)
  {
    lst.push_back(current_row);
    unformatted.push_back(false);
    current_row.clear();
  }
}

static unsigned int str_screen_len(const string &s)
{
  unsigned int i, res = 0, n = s.size();
  for(i = 0; i < n; i++)
  {
    if(((unsigned char) s[i]) == 0x1b)
    {
      i++;
      while((i < n) && (s[i] != 'm'))
        i++;
    }
    else
      res++;
  }
  return res;
}

std::string TextMatrix::get_result()
{
  unsigned int i, j, k, n = lst.size();
  std::vector<uint32_t> max_size;
  std::string res = "";

  next_line();

  max_size.clear();

  for(i = 0; i < ncols; i++)
    max_size.push_back(0);

  for(i = 0; i < n; i++)
  {
    if(unformatted[i])
      continue;
    if(lst[i].size() > max_size.size())
    {
      uint32_t diff = lst[i].size() - max_size.size();
      for(j = 0; j < diff; j++)
        max_size.push_back(0);
    }
    if(lst[i].size() > 1)
    {
      for(j = 0; j < lst[i].size(); j++)
      {
        uint32_t len = str_screen_len(lst[i][j]);
        if(len > max_size[j])
          max_size[j] = len;
      }
    }
  }

  for(i = 0; i < n; i++)
  {
    if(unformatted[i])
    {
      res += lst[i][0];
      continue;
    }
    for(j = 0; j < lst[i].size(); j++)
    {
      std::string cell = lst[i][j];

      res += cell;

      if(j + 1 < lst[i].size())
      {
        for(k = 0; k < (max_size[j] - str_screen_len(cell)) + 1; k++)
          res += " ";
      }
    }
    res += "\n";
  }

  current_row.clear();
  lst.clear();
  unformatted.clear();
  return res;
}

void TextMatrix::reset(uint32_t ncols)
{
  this->ncols = ncols;
  current_row.clear();
  lst.clear();
}


TextAlign::TextAlign()
{

}

void TextAlign::add(std::string s1, std::string s2)
{
  alst1.push_back(s1);
  alst2.push_back(s2);
}

void TextAlign::add(std::string s1, std::string s2, std::string s3)
{
  alst1.push_back(s1);
  alst2.push_back(s2);
  alst3.push_back(s3);
}

void TextAlign::add(std::string comment)
{
  comments.push_back(comment);
  comments_pos.push_back(alst1.size());
}

std::string TextAlign::get_result()
{
  int ncols = 2;
  if(alst3.size() == alst1.size())
    ncols = 3;
  unsigned int i, j, n = alst1.size();
  unsigned int max_size = 0, max_size2 = 0;
  for(i = 0; i < n; i++)
  {
    unsigned int len1 = alst1[i].size();
    if(len1 > max_size)
      max_size = len1;
    unsigned int len2 = alst2[i].size();
    if(len2 > max_size2)
      max_size2 = len2;
  }
  std::string res = "";
  unsigned int k = 0;
  for(i = 0; i < n; i++)
  {
    if((k < comments.size()) && (i == comments_pos[k]))
      res += comments[k++];
    res += alst1[i];
    for(j = 0; j < (max_size - alst1[i].size()) + 2; j++)
      res += " ";
    res += alst2[i];
    if(ncols == 3)
    {
      for(j = 0; j < (max_size2 - alst2[i].size()) + 2; j++)
        res += " ";
      res += alst3[i];
    }
    res += "\n";
  }

  if((k < comments.size()) && (i == comments_pos[k]))
    res += comments[k++];

  comments.clear();
  comments_pos.clear();
  alst1.clear();
  alst2.clear();
  alst3.clear();
  return res;
}





void Section::load(std::string nom_fichier)
{
  MXml mx;
  if(mx.from_file(nom_fichier))
  {
    erreur("Impossible de charger le fichier de loc (%s).", nom_fichier.c_str());
    return;
  }
  load(mx);
}


void Section::load()
{
  load("lang.xml");
}

Section::Section()
{

}

void Section::operator =(const Section &c)
{
  nom = c.nom;
  elmts = c.elmts;
  subs = c.subs;
  //data = c.data;
  //this->current_language = c.current_language;
}

Section::Section(const Section &c)
{
  *this = c;
}



static unsigned char utf8_to_latin_char(unsigned char a, unsigned char b)
{
  if(a == 0xc3)
  {
    return b + 0x40;
  }
  else if(a == 0xc2)
  {
    return b;
  }
  return ' ';
}

std::string str::utf8_to_latin(std::string s_)
{
  unsigned char *buf = (unsigned char *) malloc(s_.size() + 1);//[2000];
  const unsigned char *s = (const unsigned char *) s_.c_str();
  int j = 0;
  unsigned int n = strlen((const char *)s);
  for(unsigned int i = 0; i < n; i++)
  {
    if((s[i] == 0xc3) || (s[i] == 0xc2))
    {
      buf[j++] = utf8_to_latin_char(s[i], s[i+1]);
      i++;
    }
    else
    {
      buf[j++] = s[i];
    }
  }
  buf[j] = 0;
  std::string res = std::string((char *) buf);
  free(buf);
  return res;
}

std::string str::latin_to_utf8(std::string s)
{
  // Unsigned char cast, otherwise faile to compare with hexadecimal values.
  const unsigned char *s2 = (const unsigned char *) s.c_str();
  unsigned char *buf = (unsigned char *) malloc(s.size() * 2 + 1);
  unsigned int j = 0, n = s.size();

  for(unsigned int i = 0; i < n; i++)
  {
    /** Already UTF8? Skip. */
    if((s2[i] == 0xc2) || (s2[i] == 0xc3))
    {
      buf[j++] = s2[i++];
      buf[j++] = s2[i];
    }
    else if((s2[i] >= 0x80) && (s2[i] <= 0xbf))
    {
      buf[j++] = 0xc2;
      buf[j++] = s2[i];
    }
    else if(s2[i] >= 0xc0)
    {
      buf[j++] = 0xc3;
      buf[j++] = s2[i] - 0x40;
    }
    else
      buf[j++] = s2[i];
  }
  buf[j] = 0;
  std::string res = std::string((char *) buf);
  free(buf);
  return res;
}

bool Section::has_item(const std::string &name) const
{
  //return data.has_child("item", "name", name);
  for(const auto &e: elmts)
    if(e.get_id() == name)
      return true;
  return false;
}

const utils::model::Localized &Section::get_localized(const std::string &name) const
{
  for(const auto &e: elmts)
    if(e.get_id() == name)
      return e;
  erreur("Item de localisation non trouve: %s", name.c_str());
  return elmts[0];
}

std::string Section::get_item(const std::string &name) const
{

  for(const auto &e: elmts)
    if(e.get_id() == name)
      return e.get_localized();

  erreur("Item de localisation non trouvé : %s (dans section [%s]).",
                name.c_str(), this->nom.c_str());
  return name + "=?";
}

/*const char *Section::get_text(std::string name)
{*/
  /*if((data == nullptr) || (data == 0))
    return name.c_str();*/

/*  if(!data.has_child("item", "name", name))
  {
    printf("Item not found: %s\n", name.c_str());
    return name.c_str();
  }

  MXml elt = data.get_child("item", "name", name);
  return elt.get_attribute(this->current_language).string_value.c_str();
}*/

const Section &Section::get_section(const std::string &name) const
{
  for(const auto &s: subs)
    if(s->nom == name)
      return *s;
  erreur("Sous section non trouvee : %s", name.c_str());
  return *this;
}

Section &Section::get_section(const std::string &name)
{
  for(auto &s: subs)
    if(s->nom == name)
      return *s;
  erreur("Sous section non trouvee : %s", name.c_str());
  return *this;
}

Section::~Section()
{
  //printf("delete section.\n");
}

void Section::load(const utils::model::MXml &mx)
{
  this->nom = mx.get_attribute("name").to_string();
  for(auto &ch: mx.children)
  {
    if(ch.name == "include")
    {
      load(utils::get_fixed_data_path() + PATH_SEP + ch.get_attribute("file").to_string());
    }
    else if(ch.name == "item")
    {
      elmts.push_back(utils::model::Localized(ch));
    }
    else if(ch.name == "section")
    {
      Section *sec = new Section(ch);
      subs.push_back(utils::refptr<Section>(sec));
    }
  }
}

Section::Section(const MXml &mx)
{
  load(mx);
}


uint32_t Util::extract_bits(uint8_t *buffer, uint32_t offset_in_bits, uint32_t nbits)
{
  uint32_t res = 0;

  buffer += offset_in_bits / 8;
  offset_in_bits &= 7;

  /*************************************************************
   * Hypoth�se :
   *
   * Si mot de 12 bits, d�cal� de 1 bit.
   *
   *
   * [7  6       0] [7   5 4   0] [7       0]
   *   [11                 0]
   *
   *************************************************************/

  while(nbits > 0)
  {
    // Extract n bits from the first byte in the buffer
    // n = max(nbits, 8 - offset)
    uint16_t n = nbits;
    if(n > 8 - offset_in_bits)
      n = 8 - offset_in_bits;



    //uint32_t extract = (((uint32_t) buffer[0]) << offset_in_bits) & 0xff;

    uint32_t extract = ((uint32_t) buffer[0]) & (0x000000ff >> offset_in_bits);

    // Append to result
    res = (res << 8) | extract;


    nbits -= n;
    buffer++;
    offset_in_bits = 0;
  }
  return res;
}

CmdeLine::CmdeLine()
{

}

void CmdeLine::operator =(const CmdeLine &cmdeline)
{
  this->argv0 = cmdeline.argv0;
  this->prms = cmdeline.prms;
}

CmdeLine::CmdeLine(const std::string args)
{
  vector<string> lst;
  string s;
  istringstream is(args);
  while(is >> s)
    lst.push_back(s);
  init(lst);
}

void CmdeLine::init(vector<string> &argv)
{
  if(argv.size() > 0)
    argv0 = std::string(argv[0]);

  for(unsigned int i = 1; i < argv.size(); i++)
  {
    string opt = argv[i];

    if(opt[0] == '-')
    {
      CmdeLinePrm prm;
      prm.option = opt;

      if(i + 1 < argv.size())
      {
        string val = argv[i + 1];
        if(val[0] != '-')
        {
          prm.value = val;
          i++;
        }
      }
      prms.push_back(prm);
    }
    else
    {
      CmdeLinePrm prm;
      prm.option = opt;
      prms.push_back(prm);
    }
  }
}

void CmdeLine::init(int argc, const char **argv)
{
  vector<string> lst;
  for(unsigned int i = 0; i < (unsigned int) argc; i++)
    lst.push_back(string(argv[i]));
  init(lst);
}

CmdeLine::CmdeLine(int argc_, char **argv_)//: argc(argc_), argv((const char **) argv_)
{
  init(argc_, (const char **) argv_);
}

CmdeLine::CmdeLine(int argc_, const char **argv)//: argc(argc_)
{
  init(argc_, (const char **) argv);
}

bool CmdeLine::has_option(const std::string &name) const
{
  for(unsigned int i = 0; i < prms.size(); i++)
  {
    if(prms[i].option.compare(name) == 0)
      return true;
  }
  return false;
}

/*bool CmdeLine::get_boolean_option(const std::string &name,
                                  bool default_value)
{

}*/

int CmdeLine::get_int_option(const std::string &name,
                     int default_value) const
{
  std::string res = get_option(name, str::int2str(default_value));
  return atoi(res.c_str());
}

std::string CmdeLine::get_option(const std::string &name,
                       const std::string &default_value) const
{
  for(unsigned int i = 0; i < prms.size(); i++)
  {
    if(prms[i].option.compare(name) == 0)
    {
      if(prms[i].value.size() == 0)
        return default_value;
      return prms[i].value;
    }
  }
  return default_value;
}

void CmdeLine::set_option(const std::string &name, const std::string &value)
{
  for(unsigned int i = 0; i < prms.size(); i++)
  {
    if(prms[i].option.compare(name) == 0)
    {
      prms[i].value = value;
      return;
    }
  }
  CmdeLinePrm prm;
  prm.option = name;
  prm.value = value;
  prms.push_back(prm);
}


namespace model
{
Localized::Localized()
{

}

Localized::Localized(const Localized &l)
{
  *(this) = l;
}

void Localized::operator =(const Localized &l)
{
  items = l.items;
  descriptions = l.descriptions;
  id = l.id;
}

void Localized::set_value(Language lg, std::string value)
{
  if(value.size() == 0)
    return;

  if(lg == Localized::LANG_ID)
    id = value;

  for(unsigned int i = 0; i < items.size(); i++)
  {
    if(items[i].first == lg)
    {
      items[i].second = value;

      return;
    }
  }
  std::pair<Language, std::string> item;
  item.first  = lg;
  item.second = value;
  items.push_back(item);
}

std::string Localized::to_string() const
{
  unsigned int i;
  std::string s = "";
  for(i = 0; i < items.size(); i++)
  {
    s += "name[" + str::int2str((int) items[i].first) + "] = " + items[i].second + "\n";
  }
  for(i = 0; i < descriptions.size(); i++)
  {
    s += "desc[" + str::int2str((int) descriptions[i].first) + "] = " + descriptions[i].second + "\n";
  }
  return s;
}

void Localized::set_description(Language lg, std::string desc)
{
  if(desc.size() == 0)
    return;
  for(unsigned int i = 0; i < descriptions.size(); i++)
  {
    if(descriptions[i].first == lg)
    {
      descriptions[i].second = desc;
      return;
    }
  }
  std::pair<Language, std::string> item;
  item.first  = lg;
  item.second = desc;
  descriptions.push_back(item);
}

std::string Localized::get_value(Language lg) const
{
  std::string default_value = "";

  if(items.size() > 0)
    default_value = items[0].second;

  for(unsigned int i = 0; i < items.size(); i++)
  {
    if(items[i].first == lg)
    {
      return items[i].second;
    }
    if(items[i].first == LANG_EN)
      default_value = items[i].second;
  }


  //auto l = Localized::language_id(lg);
  //avertissement("Item loc non trouve : [%s], en langue [%s].", this->id.c_str(), l.c_str());


  return default_value;
}

/** @brief Equivalent to get_value(LANG_CURRENT) */
std::string Localized::get_localized() const
{
  return get_value(current_language);
}


/*std::string Localized::get_id() const
{
  return id;//get_value(Localized::LANG_ID);
}*/

/** @brief Get the HTML description in the specified language */
std::string Localized::get_description(Language lg) const
{
  std::string default_value = "";
  if(lg == LANG_CURRENT)
    lg = current_language;

  if(descriptions.size() > 0)
    default_value = descriptions[0].second;

  for(unsigned int i = 0; i < descriptions.size(); i++)
  {
    if(descriptions[i].first == lg)
    {
      return descriptions[i].second;
    }
    if(descriptions[i].first == LANG_EN)
      default_value = descriptions[i].second;
  }
  return default_value;
}

bool Localized::has_description() const
{
  std::string s = get_description(LANG_CURRENT);
  return (s.size() > 0);
}

std::vector<Localized::Language> Localized::language_list()
{
  std::vector<Localized::Language> res;

  for(auto i = 0u; i < NLANG; i++)
    res.push_back((Localized::Language) (i + (int) Localized::LANG_FR));

  return res;
}

std::string Localized::language_id(Localized::Language l)
{
  int i = ((int) l) - (int) LANG_FR;
  if(i >= NLANG)
    return "?";
  return lglist[i];
}

Localized::Language Localized::parse_language(std::string id)
{
  for(auto i = 0; i < NLANG; i++)
    if(id.compare(lglist[i]) == 0)
      return (Localized::Language) (((int) LANG_FR) + i);
  return LANG_UNKNOWN;
}

Localized::Localized(const MXml &mx)
{
  if(mx.has_attribute("name"))
    set_value(LANG_ID, mx.get_attribute("name").to_string());
  else if(mx.has_attribute("type"))
    set_value(LANG_ID, mx.get_attribute("type").to_string());

  for(auto i = 0; i < NLANG; i++)
  {
    if(mx.has_attribute(lglist[i])) // e.g. fr, en, etc.
      set_value((Language) ((int)LANG_FR + i),
          mx.get_attribute(lglist[i]).to_string());
  }

  /*if(mx.has_attribute("fr"))
    set_value(LANG_FR, mx.get_attribute("fr").to_string());

  if(mx.has_attribute("en"))
    set_value(LANG_EN, mx.get_attribute("en").to_string());*/

  std::vector<MXml> lst = mx.get_children("description");

  for(unsigned int i = 0; i < lst.size(); i++)
  {
    std::string contents = lst[i].dump_content();

    if(lst[i].has_attribute("lang"))
    {
      set_description(parse_language(lst[i].get_attribute("lang").to_string()),
                      contents);
    }
    else
    {
      set_description(LANG_FR, contents);
    }
  }
}
}


TestUtil::TestUtil(const CmdeLine &cmdeline)
{
  this->cmdeline = cmdeline;
}

TestUtil::TestUtil(const string &module, const string &prg, int argc, const char **argv):
    cmdeline(argc, argv)
{
  utils::init(cmdeline, module, prg);
}

int TestUtil::add_test(const string &name, int (*test_routine)())
{
  TestUnit tu;
  tu.name = name;
  tu.test_routine = test_routine;
  tu.test = nullptr;
  units.push_back(tu);
  return 0;
}

int TestUtil::add_test(const std::string &name, Test *test)
{
  TestUnit tu;
  tu.name = name;
  tu.test_routine = nullptr;
  tu.test = test;
  units.push_back(tu);
  return 0;
}

int TestUtil::check_value(float v, float ref, float precision, const std::string &refname)
{
  // TODO: not true
  if((ref < 0.000001) && (v < 0.000001))
    return 0;

  if(((ref == 0.0) || (ref == -0.0)) && (v == 0.0))
    return 0;

  float err = 100.0 * std::abs((v - ref) / ref);

  if(err > precision)
  {
    erreur("%s: too much error. Value = %f, reference = %f, relative error = %f %%, max relative error = %f %%.", refname.c_str(), v, ref, err, precision);
    return -1;
  }

  return 0;
}

int TestUtil::proceed()
{
  int res = 0;


  if(cmdeline.has_option("--help") || cmdeline.has_option("--usage"))
  {
    cout << "Usage for " << appdata.nom_projet << "/" << appdata.nom_appli << ":" << endl;

    for(uint32_t i = 0; i < units.size(); i++)
    {
      cout << "-t " << (i+1) << ": \033[1m" << units[i].name << "\033[0m..." << endl;
    }

    return 0;
  }


  cout << "Proceeding tests of " << appdata.nom_projet << "/" << appdata.nom_appli << endl;

  float t0 = hal::get_tick_count_us();

  for(uint32_t i = 0; i < units.size(); i++)
  {
    if(cmdeline.has_option("-t"))
    {
      if(cmdeline.get_int_option("-t", 1) != (int) (i + 1))
        continue;
    }

    cout << "-------------------------------\n";
    cout << "Test[" << (i+1) << "/" << units.size() << "] \033[1m" << units[i].name << "\033[0m..." << endl;

    float t00 = hal::get_tick_count_us();

    int test_result;

    if(units[i].test == nullptr)
      test_result = units[i].test_routine();
    else
      test_result = units[i].test->proceed();

    float t01 = hal::get_tick_count_us();

    printf("  ... done, duration = %.2f ms.\n", (t01 - t00) / 1000.0);

    if(test_result)
    {
      cerr << "Test[" << (i+1) << "/" << units.size() << "] \"" << units[i].name << "\": failed." << endl;
      cerr << "Test process aborted." << endl;
      break;
    }
  }



  cout << "All tests done." << endl;

  float t1 = hal::get_tick_count_us();

  printf("\nDuration of the whole tests: %.2f ms.\n", (t1 - t0) / 1000.0);

  uint32_t nb_warnings  = journal::get_warning_count();
  uint32_t nb_anomalies = journal::get_anomaly_count();

  cout << nb_warnings << " warning(s), " << nb_anomalies << " anomalie(s)." << endl;

  if(nb_anomalies > 0)
    res = -1;

  return res;
}

}






