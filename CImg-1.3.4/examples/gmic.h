/*
 #
 #  File        : gmic.h
 #                ( C++ header file )
 #
 #  Description : GREYC's Magic Image Converter
 #                ( http://gmic.sourceforge.net )
 #                This file is a part of the CImg Library project.
 #                ( http://cimg.sourceforge.net )
 #
 #  Note        : Include this file in your C++ source code, if you
 #                want to use the G'MIC interpreter in your own program.
 #
 #  Copyright   : David Tschumperle
 #                ( http://www.greyc.ensicaen.fr/~dtschump/ )
 #
 #  License     : CeCILL v2.0
 #                ( http://www.cecill.info/licences/Licence_CeCILL_V2-en.html )
 #
 #  This software is governed by the CeCILL  license under French law and
 #  abiding by the rules of distribution of free software.  You can  use,
 #  modify and/ or redistribute the software under the terms of the CeCILL
 #  license as circulated by CEA, CNRS and INRIA at the following URL
 #  "http://www.cecill.info".
 #
 #  As a counterpart to the access to the source code and  rights to copy,
 #  modify and redistribute granted by the license, users are provided only
 #  with a limited warranty  and the software's author,  the holder of the
 #  economic rights,  and the successive licensors  have only  limited
 #  liability.
 #
 #  In this respect, the user's attention is drawn to the risks associated
 #  with loading,  using,  modifying and/or developing or reproducing the
 #  software by the user in light of its specific status of free software,
 #  that may mean  that it is complicated to manipulate,  and  that  also
 #  therefore means  that it is reserved for developers  and  experienced
 #  professionals having in-depth computer knowledge. Users are therefore
 #  encouraged to load and test the software's suitability as regards their
 #  requirements in conditions enabling the security of their systems and/or
 #  data to be ensured and,  more generally, to use and operate it in the
 #  same conditions as regards security.
 #
 #  The fact that you are presently reading this means that you have had
 #  knowledge of the CeCILL license and that you accept its terms.
 #
*/

#ifndef gmic_version
#define gmic_version 1341

// Define environment variables.
#ifndef cimg_verbosity
#define cimg_verbosity 1
#endif
#if defined(cimg_build)
#define cimg_plugin "examples/gmic.cpp"
#define cimg_location "../CImg.h"
#elif defined(gmic_build)
#define cimg_plugin "gmic.cpp"
#define cimg_location "./CImg.h"
#endif

// Define the structures used to store images and image lists.
// You image data must be copied in such structures before
// calling the G'MIC interpreter.
#ifdef cimg_location
#include cimg_location

// Define some character codes used for replacement in double quoted strings.
const char _tilde = 25, _lbrace = 26, _rbrace = 28, _comma = 29, _dquote = 30, _arobace = 31;

#else
#include <cstdio>
#include <cstring>

#ifndef cimg_version
namespace cimg_library {

  // Define the G'MIC image class.
  //------------------------------

  template<typename T> struct CImg {
    unsigned int _width;       // Number of image columns (dimension along the X-axis).
    unsigned int _height;      // Number of image lines (dimension along the Y-axis)
    unsigned int _depth;       // Number of image slices (dimension along the Z-axis).
    unsigned int _spectrum;    // Number of image channels (dimension along the C-axis).
    bool _is_shared;           // Tells if the data buffer is also shared by another structure (avoid buffer copy).
    T *_data;                  // Pointer to the first pixel value.

    ~CImg();                                                                          // Destructor.
    CImg():_width(0),_height(0),_depth(0),_spectrum(0),_is_shared(false),_data(0) {}  // Empty constructor.
    CImg<T>& assign(const unsigned int w, const unsigned int h=1,                     // Use to allocate a new image with
                    const unsigned int d=1, const unsigned int s=1);                  // specified dimension.
  };

  // Define the G'MIC image list class.
  //-----------------------------------
  template<typename T> struct CImgList {
    unsigned int _width;           // Number of images in the list.
    unsigned int _allocated_width; // Allocated items in the list (must be >size and equal to a power of 2).
    CImg<T> *_data;                // Pointer to the first image of the list.

    ~CImgList();                                         // Destructor.
    CImgList():_width(0),_allocated_width(0),_data(0) {} // Empty constructor.
    CImgList<T>& assign(const unsigned int n);           // Use to allocate a new image list with specified dimension.
  };
}
#endif  // #ifndef cimg_version
#endif  // #ifdef cimg_location
#define gmic_image cimg_library::CImg
#define gmic_list cimg_library::CImgList

// Define the G'MIC exception class.
//----------------------------------
struct gmic_exception {
  char _message[16384];
  char _command[1024];
  gmic_exception() { *_message = 0; *_command = 0; }
  gmic_exception(const char *const command, const char *const message) {
    if (command) std::strcpy(_command,command); else *_command = 0;
    if (message) std::strcpy(_message,message); else *_message = 0;
  }
  const char *what()    const { return _message; }   // Give the error message returned by the G'MIC interpreter.
  const char *command() const { return _command; }
};

// Define the G'MIC interpreter class.
//------------------------------------
struct gmic {

  // Internal environment variables.
#if cimg_display!=0
  cimg_library::CImgDisplay instant_window[10];
#endif
  gmic_list<char> command_names, command_definitions, scope, stack;
  gmic_list<unsigned int> dowhiles, repeatdones;
  gmic_image<unsigned char> background3d, light3d;
  float pose3d[12], focale3d, light3d_x, light3d_y, light3d_z, specular_light3d, specular_shine3d, _progress, *progress;
  bool is_released, is_debug, is_start, is_double3d, check_elif;
  int verbosity, render3d, renderd3d;
  volatile int _cancel, *cancel;
  unsigned int position;
  char *tmpstr, *_tmpstr;

