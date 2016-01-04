/**
 *  This file is part of LIBCUTIL.
 *
 *  LIBCUTIL is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  LIBCUTIL is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with LIBCUTIL.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Copyright 2007-2011 J. A.
 */

#ifndef _CUTIL_H
#define	_CUTIL_H

#include "mxml.hpp"
#include "slots.hpp"

#include <string>
#include <stdint.h>


#ifdef BLACKFIN
#define __func__ ""
#define nullptr 0
#endif


/** @brief Portable miscellaneous functions */
namespace utils
{

/** @cond not-documented */
template <class T>
class refcnt
{
public:
  T *ptr;
  int count;
};
/** @endcond */

/** @brief Reference counting shared pointer (until std::shared_ptr supported natively by visual dsp)
 *
 *  TODO: remove and replace by std::shared_ptr (NOT UNTIL VISUAL DSP COMPILER SUPPORT C++11).
 * */
template <typename T>
class refptr
{
public:

  explicit refptr(T *p)
  {
    reference = new refcnt<T>();
    reference->ptr   = p;
    reference->count = 1;
  }

  explicit refptr(const T& t)
  {
    reference = new refcnt<T>();
    reference->ptr   = new T(t);
    reference->count = 1;
  }

  /*explicit refptr(T *p, void (T:: *tell_container)(refcnt<T> *ptr))
  {
    reference = new refcnt<T>();
    reference->ptr   = p;
    reference->count = 1;
    // Tell the contained class its unique container.
    if(tell_container != 0)
    {
      p->tell_container(reference);
    }
  }*/

  refptr()
  {
    reference = nullptr;
  }

  bool is_nullptr() const
  {
    return reference == nullptr;
  }

  bool operator ==(const refptr &r) const
  {
    return reference == r.reference;
  }

  refptr &operator =(const refptr &r)
  {
    if(r.reference != nullptr)
      r.reference->count++;
    if((reference != nullptr) && (r.reference != reference))
    {
      reference->count--;
      if(reference->count <= 0)
      {
        delete reference->ptr;
        delete reference;
        reference = nullptr;
      }
    }
    reference = r.reference;
    return *this;
  }
  refptr(const refptr &r)
  {
    reference = nullptr;
    *(this) = r;
  }
  T *get_reference()
  {
    if(reference == nullptr)
      return nullptr;
    return reference->ptr;
  }
  ~refptr()
  {
    if(reference != nullptr)
    {
      reference->count--;
      if(reference->count == 0)
      {
        if(reference->ptr != nullptr)
        {
          delete reference->ptr;
          reference->ptr = nullptr;
        }
        delete reference;
        reference = nullptr;
      }
    }
  }
  T &operator*()
  {
    return *(reference->ptr);
  }
  const T &operator*() const
  {
    return *(reference->ptr);
  }
  T *operator->()
  {
    return reference->ptr;
  }
  const T *operator->() const
  {
    return reference->ptr;
  }

  //private: // (to uncomment)
  refcnt<T> *reference;

  explicit refptr(refcnt<T> *rcnt)
  {
    reference = rcnt;
    if(reference != nullptr)
      reference->count++;
  }
};


template <typename T>
class uptr
{
public:

  explicit uptr(T *p)
  {
    ptr   = p;
  }

  explicit uptr(const T &p)
  {
    ptr   = new T(p);
  }

  /*explicit refptr(T *p, void (T:: *tell_container)(refcnt<T> *ptr))
  {
    reference = new refcnt<T>();
    reference->ptr   = p;
    reference->count = 1;
    // Tell the contained class its unique container.
    if(tell_container != 0)
    {
      p->tell_container(reference);
    }
  }*/

  uptr()
  {
    ptr = nullptr;
  }

  bool is_nullptr() const
  {
    return ptr == nullptr;
  }

  bool operator ==(const uptr &r) const
  {
    return *(r.ptr) == *ptr;
  }

  uptr &operator =(const uptr &r)
  {
    if(r.ptr != nullptr)
      ptr = new T(*(r.ptr));
    else
      ptr = nullptr;
    return *this;
  }
  uptr(const uptr &r)
  {
    ptr = nullptr;
    *(this) = r;
  }
  T *get_reference()
  {
    if(ptr == nullptr)
      return nullptr;
    return ptr;
  }
  ~uptr()
  {
    if(ptr != nullptr)
    {
        delete ptr;
        ptr = nullptr;
    }
  }
  T &operator*()
  {
    return *ptr;
  }
  const T &operator*() const
  {
    return *ptr;
  }
  T *operator->()
  {
    return ptr;
  }
  const T *operator->() const
  {
    return ptr;
  }

private:
  T *ptr;

  explicit uptr(uptr<T> *rcnt)
  {
    ptr = new T(*(rcnt->ptr));
  }
};


/** @cond not-documented */
template <class T>
class arraycnt
{
public:
  T *ptr;
  int count;
};
/** @endcond */

/** @brief Reference counting shared pointer (until std::shared_ptr supported natively by visual dsp)
 *
 *  TODO: remove and replace by std::shared_ptr (NOT UNTIL VISUAL DSP COMPILER SUPPORT C++11).
 * */
template <typename T>
class arrayptr
{
public:

  explicit arrayptr(T *p)
  {
    reference = new arraycnt<T>();
    reference->ptr   = p;
    reference->count = 1;
  }

  /*explicit refptr(T *p, void (T:: *tell_container)(refcnt<T> *ptr))
  {
    reference = new refcnt<T>();
    reference->ptr   = p;
    reference->count = 1;
    // Tell the contained class its unique container.
    if(tell_container != 0)
    {
      p->tell_container(reference);
    }
  }*/

  arrayptr()
  {
    reference = nullptr;
  }

  bool is_nullptr() const
  {
    return reference == nullptr;
  }

  bool operator ==(const arrayptr &r) const
  {
    return reference == r.reference;
  }

  arrayptr &operator =(const arrayptr &r)
  {
    if(r.reference != nullptr)
      r.reference->count++;
    if((reference != nullptr) && (r.reference != reference))
    {
      reference->count--;
      if(reference->count <= 0)
      {
        delete [] reference->ptr;
        reference->ptr = nullptr;
        delete reference;
        reference = nullptr;
      }
    }
    reference = r.reference;
    return *this;
  }
  arrayptr(const arrayptr &r)
  {
    reference = nullptr;
    *(this) = r;
  }
  T *get_reference()
  {
    if(reference == nullptr)
      return nullptr;
    return reference->ptr;
  }
  ~arrayptr()
  {
    if(reference != nullptr)
    {
      reference->count--;
      if(reference->count == 0)
      {
        if(reference->ptr != nullptr)
        {
          delete [] reference->ptr;
          reference->ptr = nullptr;
        }
        delete reference;
        reference = nullptr;
      }
    }
  }
  T &operator*()
  {
    return *(reference->ptr);
  }
  T *operator->()
  {
    return reference->ptr;
  }
  const T *operator->() const
  {
    return reference->ptr;
  }

  //private: // (to uncomment)
  arraycnt<T> *reference;

  explicit arrayptr(arraycnt<T> *rcnt)
  {
    reference = rcnt;
    if(reference != nullptr)
      reference->count++;
  }
};



namespace model
{

/** @brief A localized string, in UTF 8. */
class Localized
{
public:

  /** Different languages */
  typedef enum LanguageEnum
  {
    /** Unlocalized identifier, used for identification in the code */
    LANG_ID = 0,

    /** Currently configured language */
    LANG_CURRENT = 1,

    /** French */
    LANG_FR   = 2,

    /** English */
    LANG_EN   = 3,

    /** German */
    LANG_DE   = 4,

    /** Russian */
    LANG_RU   = 5,

    /** Unspecified language */
    LANG_UNKNOWN = 255
  } Language;

  Localized();
  Localized(const Localized &l);
  Localized(const utils::model::MXml &mx);
  void operator =(const Localized &l);
  void set_value(Language lg, std::string value);
  void set_description(Language lg, std::string desc);

  /** @brief Get the value in the specified language (by default "ID language") */
  std::string get_value(Language lg = LANG_ID) const;

  /** @brief Equivalent to get_value(LANG_CURRENT) */
  std::string get_localized() const;

  /** @brief Equivalent to get_value(LANG_ID) */
  inline std::string get_id() const  {return id;}

  /** @brief Get the HTML description in the specified language */
  std::string get_description(Language lg = LANG_CURRENT) const;

  bool has_description() const;

  /** @brief Parse language string ("fr" -> LANG_FR, "en" -> LANG_EN, ...) */
  static Language parse_language(std::string id);
  static std::string language_id(Language l);
  static std::vector<Language> language_list();

  std::string to_string() const;
private:
  /** @brief The different values in different languages, in UTF8 format. */
  std::vector<std::pair<Language, std::string> > items;
  /** @brief The different descriptions in different languages, in UTF8 format. */
  std::vector<std::pair<Language, std::string> > descriptions;

  std::string id;
};
}





/** @brief Command line argument parsing */
class CmdeLine
{
public:
  CmdeLine(const std::string args);
  CmdeLine(int argc, const char **argv);
  CmdeLine(int argc, char **argv);
  CmdeLine();
  void operator =(const CmdeLine &cmdeline);

  bool has_option(const std::string &name)  const;
  std::string get_option(const std::string &name,
                         const std::string &default_value = "") const;
  int get_int_option(const std::string &name,
                     int default_value)  const;
  void set_option(const std::string &name, const std::string &value);
private:

  void init(std::vector<std::string> &lst);
  void init(int argc, const char **argv);

  class CmdeLinePrm
  {
  public:
    std::string option;
    std::string value;
  };
  std::deque<CmdeLinePrm> prms;
public:
  //int argc;
  std::string argv0;
};


/** @brief Initialise paths and analyse standard command line options.
 *  Options managed:
 *  - [-v]          : enable debug trace on stdout
 *  - [-L fr|en|de] : specify current language
 *  - [-q]          : disable log files.
 *
 *  @param cmdeline  Command line arguments.
 *  @param project   Name of the project (used to define the application data folders).
 *  @param app       Name of the application */
extern void init(CmdeLine &cmde_line,
                 const std::string &project,
                 const std::string &app);


/** @brief Read an environment variable */
extern std::string get_env_variable(const std::string &name, const std::string &default_value = "");

/** @brief Path to fixed data (same as exec path for windows, /usr/share/appname for linux) */
extern std::string get_fixed_data_path();

/** @brief Get the path to the current user parameters directory (e.g. C:\\Doc and Settings\\CURRENT_USER\\AppData\\AppName) */
extern std::string get_current_user_path();

/** @brief Get the path to path the all user parameters directory (e.g. C:\\Doc and Settings\\All Users\\AppData\\AppName) */
extern std::string get_all_user_path();

/** @brief Path to local images (returns img sub-folder of fixed data path. */
extern std::string get_img_path();

/** @brief Return the path containing the currently executed module . */
extern std::string get_execution_path();

extern std::string get_current_date_time();

/** Execute a system command, block until it is finished. */
extern int proceed_syscmde(std::string cmde, ...);

/** Execute a system command, returns immediatly. */
extern int proceed_syscmde_bg(std::string cmde, ...);


/** @brief Static utilities functions */
class Util
{
public:
#ifndef TESTCLI  
  static void show_error(std::string title, std::string content);
  static void show_warning(std::string title, std::string content);
#endif  
  static uint32_t extract_bits(uint8_t *buffer, uint32_t offset_in_bits, uint32_t nbits);
};


namespace str
{
  std::string to_latex(const std::string s);
  std::string str_to_cst(std::string name);
  std::string str_to_var(std::string name);
  std::string str_to_class(std::string name);
  std::string str_to_file(std::string name);
  /** Returns an abbridged version of the specified file path (for display purpose) */
  std::string get_filename_resume(const std::string &filename);

  bool is_deci(char c);
  bool is_hexa(char c);

  /** @brief Convert an integer to "x bytes", "x kbi", "x mbi", "y gbi" */
  std::string int2str_capacity(uint64_t val, bool truncate = false);
  std::string int2str(int i);
  std::string int2str(int i, int nb_digits);
  std::string int2strhexa(int i);
  std::string int2strhexa(int i, int nbits);
  std::string uint2strhexa(int i);
  std::string int2strasm(int i);
  std::string xmlAtt(std::string name, std::string val);
  std::string xmlAtt(std::string name, int val);
  std::string xmlAtt(std::string name, bool val);

  std::string latin_to_utf8(std::string s_);
  std::string utf8_to_latin(std::string s);
  std::string lowercase(std::string s);
  int parse_int_list(const std::string s, std::vector<int> &res);
  int parse_string_list(const std::string s, std::vector<std::string> &res, char separator);
  int parse_hexa_list(const std::string s, std::vector<unsigned char> &res);

  void encode_str(std::string str, std::vector<unsigned char> vec);
  void encode_byte_array_deci(std::string str, std::vector<unsigned char> vec);
  void encode_byte_array_hexa(std::string str, std::vector<unsigned char> vec);

  std::string unix_path_to_win_path(std::string s_);
}



namespace files
{
  bool file_exists(std::string name);
  bool dir_exists(std::string name);
  int copy_file(std::string target, std::string source);
  int create_directory(std::string path);
  /** @brief If the directory does not exist, try to create it. */
  int check_and_build_directory(std::string path);
  void split_path_and_filename(const std::string complete_filename, std::string &path, std::string &filename);
  std::string get_path_separator();
  std::string correct_path_separators(std::string s);
  std::string build_absolute_path(const std::string absolute_origin, const std::string relative_path);
  void delete_file(std::string name);
  int save_txt_file(std::string filename, std::string content);

  /** @brief Retrieve file extension from a file path
   *  Example: from "a.jpg", returns "jpg". */
  std::string get_extension(const std::string &filepath);
  std::string remove_extension(const std::string &filepath);

  int parse_filepath(const std::string &path,
      std::vector<std::string> &items);

  int abs2rel(const std::string &ref,
                     const std::string &abs,
                     std::string &result);

  int rel2abs(const std::string &ref,
                     const std::string &rel,
                     std::string &result);
}





class TextAlign
{
public:
  TextAlign();
  void add(std::string comment);
  void add(std::string s1, std::string s2);
  void add(std::string s1, std::string s2, std::string s3);
  std::string get_result();
private:
  std::vector<std::string > alst1;
  std::vector<std::string > alst2;
  std::vector<std::string > alst3;
  std::vector<std::string > comments;
  std::vector<unsigned int> comments_pos;
};





class TextMatrix
{
public:
  TextMatrix(uint32_t ncols);
  void add(std::string s);
  void add_unformatted_line(std::string s);
  void next_line();
  std::string get_result();
  void reset(uint32_t ncols);
private:
  uint32_t ncols;
  std::vector<std::string> current_row;
  std::vector<std::vector<std::string> > lst;
  std::vector<bool> unformatted;
};


// TODO: rename
class Section
{
public:
  Section();
  Section(const Section &c);
  ~Section();
  void operator =(const Section &c);
  bool has_item(std::string name);
  std::string get_item(std::string name);
  const char *get_text(std::string name);
  Section get_section(std::string name);
  std::string current_language;
  void load();
  void load(std::string filename);
private:
  Section(const utils::model::MXml &data);
  utils::model::MXml data;
};







/** @brief Test framework */
class TestUtil
{
public:
  TestUtil(const utils::CmdeLine &cmdeline);
  TestUtil(const std::string &module, const std::string &prg, int argc, const char **argv);
  int add_test(const std::string &name, int (*test_routine)());
  int proceed();

  /** @param precision: relative precision, in percent. */
  static int check_value(float v, float ref, float precision = 0.0, const std::string &refname = "");

private:
  class TestUnit
  {
  public:
    std::string name;
    int (*test_routine)();
  };
  std::vector<TestUnit> units;
  utils::CmdeLine cmdeline;
};


/** @brief Command line, as defined by the application */
class CmdeLineDefinition
{
public:
  /** @brief Load the command line definition from an XML string.
   *
   *
   *  // PB: temps de chargement std-schema...
   *  <schema name="cmdeline-spec" root="cmdeline-spec">
   *    <node name="cmdeline-spec">
   *      <sub-node name="arg" inherits="attribute"/>
   *    </node>
   *  </schema>
   *
   *
   *  Example:
   *  <cmdeline>
   *    <arg name="i" mandatory="true" fr="Fichier d'entrée">
   *      <description lang="fr">Chemin complet du fichier d'entrée</description>
   *    </arg>
   *  </cmdeline>
   */
  int from_xml(const std::string &xml_definition);

  /** @brief Display usage info on the specified output stream */
  void usage(std::ostream &out) const;

  /** @returns 0 if cmdeline is valid, otherwise display an error message.
   *  also, recognize and manage --help and --usage commands, and in this case,
   *  exits silently. */
  int check_cmdeline(const CmdeLine &cmdeline) const;

  /** exit(-1) if cmdeline is not valid. */
  void check_cmdeline_and_fail(const CmdeLine &cmdeline) const;

private:

  class Prm
  {
  public:
    model::Localized name;
    enum type
    {
      TP_FILE   = 0, // file path
      TP_EXFILE = 1, // existing file path
      TP_INT    = 2, // integer
      TP_STRING = 3  // string
    };
    bool mandatory;
    bool has_default;
    std::string default_value;
    std::string value;
  };

  std::vector<Prm> prms;
};



template<class VT, class T>
  bool contains(const VT &container, const T &t)
{
  for(const T &t2: container)
  {
    if(t2 == t)
      return true;
  }
  return false;
}

#ifdef LINUX
# define PATH_SEP "/"
#else
# define PATH_SEP "\\"
#endif


# ifdef SAFE_MODE
# define assert_safe(EE) assert((EE))
# else
# define assert_safe(EE)
# endif

extern Section langue;

}

#include "templates/cutil-private.hpp"


#endif	/* _CUTIL_H */