  // Constructors - Destructors.
  // Use them to run the G'MIC interpreter from your C++ source.
  gmic(const char *const command_line, const char *const custom_commands=0, const bool default_commands=true,
       float *const p_progress=0, int *const p_cancel=0);
  template<typename T> gmic(const int argc, const char *const *const argv, gmic_list<T>& images,
                            const char *const custom_commands=0, const bool default_commands=true,
                            float *const p_progress=0, int *const p_cancel=0);
  template<typename T> gmic(const char *const command_line, gmic_list<T>& images,
                            const char *const custom_commands=0, const bool default_commands=true,
                            float *const p_progress=0, int *const p_cancel=0);
  ~gmic();

  // All functions below should be considered as 'private' and thus, should not be used
  // in your own C++ source code. Use them at your own risk.
  gmic& add_commands(const char *const data_commands);
  gmic& add_commands(std::FILE *const file);
  gmic_image<char> scope2string() const;
  gmic_image<char> scope2string(const gmic_image<unsigned int>& scope_selection) const;

  gmic& assign(const char *const custom_commands=0, const bool default_commands=true,
               float *const p_progress=0, int *const p_cancel=0);

  gmic_image<unsigned int> selection2cimg(const char *const string, const unsigned int indice_max,
                                          const char *const command, const bool is_selection) const;

  gmic_list<char> command_line_to_CImgList(const char *const command_line) const;

  char *selection2string(const gmic_image<unsigned int>& selection,
                         const gmic_list<char>& filenames,
                         const bool display_selection) const;

  template<typename T>
  bool substitute_item(const char *const source, char *const destination, const gmic_list<T>& images,
                       const gmic_list<char>& filenames, const gmic_list<unsigned int>& repeatdones) const;

  const gmic& print(const char *format, ...) const;
  template<typename T>
  const gmic& print(const gmic_list<T>& list, const char *format, ...) const;
  template<typename T>
  const gmic& print(const gmic_list<T>& list, const gmic_image<unsigned int>& scope_selection, const char *format, ...) const;

  const gmic& warning(const char *format, ...) const;
  template<typename T>
  const gmic& warning(const gmic_list<T>& list, const char *format, ...) const;
  template<typename T>
  const gmic& warning(const gmic_list<T>& list, const gmic_image<unsigned int>& scope_selection, const char *format, ...) const;

  const gmic& error(const char *format, ...) const;
  template<typename T>
  const gmic& error(const gmic_list<T>& list, const char *format, ...) const;
  template<typename T>
  const gmic& error(const gmic_list<T>& list, const gmic_image<unsigned int>& scope_selection, const char *format, ...) const;
  template<typename T>
  const gmic& _arg_error(const gmic_list<T>& list, const char *const command, const char *const argument) const;

  const gmic& debug(const char *format, ...) const;
  template<typename T>
  const gmic& debug(const gmic_list<T>& list, const char *format, ...) const;

  template<typename T>
  gmic& display_images(const gmic_list<T>& images,
                       const gmic_list<char>& filenames,
                       const gmic_image<unsigned int>& selection);
  template<typename T>
  gmic& display_plots(const gmic_list<T>& images,
                      const gmic_list<char>& filenames,
                      const gmic_image<unsigned int>& selection,
                      const unsigned int plot_type, const unsigned int vertex_type,
                      const double xmin, const double xmax,
                      const double ymin, const double ymax);
  template<typename T>
  gmic& display_objects3d(const gmic_list<T>& images,
                          const gmic_list<char>& filenames,
                          const gmic_image<unsigned int>& selection);

  template<typename T>
  gmic& parse(const gmic_list<char>& command_line, unsigned int& position,
              gmic_list<T> &images, gmic_list<char> &filenames);
  gmic& parse_bool(const gmic_list<char>& command_line, unsigned int& position,
                   gmic_list<bool>& images, gmic_list<char> &filename);
  gmic& parse_uchar(const gmic_list<char>& command_line, unsigned int& position,
                    gmic_list<unsigned char>& images, gmic_list<char> &filenames);
  gmic& parse_char(const gmic_list<char>& command_line, unsigned int& position,
                   gmic_list<char>& images, gmic_list<char> &filenames);
  gmic& parse_ushort(const gmic_list<char>& command_line, unsigned int& position,
                     gmic_list<unsigned short>& images, gmic_list<char> &filenames);
  gmic& parse_short(const gmic_list<char>& command_line, unsigned int& position,
                    gmic_list<short>& images, gmic_list<char> &filenames);
  gmic& parse_uint(const gmic_list<char>& command_line, unsigned int& position,
                   gmic_list<unsigned int>& images, gmic_list<char> &filenames);
  gmic& parse_int(const gmic_list<char>& command_line, unsigned int& position,
                  gmic_list<int>& images, gmic_list<char> &filenames);
  gmic& parse_float(const gmic_list<char>& command_line, unsigned int& position,
                    gmic_list<float>& images, gmic_list<char> &filenames);
  gmic& parse_double(const gmic_list<char>& command_line, unsigned int& position,
                     gmic_list<double>& images, gmic_list<char> &filenames);

}; // End of the 'gmic' class.

#endif

// Local Variables:
// mode: c++
// End:
