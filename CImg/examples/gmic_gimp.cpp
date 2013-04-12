/*
 #
 #  File        : gmic_gimp.cpp
 #                ( C++ source file )
 #
 #  Description : G'MIC for GIMP - A plug-in to allow the use
 #                of G'MIC commands in GIMP.
 #                This file is a part of the CImg Library project.
 #                ( http://cimg.sourceforge.net )
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
 #  data to be ensured and, more generally, to use and operate it in the
 #  same conditions as regards security.
 #
 #  The fact that you are presently reading this means that you have had
 #  knowledge of the CeCILL license and that you accept its terms.
 #
*/

// Include necessary header files.
//--------------------------------
#define cimg_display_type 0
#include "gmic.h"
#undef _gmic_path
#if cimg_OS==2
#define _gmic_path "_gmic\\"
#else
#define _gmic_path ""
#endif
#if !defined(__MACOSX__)  && !defined(__APPLE__)
#include <pthread.h>
#endif
#include <locale>
#include <gtk/gtk.h>
#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>
#undef min
#undef max
extern char data_gmic_def[];
extern unsigned int size_data_gmic_def;
extern unsigned char data_gmic_logo[];
extern unsigned int size_data_gmic_logo;
using namespace cimg_library;

// Define plug-in global variables.
//---------------------------------
CImgList<char> gmic_entries;             // The list of recognized G'MIC menu entries.
CImgList<char> gmic_1stlevel_entries;    // The treepath positions of 1st-level G'MIC menu entries.
CImgList<char> gmic_commands;            // The list of corresponding G'MIC commands to process the image.
CImgList<char> gmic_preview_commands;    // The list of corresponding G'MIC commands to preview the image.
CImgList<char> gmic_arguments;           // The list of corresponding needed filter arguments.
CImgList<double> gmic_preview_factors;   // The list of default preview factors for each filter.
bool _create_dialog_gui;                 // Return value of the 'create_gui_dialog()' function (set by events handlers).
void **event_infos;                      // Infos that are passed to the GUI callback functions.
char *gmic_custom_commands = 0;          // The array of custom G'MIC commands.
int image_id = 0;                        // The image concerned by the plug-in execution.
GimpRunMode run_mode;                    // Run-mode used to call the plug-in.
GtkTreeStore *tree_view_store = 0;       // The list of the filters as a GtkTreeView model.
GimpDrawable *drawable_preview = 0;      // The drawable used by the preview window.
GtkWidget *dialog_window = 0;            // The plug-in dialog window.
GtkWidget *left_pane  = 0;               // The left pane, containing the preview window.
GtkWidget *gui_preview = 0;              // The preview window.
GtkWidget *tree_mode_stockbutton = 0;    // A temporary stock button for the expand/collapse button.
GtkWidget *tree_mode_button = 0;         // Expand/Collapse button for the treeview.
GtkWidget *right_frame = 0;              // The right frame containing the filter parameters.

#define gmic_xstr(x) gmic_str(x)
#define gmic_str(x) #x
#define gmic_update_server "http://gmic.sourceforge.net/"     // The filters update server address.
#define gmic_update_file "gmic_def." gmic_xstr(gmic_version)  // The filters update filename.

// Set/get plug-in persistent variables, using GIMP {get,set}_data() features.
//-----------------------------------------------------------------------------

// Set/get the indice of the currently selected filter.
void set_current_filter(const unsigned int current_filter) {
  const unsigned int ncurrent_filter = current_filter>gmic_entries.size()?0:current_filter;
  gimp_set_data("gmic_current_filter",&ncurrent_filter,sizeof(unsigned int));
}

unsigned int get_current_filter() {
  unsigned int current_filter = 0;
  gimp_get_data("gmic_current_filter",&current_filter);
  if (current_filter>gmic_entries.size()) current_filter = 0;
  return current_filter;
}

// Set/get the number of parameters of the specified filter.
void set_filter_nbparams(const unsigned int filter, const unsigned int nbparams) {
  char s_tmp[256] = { 0 };
  std::sprintf(s_tmp,"gmic_filter%u_nbparams",filter);
  gimp_set_data(s_tmp,&nbparams,sizeof(unsigned int));
}

unsigned int get_filter_nbparams(const unsigned int filter) {
  char s_tmp[256] = { 0 };
  std::sprintf(s_tmp,"gmic_filter%u_nbparams",filter);
  unsigned int nbparams = 0;
  gimp_get_data(s_tmp,&nbparams);
  return nbparams;
}

// Set/get one particular parameter of a filter.
void set_filter_parameter(const unsigned int filter, const unsigned int n, const char *const param) {
  char s_tmp[256] = { 0 };
  std::sprintf(s_tmp,"gmic_filter%u_parameter%u",filter,n);
  gimp_set_data(s_tmp,param,std::strlen(param)+1);
}

char *get_filter_parameter(const unsigned int filter, const unsigned int n) {
  char s_tmp[256] = { 0 };
  std::sprintf(s_tmp,"gmic_filter%u_parameter%u",filter,n);
  static char s_param[4096] = { 0 };
  s_param[0] = 0; gimp_get_data(s_tmp,s_param);
  return s_param;
}

// Reset all parameters of all filters.
void reset_filters_parameters() {
  const char empty[] = { 0 };
  for (unsigned int i = 1; i<gmic_entries.size(); ++i)
    for (unsigned int j = 0; ; ++j) {
      const char *const val = get_filter_parameter(i,j);
      if (*val) set_filter_parameter(i,j,empty); else break;
    }
}

// Set/get the filter input, output and preview and verbosity modes.
void set_input_mode(const unsigned int input_mode) {
  gimp_set_data("gmic_input_mode",&input_mode,sizeof(unsigned int));
}

unsigned int get_input_mode(const bool normalized=true) {
  unsigned int input_mode = 0;
  gimp_get_data("gmic_input_mode",&input_mode);
  return normalized?(input_mode<2?1:(input_mode-2)):input_mode;
}

void set_output_mode(const unsigned int output_mode) {
  gimp_set_data("gmic_output_mode",&output_mode,sizeof(unsigned int));
}

unsigned int get_output_mode(const bool normalized=true) {
  unsigned int output_mode = 0;
  gimp_get_data("gmic_output_mode",&output_mode);
  return normalized?(output_mode<2?0:(output_mode-2)):output_mode;
}

void set_preview_mode(const unsigned int preview_mode) {
  gimp_set_data("gmic_preview_mode",&preview_mode,sizeof(unsigned int));
}

unsigned int get_preview_mode(const bool normalized=true) {
  unsigned int preview_mode = 0;
  gimp_get_data("gmic_preview_mode",&preview_mode);
  return normalized?(preview_mode<2?0:(preview_mode-2)):preview_mode;
}

void set_verbosity_mode(const unsigned int verbosity) {
  gimp_set_data("gmic_verbosity_mode",&verbosity,sizeof(unsigned int));
}

unsigned int get_verbosity_mode(const bool normalized=true) {
  unsigned int verbosity_mode = 0;
  gimp_get_data("gmic_verbosity_mode",&verbosity_mode);
  return normalized?(verbosity_mode<2?0:(verbosity_mode-2)):verbosity_mode;
}

// Set/get the tree collapse/expand mode.
void set_tree_mode(const bool expand) {
  gimp_set_data("gmic_tree_mode",&expand,sizeof(bool));
}

bool get_tree_mode() {
  bool tree_mode = false;
  gimp_get_data("gmic_tree_mode",&tree_mode);
  return tree_mode;
}

// Set/get the net update activation state.
void set_net_update(const bool net_update) {
  gimp_set_data("gmic_net_update",&net_update,sizeof(bool));
}

bool get_net_update() {
  bool net_update = true;
  gimp_get_data("gmic_net_update",&net_update);
  return net_update;
}

// Set/get the current locale.
void set_locale() {
  char locale[8] = { 0 };
  std::sscanf(std::setlocale(LC_ALL,0),"%c%c",&(locale[0]),&(locale[1]));
  cimg::uncase(locale);
  gimp_set_data("gmic_locale",locale,std::strlen(locale)+1);
}

const char *get_locale() {
  static char res[256] = { 0 };
  res[0] = 0;
  gimp_get_data("gmic_locale",res);
  return res;
}

// Translate string into the current locale.
//------------------------------------------
#define _t(source,dest) if (!std::strcmp(source,s)) { static const char *const ns = dest; return ns; }
const char *t(const char *const s) {

  // French translation
  if (!std::strcmp(get_locale(),"fr")) {
    if (!s) {
      static const char *const ns = "<b>Mise &#224; jour depuis Internet impossible !</b>\n\n"
        "Merci de v&#233;rifier l'&#233;tat de votre connexion. Vous pouvez\n"
        "manuellement mettre &#224; jour vos filtres en t&#233;l&#233;chargeant :\n\n"
        "<u><small>%s%s</small></u>\n\n"
        "et en le copiant comme le fichier <i>.%s</i>\n"
        "dans votre r&#233;pertoire <i>Home</i> ou <i>Application Data</i>.";
      return ns;
    }
    _t("G'MIC for GIMP","G'MIC pour GIMP");
    _t("<i>Select a filter...</i>","<i>Choisissez un filtre...</i>");
    _t("<i>No parameters to set...</i>","<i>Pas de param&#232;tres...</i>");
    _t("<b> Input / Output : </b>","<b> Entr&#233;es / Sorties : </b>");
    _t("Input layers...","Calques d'entr\303\251e...");
    _t("None","Aucun");
    _t("Active (default)","Actif (d\303\251faut)");
    _t("All","Tous");
    _t("Active & below","Actif & en dessous");
    _t("Active & above","Actif & au dessus");
    _t("All visibles","Tous les visibles");
    _t("All invisibles","Tous les invisibles");
    _t("All visibles (decr.)","Tous les visibles (d\303\251cr.)");
    _t("All invisibles (decr.)","Tous les invisibles (d\303\251cr.)");
    _t("All (decr.)","Tous (d\303\251cr.)");
    _t("Output mode...","Mode de sortie...");
    _t("In place (default)","Sur place (d\303\251faut)");
    _t("New layer(s)","Nouveau(x) calque(s)");
    _t("New active layer(s)","Nouveau(x) calque(s) actifs");
    _t("New image","Nouvelle image");
    _t("Output preview...","Mode d'aper\303\247u...");
    _t("1st output (default)","1\303\250re image (d\303\251faut)");
    _t("2nd output","2\303\250me image");
    _t("3rd output","3\303\250me image");
    _t("4th output","4\303\250me image");
    _t("1st -> 2nd","1\303\250re -> 2\303\250me");
    _t("1st -> 3rd","1\303\250re -> 3\303\250me");
    _t("1st -> 4th","1\303\250re -> 4\303\250me");
    _t("All outputs","Toutes les images");
    _t("Output messages...","Messages de sortie...");
    _t("Quiet (default)","Aucun message (d\303\251faut)");
    _t("Verbose","Mode verbeux");
    _t("Very verbose","Mode tr\303\250s verbeux");
    _t("Debug mode","Mode d\303\251bogage");
    _t("Internet updates","Mises \303\240 jour Internet");
    _t(" Available filters (%u) :"," Filtres disponibles (%u) :");
    _t("Maximize prev_iew","Max_imiser l'aper\303\247u");
    _t("Restore prev_iew","R\303\251du_ire l'aper\303\247u");
    _t("_Manual preview","Aper\303\247u _manuel");
  }

  // Catalan translation
  if (!std::strcmp(get_locale(),"ca")) {
    if (!s) {
      static const char *const ns =
        "<b>No ha estat possible establir una connexi&#243; a Internet !</b>\n\n"
        "Verifiqueu l'estat de la vostra connexi&#243;. Podeu\n"
        "actualitzar els vostres filtres descarregant-vos:\n\n"
        "<u><small>%s%s</small></u>\n\n"
        "i copiant-lo com a <i>.%s</i>\n"
        "a la vostra carpeta d'<i>inici</i> o la carpeta de <i>Dades de Programa</i>.";
      return ns;
    }
    _t("G'MIC for GIMP","G'MIC per al GIMP");
    _t("<i>Select a filter...</i>","<i>Selecciona un filtre...</i>");
    _t("<i>No parameters to set...</i>","<i>Sense par\303\240metres...</i>");
    _t("<b> Input / Output : </b>","<b> Entrades / Sortides : </b>");
    _t("Input layers...","Capes d'entrada...");
    _t("None","Cap");
    _t("Active (default)","Actiu (predet.)");
    _t("All","Tots");
    _t("Active & below","L'activa i les de sota");
    _t("Active & above","L'activa i les de sobre");
    _t("All visibles","Totes les visibles");
    _t("All invisibles","Totes les invisibles");
    _t("All visibles (decr.)","Totes les visibles (decr.)");
    _t("All invisibles (decr.)","Totes les invisibles (decr.)");
    _t("All (decr.)","Totes (decr.)");
    _t("Output mode...","Mode de sortida...");
    _t("In place (default)","A la capa actual (predet.)");
    _t("New layer(s)","Nova/es capa/es");
    _t("New active layer(s)","Nova/es capa/es actius");
    _t("New image","Nova imatge");
    _t("Output preview...","Previsualitzaci\303\263 de sortida...");
    _t("1st output (default)","1era imatge (predet.)");
    _t("2nd output","2ona imatge");
    _t("3rd output","3era imatge");
    _t("4th output","4rta imatge");
    _t("1st -> 2nd","1era -> 2ona");
    _t("1st -> 3rd","1era -> 3era");
    _t("1st -> 4th","1era -> 4rta");
    _t("All outputs","Totes les imatges");
    _t("Output messages...","Missatges de sortida...");
    _t("Quiet (default)","Sense missatges (predet.)");
    _t("Verbose","Verb\303\263s");
    _t("Very verbose","Molt verb\303\263s");
    _t("Debug mode","Depuraci\303\263");
    _t("Internet updates","Actualitzacions per Internet");
    _t(" Available filters (%u) :"," Filtres disponibles (%u) :");
    _t("Maximize prev_iew","Maximitzar la prev_isualitzaci\303\263");
    _t("Restore prev_iew","Restaurar la prev_isualitzaci\303\263");
    _t("_Manual preview","Previsualitzaci\303\263 _manual");
  }

  // Italian translation
  if (!std::strcmp(get_locale(),"it")) {
    if (!s) {
      static const char *const ns = "<b>Impossibile aggiornare da Internet !</b>\n\n"
        "Controllate lo stato della vostra connessione. Potete anche\n"
        "aggiornare manualmente i filtri scaricando :\n\n"
        "<u><small>%s%s</small></u>\n\n"
        "e copiandoli come il file <i>.%s</i>\n"
        "nella directory <i>Home</i> o <i>Application Data</i>.";
      return ns;
    }
    _t("G'MIC for GIMP","G'MIC per GIMP");
    _t("<i>Select a filter...</i>","<i>Sciegliete un Filtro...</i>");
    _t("<i>No parameters to set...</i>","<i>Filtro senza Parametri...</i>");
    _t("<b> Input / Output : </b>","<b> Input / Output : </b>");
    _t("Input layers...","Input da Layers...");
    _t("None","Nessuno");
    _t("Active (default)","Layer Attivo (default)");
    _t("All","Tutti");
    _t("Active & below","Attivo & superiori");
    _t("Active & above","Attivo & inferiori");
    _t("All visibles","Tutti i Visibili");
    _t("All invisibles","Tutti gli invisibili");
    _t("All visibles (decr.)","Tutti i visibili (dal fondo)");
    _t("All invisibles (decr.)","Tutti gli invisibili (dal fondo)");
    _t("All (decr.)","Tutti");
    _t("Output mode...","Tipo di output...");
    _t("In place (default)","Applica al Layer attivo (default) ");
    _t("New layer(s)","Nuovo(i) Layer(s)");
    _t("New active layer(s)","Attiva Nuovo(i) Layer(s)");
    _t("New image","Nuova Immagine");
    _t("Output preview...","Anteprima...");
    _t("1st output (default)","Primo Output (default)");
    _t("2nd output","Secondo Output");
    _t("3rd output","Terzo Output");
    _t("4th output","Quarto Output");
    _t("1st -> 2nd","1 -> 2");
    _t("1st -> 3rd","1 -> 3");
    _t("1st -> 4th","1 -> 4");
    _t("All outputs","Tutti i layers");
    _t("Output messages...","Messaggi di Output...");
    _t("Quiet (default)","Nessun Messaggio (default)");
    _t("Verbose","Verbose");
    _t("Very verbose","Messaggi Dettagliati");
    _t("Debug mode","Debug Mode");
    _t("Internet updates","Aggiornamento via Internet");
    _t(" Available filters (%u) :"," Filtri disponibili (%u) :");
    _t("Maximize prev_iew","Maximize prev_iew");
    _t("Restore prev_iew","Restore prev_iew");
    _t("_Manual preview","Preview _manuale");
  }

  // German translation
  if (!std::strcmp(get_locale(),"de")) {
    if (!s) {
      static const char *const ns = "<b>Kein Internet-Update m\303\266glich !</b>\n\n"
        "Bitte \303\274berpr\303\274fen Sie Ihre Verbindung. Sie k\303\266nnen\n"
        "ein manuelles Update vornehmen durch Herunterladen der Datei :\n\n"
        "<u><small>%s%s</small></u>\n\n"
        "Danach kopieren Sie die Datei <i>.%s</i>\n"
        "in Ihren Anwendungsdaten-Order.";
      return ns;
    }
    _t("G'MIC for GIMP","G'MIC f\303\274r GIMP");
    _t("<i>Select a filter...</i>","<i>W\303\244hlen Sie einen Filter...</i>");
    _t("<i>No parameters to set...</i>","<i>Keine w\303\244hlbaren Parameter...</i>");
    _t("<b> Input / Output : </b>","<b> Eingabe / Ausgabe : </b>");
    _t("Input layers...","Eingabeebenen...");
    _t("None","Keine");
    _t("Active (default)","Aktive (Standard)");
    _t("All","Alle");
    _t("Active & below","Aktive & darunterliegende");
    _t("Active & above","Aktive & dar\303\274berliegende");
    _t("All visibles","Alle sichtbaren");
    _t("All invisibles","Alle nicht sichtbaren");
    _t("All visibles (decr.)","Alle sichtbaren (absteigend)");
    _t("All invisibles (decr.)","Alle nicht sichtbaren (absteigend)");
    _t("All (decr.)","Alle (absteigend)");
    _t("Output mode...","Ausgabemodus...");
    _t("In place (default)","Bestehende ersetzen (standard)");
    _t("New layer(s)","Neue Ebene(n)");
    _t("New active layer(s)","Neue aktive Ebene(n)");
    _t("New image","Neues Bild");
    _t("Output preview...","Ausgabevorschau...");
    _t("1st output (default)","1. Ausgabe (Standard)");
    _t("2nd output","2. Ausgabe");
    _t("3rd output","3. Ausgabe");
    _t("4th output","4. Ausgabe");
    _t("1st -> 2nd","1. -> 2.");
    _t("1st -> 3rd","1. -> 3.");
    _t("1st -> 4th","1. -> 4.");
    _t("All outputs","Alle Ausgaben");
    _t("Output messages...","Ausgabemeldungen...");
    _t("Quiet (default)","Keine Meldung (Standard)");
    _t("Verbose","Ausf\303\274hrlich");
    _t("Very verbose","Sehr ausf\303\274hrlich");
    _t("Debug mode","Debug-Modus");
    _t("Internet updates","Internet-Updates");
    _t(" Available filters (%u) :"," Verf\303\274gbare Filter (%u) :");
    _t("Maximize prev_iew","Max_imize Vorschau");
    _t("Restore prev_iew","Restore Vorschau");
    _t("_Manual preview","_Manuelle Vorschau");
  }

  // Dutch translation
  if (!std::strcmp(get_locale(),"nl")) {
    if (!s) {
      static const char *const ns = "<b>Geen internet-update mogelijk !</b>\n\n"
        "Controleer aub je verbinding. Je kunt handmatig bijwerken door het downloaden van het volgende bestand :\n\n"
        "<u><small>%s%s</small></u>\n\n"
        "Vervolgens kopieer je het bestand naar folder met gebruikersinformatie.";
      return ns;
    }
    _t("G'MIC for GIMP","G'MIC voor GIMP");
    _t("<i>Select a filter...</i>","<i>Kies een filter...</i>");
    _t("<i>No parameters to set...</i>","<i>Geen parameters nodig...</i>");
    _t("<b> Input / Output : </b>","<b> Input / Output : </b>");
    _t("Input layers...","Input lagen...");
    _t("None","Geen");
    _t("Active (default)","Actieve laag (standaard)");
    _t("All","Alle");
    _t("Active & below","Actieve & onderliggende");
    _t("Active & above","Actieve & bovenliggende");
    _t("All visibles","Alle zichtbare");
    _t("All invisibles","Alle niet zichtbare");
    _t("All visibles (decr.)","Alle zichtbare (afnemend)");
    _t("All invisibles (decr.)","Alle niet zichtbare (afnemend)");
    _t("All (decr.)","Alle (afnemend)");
    _t("Output mode...","Output mode...");
    _t("In place (default)","Vervang bestaande (standaard)");
    _t("New layer(s)","Nieuwe laag/lagen");
    _t("New active layer(s)","Nieuwe actieve laag/lagen");
    _t("New image","Nieuwe afbeelding");
    _t("Output preview...","Output voorbeeld...");
    _t("1st output (default)","1e Resultaat (standaard)");
    _t("2nd output","2e Resultaat");
    _t("3rd output","3e Resultaat");
    _t("4th output","4e Resultaat");
    _t("1st -> 2nd","1e -> 2e");
    _t("1st -> 3rd","1e -> 3e");
    _t("1st -> 4th","1e -> 4e");
    _t("All outputs","Alle resultaten");
    _t("Output messages...","Output berichten...");
    _t("Quiet (default)","Geen melding (standaard)");
    _t("Verbose","Uitgebreid");
    _t("Very verbose","Heel uitgebreid");
    _t("Debug mode","Debug mode");
    _t("Internet updates","Internet updates");
    _t(" Available filters (%u) :"," Beschikbare filters (%u) :");
    _t("Maximize prev_iew","Max_imaliseren voorbeeld");
    _t("Restore prev_iew","Verm_indering voorbeeld");
    _t("_Manual preview","Hand_matig voorbeeld");
  }

  // English translation (default)
  if (!s) {
    static const char *const ns = "<b>Filters update from Internet failed !</b>\n\n"
      "Please check your Internet connection. You can\n"
      "manually update your filters by downloading :\n\n"
      "<u><small>%s%s</small></u>\n\n"
      "and copy it as the file <i>.%s</i>\n"
      "in your <i>Home</i> or <i>Application Data</i> folder.";
    return ns;
  }
  return s;
}

// Flush filter tree view
//------------------------
void flush_tree_view(GtkWidget *const tree_view) {
  const unsigned int filter = get_current_filter();
  bool tree_mode = get_tree_mode();
  char current_path[256] = { 0 };
  unsigned int current_dir = 0;
  gimp_get_data("gmic_current_treepath",&current_path);

  if (tree_mode) { // Expand
    cimglist_for(gmic_1stlevel_entries,l) {
      GtkTreePath *path = gtk_tree_path_new_from_string(gmic_1stlevel_entries[l].data());
      gtk_tree_view_expand_row(GTK_TREE_VIEW(tree_view),path,false);
      gtk_tree_path_free(path);
    }
  } else { // Collapse
    if (filter && *current_path && std::sscanf(current_path,"%u",&current_dir)==1) {
      cimglist_for(gmic_1stlevel_entries,l) {
        const char *const s_path = gmic_1stlevel_entries[l].data();
        unsigned int dir = 0;
        if (std::sscanf(s_path,"%u",&dir)!=1 || dir!=current_dir) {
          GtkTreePath *path = gtk_tree_path_new_from_string(gmic_1stlevel_entries[l].data());
          gtk_tree_view_collapse_row(GTK_TREE_VIEW(tree_view),path);
          gtk_tree_path_free(path);
        }
      }
    } else gtk_tree_view_collapse_all(GTK_TREE_VIEW(tree_view));
  }

  if (filter && *current_path) {
    GtkTreePath *path = gtk_tree_path_new_from_string(current_path);
    gtk_tree_view_expand_to_path(GTK_TREE_VIEW(tree_view),path);
    gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(tree_view),path,NULL,FALSE,0,0);
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_view));
    gtk_tree_selection_select_path(selection,path);
    gtk_tree_path_free(path);
  }

  if (tree_mode_stockbutton) gtk_widget_destroy(tree_mode_stockbutton);
  tree_mode_stockbutton = gtk_button_new_from_stock(tree_mode?GTK_STOCK_ZOOM_OUT:GTK_STOCK_ZOOM_IN);
  GtkWidget *tree_image = gtk_button_get_image(GTK_BUTTON(tree_mode_stockbutton));
  gtk_button_set_image(GTK_BUTTON(tree_mode_button),tree_image);
  gtk_widget_show(tree_mode_button);

  gtk_tree_view_remove_column(GTK_TREE_VIEW(tree_view),gtk_tree_view_get_column(GTK_TREE_VIEW(tree_view),0));
  GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
  char tree_view_title[256] = { 0 };
  std::sprintf(tree_view_title,t(" Available filters (%u) :"),gmic_entries.size());
  GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes(tree_view_title,renderer,"markup",1,NULL);

  gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view),column);
}

// Update filter definitions : retrieve files and update treeview.
//----------------------------------------------------------------
bool update_filters_definition(const bool network_update) {

  // Free old definitions if necessary.
  if (gmic_custom_commands) delete[] gmic_custom_commands;
  gmic_custom_commands = 0;
  if (tree_view_store) g_object_unref(tree_view_store);
  gmic_entries.assign();
  gmic_1stlevel_entries.assign();
  gmic_commands.assign();
  gmic_preview_commands.assign();
  gmic_preview_factors.assign();
  gmic_arguments.assign();

  // Get filter definitions from the distant server.
  const char *const path_home = getenv(cimg_OS!=2?"HOME":"APPDATA");
  bool network_succeed = false;
  if (network_update) {
    char update_command[1024] = { 0 }, src_filename[1024] = { 0 }, dest_filename[1024] = { 0 };
    const char *const path_tmp = cimg::temporary_path();
    std::sprintf(src_filename,"%s/%s",path_tmp,gmic_update_file);
    std::sprintf(dest_filename,"%s/.%s",path_home,gmic_update_file);
    std::remove(src_filename);

    if (get_verbosity_mode()>0) { // Try with 'curl' first.
      std::sprintf(update_command,_gmic_path "curl --compressed %s%s -o %s",gmic_update_server,gmic_update_file,src_filename);
      std::fprintf(stderr,"\n*** Plug-in 'gmic_gimp' : Run update procedure : '%s'\n",update_command);
    } else
      std::sprintf(update_command,_gmic_path "curl --silent --compressed %s%s -o %s",gmic_update_server,gmic_update_file,src_filename);
    int status = cimg::system(update_command);
    std::FILE *file_s = std::fopen(src_filename,"r");

    if (!file_s) { // Try with 'wget' if 'curl' failed.
      if (get_verbosity_mode()>0) {
        std::sprintf(update_command,_gmic_path "wget %s%s -O %s",gmic_update_server,gmic_update_file,src_filename);
        std::fprintf(stderr,"\n*** Plug-in 'gmic_gimp' : Run update procedure : '%s'\n",update_command);
      } else
        std::sprintf(update_command,_gmic_path "wget --quiet %s%s -O %s",gmic_update_server,gmic_update_file,src_filename);
      status = cimg::system(update_command);
      file_s = std::fopen(src_filename,"r");
    }

    if (file_s) { // Check for '#@gmic' header.
      char sep = 0;
      if (std::fscanf(file_s,"#@gmi%c",&sep)!=1 || sep!='c') { // Perhaps compressed, so try 'gunzip' on it.
        std::fclose(file_s);
        std::sprintf(update_command,"%s.gz",src_filename);
        std::rename(src_filename,update_command);
        if (get_verbosity_mode()>0) {
          std::sprintf(update_command,_gmic_path "gunzip %s.gz",src_filename);
          std::fprintf(stderr,"\n*** Plug-in 'gmic_gimp' : Run update procedure : '%s'\n",update_command);
        } else
          std::sprintf(update_command,_gmic_path "gunzip --quiet %s.gz",src_filename);
        status = cimg::system(update_command);
        file_s = std::fopen(src_filename,"r");
        if (!file_s) {
          std::sprintf(update_command,"%s.gz",src_filename);
          std::remove(update_command);
        }
      } else std::rewind(file_s);
    }

    if (file_s) {
      unsigned int size_s = 0;
      std::fseek(file_s,0,SEEK_END);
      size_s = (unsigned int)std::ftell(file_s);
      std::rewind(file_s);
      if (size_s) {
        std::FILE *file_d = std::fopen(dest_filename,"w");
        char *buffer = new char[size_s], sep = 0;
        if (file_d &&
            std::fread(buffer,sizeof(char),size_s,file_s)==size_s &&
            std::sscanf(buffer,"#@gmi%c",&sep)==1 && sep=='c' &&
            std::fwrite(buffer,sizeof(char),size_s,file_d)==size_s) { network_succeed = true; std::fclose(file_d); }
        delete[] buffer;
      }
      std::fclose(file_s);
    }
  }

  // Get filter definitions from local files '.gmic' and '.gmic_def.xxxx'
  unsigned size_update = 0, size_custom = 0;
  char filename_update[1024] = { 0 }, filename_custom[1024] = { 0 };
  std::sprintf(filename_update,"%s/.gmic_def.%d",path_home,gmic_version);
  std::sprintf(filename_custom,"%s/.gmic",path_home);
  std::FILE
    *file_update = std::fopen(filename_update,"r"),
    *file_custom = std::fopen(filename_custom,"r");
  if (file_update) {
    std::fseek(file_update,0,SEEK_END);
    size_update = (unsigned int)std::ftell(file_update);
    std::rewind(file_update);
  }
  if (file_custom) {
    std::fseek(file_custom,0,SEEK_END);
    size_custom = (unsigned int)std::ftell(file_custom);
    std::rewind(file_custom);
  }
  const unsigned int size_final = size_update + size_custom + size_data_gmic_def + 1;
  char *ptrd = gmic_custom_commands = new char[size_final];
  if (size_custom) { ptrd+=std::fread(ptrd,1,size_custom,file_custom); std::fclose(file_custom); }
  if (size_update) { ptrd+=std::fread(ptrd,1,size_update,file_update); std::fclose(file_update); }
  if (size_data_gmic_def) { std::memcpy(ptrd,data_gmic_def,size_data_gmic_def-1); ptrd+=size_data_gmic_def-1; }
  *ptrd = 0;

  // Parse filters definitions and create corresponding TreeView store.
  GtkTreeIter iter, parent[16];
  tree_view_store = gtk_tree_store_new(2,G_TYPE_UINT,G_TYPE_STRING);
  char line[256*1024] = { 0 }, preview_command[4096] = { 0 }, arguments[4096] = { 0 }, entry[4096] = { 0 }, command[4096] = { 0 }, locale[8] = { 0 }, *_line = 0;
  std::strcpy(locale,get_locale());
  int level = 0, err = 0;
  std::sprintf(line,"#@gimp_%s ",locale);

  // Use English for default language if no translated filters found.
  if (!std::strstr(gmic_custom_commands,line)) { locale[0] = 'e'; locale[1] = 'n'; locale[2] = 0; }

  for (const char *data = gmic_custom_commands; *data; ) {
    _line = line;
    while (*data!='\n' && *data && _line<line+sizeof(line)) *(_line++) = *(data++); *_line = 0;       // Read new line.
    while (*data=='\n') ++data;                                                                       // Skip next '\n'.
    _line = line; while ((_line=std::strchr(_line,'\t'))!=0) *_line=' ';                              // Replace all tabs by spaces.
    if (line[0]!='#' || line[1]!='@' || line[2]!='g' ||                                               // Check for a '#@gimp' line.
        line[3]!='i' || line[4]!='m' || line[5]!='p') continue;
    if (line[6]=='_') {                                                                               // Check for a localized filter.
      if (line[7]==locale[0] && line[8]==locale[1] && line[9]==' ') _line = line + 10; else continue; // Weither the entry match current locale or not.
    } else if (line[6]==' ') _line = line + 7; else continue;                                         // Check for a non-localized filter.

    if (*_line!=':') {                                                                                // Check for a description of a possible filter or menu folder.
      *entry = *command = *preview_command = *arguments = 0;
      err = std::sscanf(_line," %4095[^:]: %4095[^,]%*c %4095[^,]%*c %4095[^\n]",
                        entry,command,preview_command,arguments);
      if (err==1) { // If entry defines a menu folder.
        cimg::strpare(entry,' ',false,true); cimg::strpare(entry,' ',false,true);
        char *nentry = entry; while (*nentry=='_') { ++nentry; --level; }
        if (level<0) level = 0; else if (level>15) level = 15;
        cimg::strpare(nentry,' ',false,true); cimg::strpare(nentry,'\"',true);
        if (*nentry) {
          gtk_tree_store_append(tree_view_store,&parent[level],level?&parent[level-1]:0);
          gtk_tree_store_set(tree_view_store,&parent[level],0,0,1,nentry,-1);
          if (!level) {
            const char *treepath = gtk_tree_model_get_string_from_iter(GTK_TREE_MODEL(tree_view_store),&parent[level]);
            CImg<char>(treepath,std::strlen(treepath)+1).move_to(gmic_1stlevel_entries);
          }
          ++level;
        }
      } else if (err>=2) { // If entry defines a regular filter.
        cimg::strpare(entry,' ',false,true); cimg::strpare(entry,'\"',true);
        cimg::strpare(command,' ',false,true);
        cimg::strpare(arguments,' ',false,true);
        CImg<char>(entry,std::strlen(entry)+1).move_to(gmic_entries);
        CImg<char>(command,std::strlen(command)+1).move_to(gmic_commands);
        CImg<char>(arguments,std::strlen(arguments)+1).move_to(gmic_arguments);
        if (err>=3) { // Filter has a specified preview command.
          cimg::strpare(preview_command,' ',false,true);
          char *const preview_mode = std::strchr(preview_command,'(');
          double factor = 1; char sep = 0;
          if (preview_mode && std::sscanf(preview_mode+1,"%lf%c",&factor,&sep)==2 && factor>=0 && sep==')') *preview_mode = 0;
          else factor = -1;
          CImg<char>(preview_command,std::strlen(preview_command)+1).move_to(gmic_preview_commands);
          CImg<double>::vector(factor).move_to(gmic_preview_factors);
        } else gmic_preview_commands.insert(1);
        gtk_tree_store_append(tree_view_store,&iter,level?&parent[level-1]:0);
        gtk_tree_store_set(tree_view_store,&iter,0,gmic_entries.size(),1,entry,-1);
      }
    } else { // Line is the continuation of an entry.
      if (gmic_arguments) {
        if (gmic_arguments.back()) gmic_arguments.back().back() = ' ';
        cimg::strpare(++_line,' ',false,true);
        gmic_arguments.back().append(CImg<char>(_line,std::strlen(_line)+1,1,1,1,true),'x');
      }
    }
  }
  return network_update?network_succeed:true;
}

// 'Convert' a CImg<float> image to a RGB[A] CImg<unsigned char> image, withing the same buffer.
//----------------------------------------------------------------------------------------------
void convert_image_float2uchar(CImg<float>& img) {
  const unsigned int siz = img.width()*img.height();
  unsigned char *ptrd = (unsigned char*)img.data();
  switch (img.spectrum()) {
  case 1 : {
    const float *ptr0 = img.data(0,0,0,0);
    for (unsigned int i = 0; i<siz; ++i) *(ptrd++) = (unsigned char)*(ptr0++);
  } break;
  case 2 : {
    const float *ptr0 = img.data(0,0,0,0), *ptr1 = img.data(0,0,0,1);
    for (unsigned int i = 0; i<siz; ++i) {
      *(ptrd++) = (unsigned char)*(ptr0++);
      *(ptrd++) = (unsigned char)*(ptr1++);
    }
  } break;
  case 3 : {
    const float *ptr0 = img.data(0,0,0,0), *ptr1 = img.data(0,0,0,1), *ptr2 = img.data(0,0,0,2);
    for (unsigned int i = 0; i<siz; ++i) {
      *(ptrd++) = (unsigned char)*(ptr0++);
      *(ptrd++) = (unsigned char)*(ptr1++);
      *(ptrd++) = (unsigned char)*(ptr2++);
    }
  } break;
  case 4 : {
    const float *ptr0 = img.data(0,0,0,0), *ptr1 = img.data(0,0,0,1), *ptr2 = img.data(0,0,0,2), *ptr3 = img.data(0,0,0,3);
    for (unsigned int i = 0; i<siz; ++i) {
      *(ptrd++) = (unsigned char)*(ptr0++);
      *(ptrd++) = (unsigned char)*(ptr1++);
      *(ptrd++) = (unsigned char)*(ptr2++);
      *(ptrd++) = (unsigned char)*(ptr3++);
    }
  } break;
  default: return;
  }
}

// Calibrate any image to fit the number of required channels (GRAY,GRAYA, RGB or RGBA).
//---------------------------------------------------------------------------------------
void calibrate_image(CImg<float>& img, const unsigned int channels, const bool preview) {
  if (!img || !channels) return;
  switch (channels) {

  case 1 : // To GRAY
    switch (img.spectrum()) {
    case 1 : // from GRAY
      break;
    case 2 : // from GRAYA
      if (preview) {
        float *ptr_r = img.data(0,0,0,0), *ptr_a = img.data(0,0,0,1);
        cimg_forXY(img,x,y) {
          const unsigned int a = (unsigned int)*(ptr_a++), i = 96 + (((x^y)&8)<<3);
          *ptr_r = (float)((a*(unsigned int)*ptr_r + (255-a)*i)>>8);
          ++ptr_r;
        }
      }
      img.channel(0);
      break;
    case 3 : // from RGB
      img.RGBtoYCbCr().channel(0);
      break;
    case 4 : // from RGBA
      img.get_shared_channels(0,2).RGBtoYCbCr();
      if (preview) {
        float *ptr_r = img.data(0,0,0,0), *ptr_a = img.data(0,0,0,3);
        cimg_forXY(img,x,y) {
          const unsigned int a = (unsigned int)*(ptr_a++), i = 96 + (((x^y)&8)<<3);
          *ptr_r = (float)((a*(unsigned int)*ptr_r + (255-a)*i)>>8);
          ++ptr_r;
        }
      }
      img.channel(0);
      break;
    default : // from multi-channel (>4)
      img.channel(0);
    } break;

  case 2: // To GRAYA
    switch (img.spectrum()) {
    case 1: // from GRAY
      img.resize(-100,-100,1,2,0).get_shared_channel(1).fill(255);
      break;
    case 2: // from GRAYA
      break;
    case 3: // from RGB
      img.RGBtoYCbCr().channels(0,1).get_shared_channel(1).fill(255);
      break;
    case 4: // from RGBA
      img.get_shared_channels(0,2).RGBtoYCbCr();
      img.get_shared_channel(1) = img.get_shared_channel(3);
      img.channels(0,1);
      break;
    default: // from multi-channel (>4)
      img.channels(0,1).get_shared_channel(1).fill(255);
    } break;

  case 3: // to RGB
    switch (img.spectrum()) {
    case 1: // from GRAY
      img.resize(-100,-100,1,3);
      break;
    case 2: // from GRAYA
      if (preview) {
        float *ptr_r = img.data(0,0,0,0), *ptr_a = img.data(0,0,0,1);
        cimg_forXY(img,x,y) {
          const unsigned int a = (unsigned int)*(ptr_a++), i = 96 + (((x^y)&8)<<3);
          *ptr_r = (float)((a*(unsigned int)*ptr_r + (255-a)*i)>>8);
          ++ptr_r;
        }
      }
      img.channel(0).resize(-100,-100,1,3);
      break;
    case 3: // from RGB
      break;
    case 4: // from RGBA
      if (preview) {
        float *ptr_r = img.data(0,0,0,0), *ptr_g = img.data(0,0,0,1), *ptr_b = img.data(0,0,0,2), *ptr_a = img.data(0,0,0,3);
        cimg_forXY(img,x,y) {
          const unsigned int a = (unsigned int)*(ptr_a++), i = 96 + (((x^y)&8)<<3);
          *ptr_r = (float)((a*(unsigned int)*ptr_r + (255-a)*i)>>8);
          *ptr_g = (float)((a*(unsigned int)*ptr_g + (255-a)*i)>>8);
          *ptr_b = (float)((a*(unsigned int)*ptr_b + (255-a)*i)>>8);
          ++ptr_r; ++ptr_g; ++ptr_b;
        }
      }
      img.channels(0,2);
      break;
    default: // from multi-channel (>4)
      img.channels(0,2);
    } break;

  case 4: // to RGBA
    switch (img.spectrum()) {
    case 1: // from GRAY
      img.resize(-100,-100,1,4).get_shared_channel(3).fill(255);
      break;
    case 2: // from GRAYA
      img.resize(-100,-100,1,4,0);
      img.get_shared_channel(3) = img.get_shared_channel(1);
      img.get_shared_channel(1) = img.get_shared_channel(0);
      img.get_shared_channel(2) = img.get_shared_channel(0);
      break;
    case 3: // from RGB
      img.resize(-100,-100,1,4,0).get_shared_channel(3).fill(255);
      break;
    case 4: // from RGBA
      break;
    default: // from multi-channel (>4)
      img.resize(-100,-100,1,4,0);
    } break;
  }
}

// Get the input layers of a GIMP image as a list of CImg<float>.
//---------------------------------------------------------------
template<typename T>
CImg<int> get_input_layers(CImgList<T>& images) {

  // Retrieve the list of desired layers.
  int
    nb_layers = 0,
    *layers = gimp_image_get_layers(image_id,&nb_layers),
    active_layer = gimp_image_get_active_layer(image_id);
  CImg<int> input_layers;
  const unsigned int input_mode = get_input_mode();
  switch (input_mode) {
  case 0 : // Input none
    break;
  case 1 : // Input active layer
    input_layers = CImg<int>::vector(active_layer);
    break;
  case 2 : case 9 : // Input all image layers
    input_layers = CImg<int>(layers,1,nb_layers);
    if (input_mode==9) input_layers.mirror('y');
    break;
  case 3 : { // Input active & below layers
    int i = 0; for (i = 0; i<nb_layers; ++i) if (layers[i]==active_layer) break;
    if (i<nb_layers-1) input_layers = CImg<int>::vector(active_layer,layers[i+1]);
    else input_layers = CImg<int>::vector(active_layer);
  } break;
  case 4 : { // Input active & above layers
    int i = 0; for (i = 0; i<nb_layers; ++i) if (layers[i]==active_layer) break;
    if (i>0) input_layers = CImg<int>::vector(active_layer,layers[i-1]);
    else input_layers = CImg<int>::vector(active_layer);
  } break;
  case 5 : case 7 : { // Input all visible image layers
    CImgList<int> visible_layers;
    for (int i = 0; i<nb_layers; ++i)
      if (gimp_drawable_get_visible(layers[i])) CImg<int>::vector(layers[i]).move_to(visible_layers);
    input_layers = visible_layers>'y';
    if (input_mode==7) input_layers.mirror('y');
  } break;
  default : { // Input all invisible image layers
    CImgList<int> invisible_layers;
    for (int i = 0; i<nb_layers; ++i)
      if (!gimp_drawable_get_visible(layers[i])) CImg<int>::vector(layers[i]).move_to(invisible_layers);
    input_layers = invisible_layers>'y';
    if (input_mode==8) input_layers.mirror('y');
  } break;
  }

  // Read input image data into a CImgList<float>.
  images.assign(input_layers.height());
  GimpPixelRgn region;
  gint x1, y1, x2, y2;
  cimglist_for(images,l) {
    GimpDrawable *drawable = gimp_drawable_get(input_layers[l]);
    gimp_drawable_mask_bounds(drawable->drawable_id,&x1,&y1,&x2,&y2);
    const int channels = drawable->bpp;
    gimp_pixel_rgn_init(&region,drawable,x1,y1,x2-x1,y2-y1,false,false);
    guchar *const row = g_new(guchar,(x2-x1)*channels), *ptrs = 0;
    CImg<T> img(x2-x1,y2-y1,1,channels);
    switch (channels) {
    case 1 : {
      T *ptr_r = img.data(0,0,0,0);
      cimg_forY(img,y) {
        gimp_pixel_rgn_get_row(&region,ptrs=row,x1,y1+y,img.width());
        cimg_forX(img,x) *(ptr_r++) = (T)*(ptrs++);
      }
    } break;
    case 2 : {
      T *ptr_r = img.data(0,0,0,0), *ptr_g = img.data(0,0,0,1);
      cimg_forY(img,y) {
        gimp_pixel_rgn_get_row(&region,ptrs=row,x1,y1+y,img.width());
        cimg_forX(img,x) { *(ptr_r++) = (T)*(ptrs++); *(ptr_g++) = (T)*(ptrs++); }
      }
    } break;
    case 3 : {
      T *ptr_r = img.data(0,0,0,0), *ptr_g = img.data(0,0,0,1), *ptr_b = img.data(0,0,0,2);
      cimg_forY(img,y) {
        gimp_pixel_rgn_get_row(&region,ptrs=row,x1,y1+y,img.width());
        cimg_forX(img,x) { *(ptr_r++) = (T)*(ptrs++); *(ptr_g++) = (T)*(ptrs++); *(ptr_b++) = (T)*(ptrs++); }
      }
    } break;
    case 4 : {
      T *ptr_r = img.data(0,0,0,0), *ptr_g = img.data(0,0,0,1), *ptr_b = img.data(0,0,0,2), *ptr_a = img.data(0,0,0,3);
      cimg_forY(img,y) {
        gimp_pixel_rgn_get_row(&region,ptrs=row,x1,y1+y,img.width());
        cimg_forX(img,x) {
          *(ptr_r++) = (T)*(ptrs++); *(ptr_g++) = (T)*(ptrs++); *(ptr_b++) = (T)*(ptrs++); *(ptr_a++) = (T)*(ptrs++);
        }
      }
    } break;
    }
    g_free(row);
    gimp_drawable_detach(drawable);
    img.move_to(images[l]);
  }
  return input_layers;
}

// Return the G'MIC command line needed to run the selected filter.
//-----------------------------------------------------------------
const char* get_command_line(const bool preview) {
  const unsigned int
    filter = get_current_filter(),
    nbparams = get_filter_nbparams(filter);
  if (!filter) return 0;
  static CImg<char> res;
  CImgList<char> lres;
  switch (get_verbosity_mode()) {
  case 0: CImg<char>("-v - -",6).move_to(lres); break;
  case 1: CImg<char>("-",1).move_to(lres); break;
  case 2: CImg<char>("-v 99 -",7).move_to(lres); break;
  default: CImg<char>("-debug -",8).move_to(lres);
  }
  const unsigned int N = filter - 1;
  const CImg<char> &command_item = (preview?gmic_preview_commands[N]:gmic_commands[N]);
  if (command_item) {
    lres.insert(command_item);
    if (nbparams) {
      lres[1].back() = ' ';
      for (unsigned int p = 0; p<nbparams; ++p) {
        const char *ss = get_filter_parameter(filter,p);
        char nparam[256] = { 0 }, *sd = nparam;
        const unsigned int l = std::strlen(ss);
        if (l>=2 && ss[0]=='\"' && ss[l-1]=='\"') { // Replace special characters in a string or a filename.
          ++ss; *(sd++) = '\"';
          for (unsigned int i = 1; i<l-1; ++i, ++ss) { const char c = *ss; *(sd++) = c=='{'?_lbrace:c=='}'?_rbrace:c==','?_comma:c=='\"'?_dquote:c; }
          *(sd++) = '\"'; *(sd++) = 0;
          CImg<char>(nparam,l+1).move_to(lres);
        } else CImg<char>(ss,l+1).move_to(lres);
        lres.back().back() = ',';
      }
    }
    (res=lres>'x').back() = 0;
  }
  return res.data();
}

// Set defaut zoom factor for preview of the current filter.
//----------------------------------------------------------
void set_preview_factor() {
  const unsigned int filter = get_current_filter();
  if (filter && GIMP_IS_PREVIEW(gui_preview)) {
    double factor = gmic_preview_factors(filter-1,0);
    if (factor>=0) {
      if (!factor) { // Compute factor so that 1:1 preview of the image is displayed.
        int _pw = 0, _ph = 0; gimp_preview_get_size(GIMP_PREVIEW(gui_preview),&_pw,&_ph);
        const float pw = (float)_pw, ph = (float)_ph, dw = (float)drawable_preview->width, dh = (float)drawable_preview->height;
        factor = std::sqrt((dw*dw+dh*dh)/(pw*pw+ph*ph));
      }
      gimp_zoom_model_zoom(gimp_zoom_preview_get_model(GIMP_ZOOM_PREVIEW(gui_preview)),GIMP_ZOOM_TO,factor);
    }
  }
}

// Handle GUI event functions.
//----------------------------
void create_parameters_gui(const bool);
void process_image(const char *);
void process_preview();

// Secure function for invalidate preview.
void _gimp_preview_invalidate() {
  if ((GIMP_IS_PREVIEW(gui_preview) && gimp_drawable_is_valid(drawable_preview->drawable_id)))
    gimp_preview_invalidate(GIMP_PREVIEW(gui_preview));
  else {
    if (GTK_IS_WIDGET(gui_preview)) gtk_widget_destroy(gui_preview);
    drawable_preview = gimp_drawable_get(gimp_image_get_active_drawable(image_id));
    gui_preview = gimp_zoom_preview_new(drawable_preview);
    gtk_widget_show(gui_preview);
    gtk_box_pack_end(GTK_BOX(left_pane),gui_preview,true,true,0);
    g_signal_connect(gui_preview,"invalidated",G_CALLBACK(process_preview),0);
  }
}

// Handle preview resize event.
void on_dialog_resized() {
  static int opw = 0, oph = 0;
  int pw = 0, ph = 0;
  if (GIMP_IS_PREVIEW(gui_preview)) {
    gimp_preview_get_size(GIMP_PREVIEW(gui_preview),&pw,&ph);
    if (!opw || !oph) { opw = pw; oph = ph; } else {
      if (pw!=opw || ph!=oph) { set_preview_factor(); opw = pw; oph = ph; }
    }
  }
}

// Handle widgets events related to parameter changes.
void on_float_parameter_changed(GtkAdjustment *const adjustment, const void *const event_infos) {
  double value = 0;
  gimp_double_adjustment_update(adjustment,&value);
  char s_value[256] = { 0 };
  std::sprintf(s_value,"%g",value);
  set_filter_parameter(get_current_filter(),*(int*)event_infos,s_value);
  _create_dialog_gui = true;
}

void on_int_parameter_changed(GtkAdjustment *const adjustment, const void *const event_infos) {
  int value = 0;
  gimp_int_adjustment_update(adjustment,&value);
  char s_value[256] = { 0 };
  std::sprintf(s_value,"%d",value);
  set_filter_parameter(get_current_filter(),*(int*)event_infos,s_value);
  _create_dialog_gui = true;
}

void on_bool_parameter_changed(GtkCheckButton *const checkbutton, const void *const event_infos) {
  int value = 0;
  g_object_get(checkbutton,"active",&value,NULL);
  char s_value[256] = { 0 };
  std::sprintf(s_value,"%d",value?1:0);
  set_filter_parameter(get_current_filter(),*(int*)event_infos,s_value);
  _create_dialog_gui = true;
}

void on_list_parameter_changed(GtkComboBox *const combobox, const void *const event_infos) {
  int value = 0;
  g_object_get(combobox,"active",&value,NULL);
  char s_value[256] = { 0 };
  std::sprintf(s_value,"%d",value);
  set_filter_parameter(get_current_filter(),*(int*)event_infos,s_value);
  _create_dialog_gui = true;
}

void on_text_parameter_changed(const void *const event_infos) {
  GtkWidget *entry = *((GtkWidget**)event_infos+1);
  const char *const s_value = gtk_entry_get_text(GTK_ENTRY(entry));
  char s_param[256] = { 0 };
  if (s_value && *s_value) std::sprintf(s_param,"\"%s\"",s_value); else std::strcpy(s_param,"\"\"");
  set_filter_parameter(get_current_filter(),*(int*)event_infos,s_param);
  _create_dialog_gui = true;
}

void on_file_parameter_changed(GtkFileChooser *const file_chooser, const void *const event_infos) {
  const char *const s_value = gtk_file_chooser_get_filename(file_chooser);
  char s_param[256] = { 0 };
  if (s_value && *s_value) std::sprintf(s_param,"\"%s\"",s_value); else std::strcpy(s_param,"\"\"");
  set_filter_parameter(get_current_filter(),*(int*)event_infos,s_param);
  _create_dialog_gui = true;
}

void on_color_parameter_changed(GtkColorButton *const color_button, const void *const event_infos) {
  GdkColor color;
  gtk_color_button_get_color(color_button,&color);
  char s_value[256] = { 0 };
  if (gtk_color_button_get_use_alpha(color_button))
    std::sprintf(s_value,"%d,%d,%d,%d",color.red>>8,color.green>>8,color.blue>>8,gtk_color_button_get_alpha(color_button)>>8);
  else std::sprintf(s_value,"%d,%d,%d",color.red>>8,color.green>>8,color.blue>>8);
  set_filter_parameter(get_current_filter(),*(int*)event_infos,s_value);
  _create_dialog_gui = true;
}

// Handle responses to the dialog window buttons.
void on_dialog_input_mode_changed(GtkComboBox *const combobox) {
  int value = 0;
  g_object_get(combobox,"active",&value,NULL);
  if (value<2) gtk_combo_box_set_active(combobox,value=3);
  set_input_mode((unsigned int)value);
  _gimp_preview_invalidate();
}

void on_dialog_output_mode_changed(GtkComboBox *const combobox) {
  int value = 0;
  g_object_get(combobox,"active",&value,NULL);
  if (value<2) gtk_combo_box_set_active(combobox,value=2);
  set_output_mode((unsigned int)value);
}

void on_dialog_verbosity_mode_changed(GtkComboBox *const combobox) {
  int value = 0;
  g_object_get(combobox,"active",&value,NULL);
  if (value<2) gtk_combo_box_set_active(combobox,value=2);
  set_verbosity_mode((unsigned int)value);
  _gimp_preview_invalidate();
}

void on_dialog_preview_mode_changed(GtkComboBox *const combobox) {
  int value = 0;
  g_object_get(combobox,"active",&value,NULL);
  if (value<2) gtk_combo_box_set_active(combobox,value=2);
  set_preview_mode((unsigned int)value);
  _gimp_preview_invalidate();
}

void on_dialog_maximize_button_clicked(GtkButton *const button) {
  static int ow = 0, oh = 0;
  GdkScreen* screen = gtk_window_get_screen(GTK_WINDOW(dialog_window));
  const int
    width = gdk_screen_get_width(screen),
    height = gdk_screen_get_height(screen);
  if (width>0 && height>0 && !ow && !oh) {
    gtk_window_get_size(GTK_WINDOW(dialog_window),&ow,&oh);
    gtk_window_resize(GTK_WINDOW(dialog_window),width,height);
    gtk_window_move(GTK_WINDOW(dialog_window),0,0);
    gtk_button_set_label(button,t("Restore prev_iew"));
  } else if (ow>0 && oh>0) {
    gtk_window_resize(GTK_WINDOW(dialog_window),ow,oh);
    ow = oh = 0;
    gtk_button_set_label(button,t("Maximize prev_iew"));
  }
  _gimp_preview_invalidate();
}

void on_dialog_preview_button_clicked() {
  GimpPreview *const preview = GIMP_PREVIEW(gui_preview);
  const bool state = gimp_preview_get_update(preview);
  gimp_preview_set_update(preview,false);
  if (!state) {
    preview->update_preview = true;
    _gimp_preview_invalidate();
    preview->update_preview = false;
  }
}

void on_dialog_reset_clicked() {
  create_parameters_gui(true);
  _create_dialog_gui = true;
  _gimp_preview_invalidate();
}

void on_dialog_cancel_clicked() {
  _create_dialog_gui = false;
  gtk_main_quit();
}

void on_dialog_apply_clicked() {
  process_image(0);
  _create_dialog_gui = false;
  _gimp_preview_invalidate();
}

void on_dialog_net_update_toggled(GtkToggleButton *const toggle_button) {
  set_net_update(gtk_toggle_button_get_active(toggle_button));
}

void on_dialog_tree_mode_clicked(GtkWidget *const tree_view) {
  set_tree_mode(!get_tree_mode());
  flush_tree_view(tree_view);
}

void on_dialog_update_clicked(GtkWidget *const tree_view) {
  if (!update_filters_definition(get_net_update())) {
    GtkWidget *const message = gtk_message_dialog_new_with_markup(0,GTK_DIALOG_MODAL,GTK_MESSAGE_ERROR,GTK_BUTTONS_OK,
                                                                  t(0),gmic_update_server,gmic_update_file,gmic_update_file);
    gtk_widget_show(message);
    gtk_dialog_run(GTK_DIALOG(message));
    gtk_widget_destroy(message);
  } else reset_filters_parameters();
  const unsigned int filter = get_current_filter();
  gtk_tree_view_set_model(GTK_TREE_VIEW(tree_view),GTK_TREE_MODEL(tree_view_store));
  set_current_filter(filter);
  flush_tree_view(tree_view);
  create_parameters_gui(true);
}

void on_filter_changed(GtkTreeSelection *const selection) {
  GtkTreeIter iter;
  GtkTreeModel *model;
  unsigned int choice = 0;
  if (gtk_tree_selection_get_selected(selection,&model,&iter)) {
    gtk_tree_model_get(model,&iter,0,&choice,-1);
    const char *const treepath = gtk_tree_model_get_string_from_iter(GTK_TREE_MODEL(tree_view_store),&iter);
    gimp_set_data("gmic_current_treepath",treepath,std::strlen(treepath)+1);
  }
  set_current_filter(choice);
  create_parameters_gui(false);
  _create_dialog_gui = true;
  _gimp_preview_invalidate();
}

// Process image data with the G'MIC interpreter.
//-----------------------------------------------

// This structure stores the arguments required by the processing thread.
struct st_process_thread {
  CImgList<float> images;
  CImg<char> error_message;
  bool is_thread;
  const char *command_line;
  unsigned int verbosity_mode;
  float progress;
#if !defined(__MACOSX__)  && !defined(__APPLE__)
  pthread_mutex_t is_running;
  pthread_t thread;
#endif
};

// Thread that runs the G'MIC interpreter.
void *process_thread(void *arg) {
  st_process_thread &spt = *(st_process_thread*)arg;
  try {
    if (spt.verbosity_mode>0)
      std::fprintf(stderr,"\n*** Plug-in 'gmic_gimp' : Run G'MIC, with command : '%s'\n",spt.command_line);
    std::setlocale(LC_NUMERIC,"C");
    gmic(spt.command_line,spt.images,gmic_custom_commands,true,&spt.progress);
    if (spt.verbosity_mode>0)
      std::fprintf(stderr,"\n*** Plug-in 'gmic_gimp' : G'MIC successfully returned !\n");
  } catch (gmic_exception &e) {
    spt.images.assign();
    spt.error_message.assign(e.what(),std::strlen(e.what())+1);
    if (spt.verbosity_mode>0)
      std::fprintf(stderr,"\n*** Plug-in 'gmic_gimp' :\n %s\n",e.what());
  }
#if !defined(__MACOSX__)  && !defined(__APPLE__)
  if (spt.is_thread) {
    pthread_mutex_unlock(&spt.is_running);
    pthread_exit(0);
  }
#endif
  return 0;
}

// Process the selected image/layers.
//------------------------------------
void process_image(const char *command_line) {
  if (!gimp_image_is_valid(image_id)) return;
  const unsigned int filter = get_current_filter();
  if (!command_line && !filter) return;
  const char *_command_line = command_line?command_line:get_command_line(false);
  if (!_command_line || std::strstr(_command_line,"-_none_")) return;
  if (run_mode!=GIMP_RUN_NONINTERACTIVE) {
    GtkWidget *markup2ascii = gtk_label_new(0);
    gtk_label_set_markup(GTK_LABEL(markup2ascii),gmic_entries[filter-1].data());
    gimp_progress_init_printf(" G'MIC : %s...",gtk_label_get_text(GTK_LABEL(markup2ascii)));
    gtk_widget_destroy(markup2ascii);
  }

  // Get input layers for the chosen filter.
  st_process_thread spt;
  spt.command_line = _command_line;
  spt.verbosity_mode = get_verbosity_mode();
  spt.progress = -1;
  const CImg<int> layers = get_input_layers(spt.images);
  CImg<int> dimensions(spt.images.size(),4);
  cimglist_for(spt.images,l) {
    const CImg<float>& img = spt.images[l];
    dimensions(l,0) = img.width(); dimensions(l,1) = img.height();
    dimensions(l,2) = img.depth(); dimensions(l,3) = img.spectrum();
  }

  // Create processing thread and wait for its completion.
  if (run_mode!=GIMP_RUN_NONINTERACTIVE) {
#if !defined(__MACOSX__)  && !defined(__APPLE__)
    spt.is_thread = true;
    pthread_mutex_init(&spt.is_running,0);
    pthread_mutex_lock(&spt.is_running);
    pthread_create(&(spt.thread),0,process_thread,(void*)&spt);
    while (pthread_mutex_trylock(&spt.is_running)) {
      if (spt.progress>=0) gimp_progress_update(cimg::min(1.0,spt.progress/100.0)); else gimp_progress_pulse();
      cimg::wait(350);
    }
    gimp_progress_update(1.0);
    pthread_join(spt.thread,0);
    pthread_mutex_unlock(&spt.is_running);
    pthread_mutex_destroy(&spt.is_running);
#else
  gimp_progress_update(0.5);
  process_thread(&spt);
  gimp_progress_update(1.0);
#endif
  } else {
    spt.is_thread = false;
    process_thread(&spt);
  }

  // Check that everything went fine, else display an error dialog.
  if (spt.error_message) {
    GtkWidget *const message = gtk_message_dialog_new_with_markup(0,GTK_DIALOG_MODAL,GTK_MESSAGE_ERROR,GTK_BUTTONS_OK,
                                                                  "<b>Filter processing error !</b>\n\n"
                                                                  "Message returned by the G'MIC engine :\n\n"
                                                                  "<i>%s</i>",spt.error_message.data());
    gtk_widget_show(message);
    gtk_dialog_run(GTK_DIALOG(message));
    gtk_widget_destroy(message);
  } else {

    // Get output layers dimensions and check if input/output layers have compatible dimensions.
    unsigned int max_width = 0, max_height = 0, max_channels = 0;
    cimglist_for(spt.images,l) {
      if (spt.images[l]._width>max_width) max_width = spt.images[l]._width;
      if (spt.images[l]._height>max_height) max_height = spt.images[l]._height;
      if (spt.images[l]._spectrum>max_channels) max_channels = spt.images[l]._spectrum;
    }
    bool is_compatible_dimensions = (spt.images.size()==layers._height);
    for (unsigned int p = 0; p<spt.images.size() && is_compatible_dimensions; ++p) {
      const CImg<float>& img = spt.images[p];
      if (img.width()!=dimensions(p,0) ||
          img.height()!=dimensions(p,1) ||
          img.spectrum()>dimensions(p,3)) is_compatible_dimensions = false;
    }

    // Transfer the output layers back into GIMP.
    GimpPixelRgn region;
    gint x1, y1, x2, y2;
    const unsigned int output_mode = get_output_mode();
    switch (output_mode) {
    case 0 : { // Output in 'Replace' mode.
      gimp_image_undo_group_start(image_id);
      if (is_compatible_dimensions) cimglist_for(spt.images,l) { // Direct replacement of the layer data.
          CImg<float> &img = spt.images[l];
          calibrate_image(img,dimensions(l,3),false);
          GimpDrawable *drawable = gimp_drawable_get(layers[l]);
          gimp_drawable_mask_bounds(drawable->drawable_id,&x1,&y1,&x2,&y2);
          gimp_pixel_rgn_init(&region,drawable,x1,y1,x2-x1,y2-y1,true,true);
          convert_image_float2uchar(img);
          gimp_pixel_rgn_set_rect(&region,(guchar*)img.data(),x1,y1,x2-x1,y2-y1);
          img.assign();
          gimp_drawable_flush(drawable);
          gimp_drawable_merge_shadow(drawable->drawable_id,true);
          gimp_drawable_update(drawable->drawable_id,x1,y1,x2-x1,y2-y1);
          gimp_drawable_detach(drawable);
        } else { // Indirect replacement : create new layers.
        gimp_selection_none(image_id);
        const int layer_pos = gimp_image_get_layer_position(image_id,layers[0]);
        for (unsigned int i = 0; i<layers._height; ++i) gimp_image_remove_layer(image_id,layers[i]);
        cimglist_for(spt.images,p) {
          CImg<float> &img = spt.images[p];
          if (gimp_image_base_type(image_id)==GIMP_GRAY) calibrate_image(img,(img.spectrum()==1 || img.spectrum()==3)?1:2,false);
          else calibrate_image(img,(img.spectrum()==1 || img.spectrum()==3)?3:4,false);
          gint layer_id = gimp_layer_new(image_id,"image",img.width(),img.height(),
                                         img.spectrum()==1?GIMP_GRAY_IMAGE:
                                         img.spectrum()==2?GIMP_GRAYA_IMAGE:
                                         img.spectrum()==3?GIMP_RGB_IMAGE:GIMP_RGBA_IMAGE,
                                         100.0,GIMP_NORMAL_MODE);
          gimp_image_add_layer(image_id,layer_id,layer_pos+p);
          GimpDrawable *drawable = gimp_drawable_get(layer_id);
          gimp_pixel_rgn_init(&region,drawable,0,0,drawable->width,drawable->height,true,true);
          convert_image_float2uchar(img);
          gimp_pixel_rgn_set_rect(&region,(guchar*)img.data(),0,0,img.width(),img.height());
          img.assign();
          gimp_drawable_flush(drawable);
          gimp_drawable_merge_shadow(drawable->drawable_id,true);
          gimp_drawable_update(drawable->drawable_id,0,0,drawable->width,drawable->height);
          gimp_drawable_detach(drawable);
        }
        gimp_image_resize_to_layers(image_id);
      }
      gimp_image_undo_group_end(image_id);
    } break;
    case 1 : case 2 : { // Output in 'New layer(s)' mode.
      gimp_image_undo_group_start(image_id);
      gimp_selection_none(image_id);
      const gint active_layer_id = gimp_image_get_active_layer(image_id);
      gint layer_id = 0;
      cimglist_for(spt.images,p) {
        CImg<float> &img = spt.images[p];
        if (gimp_image_base_type(image_id)==GIMP_GRAY)
          calibrate_image(img,(img.spectrum()==1 || img.spectrum()==3)?1:2,false);
        else
          calibrate_image(img,(img.spectrum()==1 || img.spectrum()==3)?3:4,false);
        layer_id = gimp_layer_new(image_id,"image",img.width(),img.height(),
                                  img.spectrum()==1?GIMP_GRAY_IMAGE:
                                  img.spectrum()==2?GIMP_GRAYA_IMAGE:
                                  img.spectrum()==3?GIMP_RGB_IMAGE:GIMP_RGBA_IMAGE,
                                  100.0,GIMP_NORMAL_MODE);
        gimp_image_add_layer(image_id,layer_id,p);
        GimpDrawable *drawable = gimp_drawable_get(layer_id);
        gimp_pixel_rgn_init(&region,drawable,0,0,drawable->width,drawable->height,true,true);
        convert_image_float2uchar(img);
        gimp_pixel_rgn_set_rect(&region,(guchar*)img.data(),0,0,img.width(),img.height());
        img.assign();
        gimp_drawable_flush(drawable);
        gimp_drawable_merge_shadow(drawable->drawable_id,true);
        gimp_drawable_update(drawable->drawable_id,0,0,drawable->width,drawable->height);
        gimp_drawable_detach(drawable);
      }
      gimp_image_resize_to_layers(image_id);
      if (output_mode==1) gimp_image_set_active_layer(image_id,active_layer_id);
      else gtk_widget_destroy(gui_preview);  // Will force the preview to refresh on the new active layer.
      gimp_image_undo_group_end(image_id);
    } break;
    default : { // Output in 'New image' mode.
      if (spt.images.size()) {
        const int nimage_id = gimp_image_new(max_width,max_height,max_channels<=2?GIMP_GRAY:GIMP_RGB);
        gimp_image_undo_group_start(nimage_id);
        cimglist_for(spt.images,p) {
          CImg<float> &img = spt.images[p];
          if (gimp_image_base_type(nimage_id)!=GIMP_GRAY)
            calibrate_image(img,(img.spectrum()==1 || img.spectrum()==3)?3:4,false);
          gint layer_id = gimp_layer_new(nimage_id,"image",img.width(),img.height(),
                                         img.spectrum()==1?GIMP_GRAY_IMAGE:
                                         img.spectrum()==2?GIMP_GRAYA_IMAGE:
                                         img.spectrum()==3?GIMP_RGB_IMAGE:GIMP_RGBA_IMAGE,
                                         100.0,GIMP_NORMAL_MODE);
          gimp_image_add_layer(nimage_id,layer_id,p);
          GimpDrawable *drawable = gimp_drawable_get(layer_id);
          GimpPixelRgn dest_region;
          gimp_pixel_rgn_init(&dest_region,drawable,0,0,drawable->width,drawable->height,true,true);
          convert_image_float2uchar(img);
          gimp_pixel_rgn_set_rect(&dest_region,(guchar*)img.data(),0,0,img.width(),img.height());
          img.assign();
          gimp_drawable_flush(drawable);
          gimp_drawable_merge_shadow(drawable->drawable_id,true);
          gimp_drawable_update(drawable->drawable_id,0,0,drawable->width,drawable->height);
          gimp_drawable_detach(drawable);
        }
        gimp_display_new(nimage_id);
        gimp_image_undo_group_end(nimage_id);
      }
    }
    }
  }
  if (run_mode!=GIMP_RUN_NONINTERACTIVE) {
    gimp_progress_end();
    gimp_displays_flush();
  }
}

// Process the preview image.
//---------------------------
void process_preview() {
  if (!gimp_image_is_valid(image_id)) return;
  const unsigned int filter = get_current_filter();
  if (!filter) return;
  const char *const command_line = get_command_line(true);
  if (!command_line || std::strstr(command_line,"-_none_")) return;

  // Get input layers for the chosen filter and convert then to the preview size if necessary.
  st_process_thread spt;
  spt.is_thread = false;
  spt.command_line = command_line;
  spt.verbosity_mode = get_verbosity_mode();
  spt.progress = -1;

  const unsigned int input_mode = get_input_mode();
  int w, h, channels, nb_layers = 0, *layers = gimp_image_get_layers(image_id,&nb_layers);
  guchar *const ptr0 = gimp_zoom_preview_get_source(GIMP_ZOOM_PREVIEW(gui_preview),&w,&h,&channels), *ptrs = ptr0;
  if (nb_layers && input_mode) {
    if (input_mode==1 ||
        (input_mode==2 && nb_layers==1) ||
        (input_mode==3 && nb_layers==1 && gimp_drawable_get_visible(layers[0])) ||
        (input_mode==4 && nb_layers==1 && !gimp_drawable_get_visible(layers[0])) ||
        (input_mode==5 && nb_layers==1)) { // If only one input layer, use the default thumbnail provided by GIMP.
      spt.images.assign(1,w,h,1,channels);
      const int wh = w*h;
      switch (channels) {
      case 1 : {
        float *ptr_r = spt.images[0].data(0,0,0,0);
        for (int xy = 0; xy<wh; ++xy) *(ptr_r++) = (float)*(ptrs++);
      } break;
      case 2 : {
        float *ptr_r = spt.images[0].data(0,0,0,0), *ptr_g = spt.images[0].data(0,0,0,1);
        for (int xy = 0; xy<wh; ++xy) { *(ptr_r++) = (float)*(ptrs++); *(ptr_g++) = (float)*(ptrs++);
        }
      } break;
      case 3 : {
        float *ptr_r = spt.images[0].data(0,0,0,0), *ptr_g = spt.images[0].data(0,0,0,1), *ptr_b = spt.images[0].data(0,0,0,2);
        for (int xy = 0; xy<wh; ++xy) {
          *(ptr_r++) = (float)*(ptrs++); *(ptr_g++) = (float)*(ptrs++); *(ptr_b++) = (float)*(ptrs++);
        }
      } break;
      case 4 : {
        float
          *ptr_r = spt.images[0].data(0,0,0,0), *ptr_g = spt.images[0].data(0,0,0,1),
          *ptr_b = spt.images[0].data(0,0,0,2), *ptr_a = spt.images[0].data(0,0,0,3);
        for (int xy = 0; xy<wh; ++xy) {
          *(ptr_r++) = (float)*(ptrs++); *(ptr_g++) = (float)*(ptrs++); *(ptr_b++) = (float)*(ptrs++); *(ptr_a++) = (float)*(ptrs++);
        }
      } break;
      }
    } else { // Else, compute a 'hand-made' set of thumbnails.
      CImgList<unsigned char> images_uchar;
      get_input_layers(images_uchar);
      const double factor = gimp_zoom_preview_get_factor(GIMP_ZOOM_PREVIEW(gui_preview));
      int xp, yp; gimp_preview_get_position(GIMP_PREVIEW(gui_preview),&xp,&yp);
      spt.images.assign(images_uchar.size());
      cimglist_for(images_uchar,l) {
        const int
          cw = images_uchar[l].width(),
          ch = images_uchar[l].height(),
          x0 = (int)(xp/factor)*cw/w,
          y0 = (int)(yp/factor)*ch/h,
          x1 = (int)((xp+w)/factor)*cw/w - 1,
          y1 = (int)((yp+h)/factor)*ch/h - 1;
        images_uchar[l].get_crop(x0,y0,x1,y1).resize(w,h,1,-100).move_to(spt.images[l]);
        images_uchar[l].assign();
      }
    }
  }

  // Run G'MIC.
  process_thread(&spt);

  // Transfer the output layers back into GIMP preview.
  CImg<float> img;
  switch (get_preview_mode()) {
  case 0 : // Preview 1st layer
    if (spt.images && spt.images.size()>0) spt.images[0].move_to(img);
    calibrate_image(img,channels,true);
    break;
  case 1 : // Preview 2nd layer
    if (spt.images && spt.images.size()>1) spt.images[1].move_to(img);
    calibrate_image(img,channels,true);
    break;
  case 2 : // Preview 3rd layer
    if (spt.images && spt.images.size()>2) spt.images[2].move_to(img);
    calibrate_image(img,channels,true);
    break;
  case 3 : // Preview 4th layer
    if (spt.images && spt.images.size()>3) spt.images[3].move_to(img);
    calibrate_image(img,channels,true);
    break;
  case 4 : { // Preview 1st->2nd layers
    if (spt.images.size()>2) spt.images.remove(2,spt.images.size()-1);
    cimglist_for(spt.images,l) calibrate_image(spt.images[l],channels,true);
    (spt.images>'x').move_to(img);
  } break;
  case 5 : { // Preview 1st->3nd layers
    if (spt.images.size()>3) spt.images.remove(3,spt.images.size()-1);
    cimglist_for(spt.images,l) calibrate_image(spt.images[l],channels,true);
    (spt.images>'x').move_to(img);
  } break;
  case 6 : { // Preview 1st->4nd layers
    if (spt.images.size()>4) spt.images.remove(4,spt.images.size()-1);
    cimglist_for(spt.images,l) calibrate_image(spt.images[l],channels,true);
    (spt.images>'x').move_to(img);
  } break;
  default : // Preview all layers
    cimglist_for(spt.images,l) calibrate_image(spt.images[l],channels,true);
    (spt.images>'x').move_to(img);
  }
  spt.images.assign();
  if (!img) { img.assign(w,h,1,4,0); calibrate_image(img,channels,true); }
  if (img.width()>img.height()) {
    const unsigned int _nh = img._height*w/img._width, nh = _nh?_nh:1;
    img.resize(w,nh,1,-100,2);
  } else {
    const unsigned int _nw = img._width*h/img._height, nw = _nw?_nw:1;
    img.resize(nw,h,1,-100,2);
  }
  if (img.width()!=w || img.height()!=h) img.resize(w,h,1,-100,0,0,0.5,0.5);
  convert_image_float2uchar(img);
  std::memcpy(ptr0,img.data(),w*h*channels*sizeof(unsigned char));
  gimp_preview_draw_buffer(GIMP_PREVIEW(gui_preview),ptr0,w*channels);
  g_free(ptr0);
}

// Create the parameters GUI for the chosen filter.
//--------------------------------------------------
void create_parameters_gui(const bool reset_params) {
  const unsigned int filter = get_current_filter();

  // Remove existing table in the parameters frame if necessary.
  GtkWidget *const child = GTK_WIDGET(gtk_bin_get_child(GTK_BIN(right_frame)));
  if (child) gtk_container_remove(GTK_CONTAINER(right_frame),child);

  // Create new table for the parameters frame.
  GtkWidget *table = 0;
  if (!filter) {  // No filter selected -> 1x1 table with default message.
    table = gtk_table_new(1,1,false);
    gtk_widget_show(table);
    GtkWidget *const label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label),t("<i>Select a filter...</i>"));
    gtk_widget_show(label);
    gtk_table_attach(GTK_TABLE(table),label,0,1,0,1,
                     (GtkAttachOptions)(GTK_EXPAND),(GtkAttachOptions)(GTK_EXPAND),0,0);
    gtk_misc_set_alignment (GTK_MISC(label),0,0.5);
    gtk_frame_set_label(GTK_FRAME(right_frame),NULL);
  } else { // Filter selected -> Build the table for setting the parameters.
    const unsigned int N = filter - 1;
    char s_label[1024] = { 0 };
    std::sprintf(s_label,"<b>  %s : </b>",gmic_entries[N].data());
    GtkWidget *const frame_title = gtk_label_new(NULL);
    gtk_widget_show(frame_title);
    gtk_label_set_markup(GTK_LABEL(frame_title),s_label);
    gtk_frame_set_label_widget(GTK_FRAME(right_frame),frame_title);

    // Count number of filter arguments.
    char argument_name[4096] = { 0 }, _argument_type[4096] = { 0 }, argument_arg[4096] = { 0 };
    unsigned int nb_arguments = 0;
    for (const char *argument = gmic_arguments[N].data(); *argument; ) {
      int err = std::sscanf(argument,"%4095[^=]=%4095[ a-zA-z](%4095[^)]",argument_name,_argument_type,&(argument_arg[0]=0));
      if (err!=3) err = std::sscanf(argument,"%4095[^=]=%4095[ a-zA-z]{%4095[^}]",argument_name,_argument_type,argument_arg);
      if (err!=3) err = std::sscanf(argument,"%4095[^=]=%4095[ a-zA-z][%4095[^]]",argument_name,_argument_type,argument_arg);
      if (err>=2) {
        argument += std::strlen(argument_name) + std::strlen(_argument_type) + std::strlen(argument_arg) + 3;
        if (*argument) ++argument;
        ++nb_arguments;
      } else break;
    }

    if (!nb_arguments) { // Filter requires no parameters -> 1x1 table with default message.
      table = gtk_table_new(1,1,false);
      gtk_widget_show(table);
      GtkWidget *label = gtk_label_new(NULL);
      gtk_label_set_markup(GTK_LABEL(label),t("<i>No parameters to set...</i>"));
      gtk_widget_show(label);
      gtk_table_attach(GTK_TABLE(table),label,0,1,0,1,
                       (GtkAttachOptions)(GTK_EXPAND),(GtkAttachOptions)(GTK_EXPAND),0,0);
      gtk_misc_set_alignment(GTK_MISC(label),0,0.5);
      if (event_infos) delete[] event_infos; event_infos = 0;
      set_filter_nbparams(filter,0);

    } else { // Filter requires parameters -> Create parameters table.

      // Create new table for putting parameters inside.
      table = gtk_table_new(nb_arguments,3,false);
      gtk_widget_show(table);
      gtk_table_set_row_spacings(GTK_TABLE(table),6);
      gtk_table_set_col_spacings(GTK_TABLE(table),6);
      gtk_container_set_border_width(GTK_CONTAINER(table),8);

      // Parse arguments list and add recognized one to the table.
      if (event_infos) delete[] event_infos; event_infos = new void*[2*nb_arguments];
      int current_argument = 0, current_table_line = 0;
      for (const char *argument = gmic_arguments[N].data(); *argument; ) {
        int err = std::sscanf(argument,"%4095[^=]=%4095[ a-zA-Z_](%4095[^)]",argument_name,_argument_type,&(argument_arg[0]=0));
        if (err!=3) err = std::sscanf(argument,"%4095[^=]=%4095[ a-zA-Z_][%4095[^]]",argument_name,_argument_type,argument_arg);
        if (err!=3) err = std::sscanf(argument,"%4095[^=]=%4095[ a-zA-Z_]{%4095[^}]",argument_name,_argument_type,argument_arg);
        if (err>=2) {
          argument += std::strlen(argument_name) + std::strlen(_argument_type) + std::strlen(argument_arg) + 3;
          if (*argument) ++argument;
          cimg::strpare(argument_name,' ',false,true); cimg::strpare(argument_name,'\"',true); cimg::strescape(argument_name);
          cimg::strpare(_argument_type,' ',false,true);
          const bool is_silent_argument = (*_argument_type=='_');
          char
            *const argument_type = _argument_type + (is_silent_argument?1:0),
            *const argument_value = get_filter_parameter(filter,current_argument);

          // Check for a float-valued argument.
          bool found_valid_argument = false;
          if (!found_valid_argument && !cimg::strcasecmp(argument_type,"float")) {
            float default_value = 0, min_value = 0, max_value = 100;
            std::setlocale(LC_NUMERIC,"C");
            std::sscanf(argument_arg,"%f%*c%f%*c%f",&default_value,&min_value,&max_value);
            if (!reset_params && std::sscanf(argument_value,"%f",&default_value)) {}
            GtkObject *const scale = gimp_scale_entry_new(GTK_TABLE(table),0,current_table_line,argument_name,50,6,
                                                          (gdouble)default_value,(gdouble)min_value,(gdouble)max_value,
                                                          0.1,0.1,2,true,0,0,0,0);
            event_infos[2*current_argument] = (void*)current_argument;
            event_infos[2*current_argument+1] = (void*)0;
            on_float_parameter_changed(GTK_ADJUSTMENT(scale),event_infos+2*current_argument);
            g_signal_connect(scale,"value_changed",G_CALLBACK(on_float_parameter_changed),event_infos+2*current_argument);
            if (!is_silent_argument) g_signal_connect(scale,"value_changed",G_CALLBACK(_gimp_preview_invalidate),0);
            found_valid_argument = true; ++current_argument;
          }

          // Check for an int-valued argument.
          if (!found_valid_argument && !cimg::strcasecmp(argument_type,"int")) {
            float default_value = 0, min_value = 0, max_value = 100;
            std::setlocale(LC_NUMERIC,"C");
            std::sscanf(argument_arg,"%f%*c%f%*c%f",&default_value,&min_value,&max_value);
            if (!reset_params && std::sscanf(argument_value,"%f",&default_value)) {}
            GtkObject *const scale = gimp_scale_entry_new(GTK_TABLE(table),0,current_table_line,argument_name,50,6,
                                                          (gdouble)(int)cimg::round(default_value,1.0f),
                                                          (gdouble)(int)cimg::round(min_value,1.0f),
                                                          (gdouble)(int)cimg::round(max_value,1.0f),
                                                          1,1,0,true,0,0,0,0);
            event_infos[2*current_argument] = (void*)current_argument;
            event_infos[2*current_argument+1] = (void*)0;
            on_int_parameter_changed(GTK_ADJUSTMENT(scale),event_infos+2*current_argument);
            g_signal_connect(scale,"value_changed",G_CALLBACK(on_int_parameter_changed),event_infos+2*current_argument);
            if (!is_silent_argument) g_signal_connect(scale,"value_changed",G_CALLBACK(_gimp_preview_invalidate),0);
            found_valid_argument = true; ++current_argument;
          }

          // Check for a bool-valued argument.
          if (!found_valid_argument && !cimg::strcasecmp(argument_type,"bool")) {
            cimg::strpare(argument_arg,' ',false,true); cimg::strpare(argument_arg,'\"',true);
            const bool
              default_value = !(!*argument_arg || !cimg::strcasecmp(argument_arg,"false") || (argument_arg[0]=='0' && argument_arg[1]==0)),
              current_value = !(!*argument_value || !cimg::strcasecmp(argument_value,"false") || (argument_value[0]=='0' && argument_value[1]==0)),
              state = (reset_params || !*argument_value)?default_value:current_value;
            GtkWidget *const checkbutton = gtk_check_button_new_with_label(argument_name);
            gtk_widget_show(checkbutton);
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbutton),state);
            gtk_table_attach(GTK_TABLE(table),checkbutton,0,3,current_table_line,current_table_line+1,
                             (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),GTK_SHRINK,0,0);
            event_infos[2*current_argument] = (void*)current_argument;
            event_infos[2*current_argument+1] = (void*)0;
            on_bool_parameter_changed(GTK_CHECK_BUTTON(checkbutton),event_infos+2*current_argument);
            g_signal_connect(checkbutton,"toggled",G_CALLBACK(on_bool_parameter_changed),event_infos+2*current_argument);
            if (!is_silent_argument) g_signal_connect(checkbutton,"toggled",G_CALLBACK(_gimp_preview_invalidate),0);
            found_valid_argument = true; ++current_argument;
          }

          // Check for a choice-valued argument.
          if (!found_valid_argument && !cimg::strcasecmp(argument_type,"choice")) {
            GtkWidget *const label = gtk_label_new(argument_name);
            gtk_widget_show(label);
            gtk_table_attach(GTK_TABLE(table),label,0,1,current_table_line,current_table_line+1,GTK_FILL,GTK_SHRINK,0,0);
            gtk_misc_set_alignment(GTK_MISC(label),0,0.5);
            GtkWidget *const combobox = gtk_combo_box_new_text();
            gtk_widget_show(combobox);
            char s_entry[4096] = { 0 }, end = 0; int err = 0;
            unsigned int default_value = 0;
            const char *entries = argument_arg;
            if (std::sscanf(entries,"%u",&default_value)==1) entries+=std::sprintf(s_entry,"%u",default_value) + 1;
            while (*entries) {
              if ((err = std::sscanf(entries,"%4095[^,]%c",s_entry,&end))>0) {
                entries += std::strlen(s_entry) + (err==2?1:0);
                cimg::strpare(s_entry,' ',false,true); cimg::strpare(s_entry,'\"',true);
                gtk_combo_box_append_text(GTK_COMBO_BOX(combobox),s_entry);
              } else break;
            }
            if (!reset_params && std::sscanf(argument_value,"%u",&default_value)) {}
            gtk_combo_box_set_active(GTK_COMBO_BOX(combobox),default_value);
            gtk_table_attach(GTK_TABLE(table),combobox,1,3,current_table_line,current_table_line+1,
                             (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),GTK_FILL,0,0);
            event_infos[2*current_argument] = (void*)current_argument;
            event_infos[2*current_argument+1] = (void*)0;
            on_list_parameter_changed(GTK_COMBO_BOX(combobox),event_infos+2*current_argument);
            g_signal_connect(combobox,"changed",G_CALLBACK(on_list_parameter_changed),event_infos+2*current_argument);
            if (!is_silent_argument) g_signal_connect(combobox,"changed",G_CALLBACK(_gimp_preview_invalidate),0);
            found_valid_argument = true; ++current_argument;
          }

          // Check for a text-valued argument.
          if (!found_valid_argument && !cimg::strcasecmp(argument_type,"text")) {
            GtkWidget *const label = gtk_label_new(argument_name);
            gtk_widget_show(label);
            gtk_table_attach(GTK_TABLE(table),label,0,1,current_table_line,current_table_line+1,GTK_FILL,GTK_SHRINK,0,0);
            gtk_misc_set_alignment(GTK_MISC(label),0,0.5);
            GtkWidget *const entry = gtk_entry_new_with_max_length(1023);
            gtk_widget_show(entry);
            if (!reset_params && *argument_value) {
              const unsigned int l = std::strlen(argument_value);
              argument_value[l-1] = 0;
              gtk_entry_set_text(GTK_ENTRY(entry),argument_value+1);
            } else {
              cimg::strpare(argument_arg,' ',false,true); cimg::strpare(argument_arg,'\"',true);
              gtk_entry_set_text(GTK_ENTRY(entry),argument_arg);
            }
            gtk_table_attach(GTK_TABLE(table),entry,1,2,current_table_line,current_table_line+1,
                             (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),(GtkAttachOptions)0,0,0);
            GtkWidget *const button = gtk_button_new_with_label(t("Update"));
            gtk_widget_show(button);
            gtk_table_attach(GTK_TABLE(table),button,2,3,current_table_line,current_table_line+1,GTK_FILL,GTK_SHRINK,0,0);
            event_infos[2*current_argument] = (void*)current_argument;
            event_infos[2*current_argument+1] = (void*)entry;
            on_text_parameter_changed(event_infos+2*current_argument);
            g_signal_connect_swapped(button,"clicked",G_CALLBACK(on_text_parameter_changed),event_infos+2*current_argument);
            g_signal_connect_swapped(entry,"changed",G_CALLBACK(on_text_parameter_changed),event_infos+2*current_argument);
            if (!is_silent_argument) {
              g_signal_connect(button,"clicked",G_CALLBACK(_gimp_preview_invalidate),0);
              g_signal_connect(entry,"activate",G_CALLBACK(_gimp_preview_invalidate),0);
            }
            found_valid_argument = true; ++current_argument;
          }

          // Check for a filename or folder name argument.
          if (!found_valid_argument && (!cimg::strcasecmp(argument_type,"file") ||
                                        !cimg::strcasecmp(argument_type,"folder"))) {
            GtkWidget *const label = gtk_label_new(argument_name);
            gtk_widget_show(label);
            gtk_table_attach(GTK_TABLE(table),label,0,1,current_table_line,current_table_line+1,GTK_FILL,GTK_SHRINK,0,0);
            gtk_misc_set_alignment(GTK_MISC(label),0,0.5);
            GtkWidget *const file_chooser = gtk_file_chooser_button_new(argument_name,
                                                                        cimg::uncase(argument_type[1])=='i'?GTK_FILE_CHOOSER_ACTION_OPEN:
                                                                        GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
            gtk_widget_show(file_chooser);
            if (!reset_params && *argument_value) {
              const unsigned int l = std::strlen(argument_value);
              argument_value[l-1] = 0;
              gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(file_chooser),argument_value+1);
            } else {
              cimg::strpare(argument_arg,' ',false,true); cimg::strpare(argument_arg,'\"',true);
              gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(file_chooser),argument_arg);
            }
            gtk_table_attach(GTK_TABLE(table),file_chooser,1,3,current_table_line,current_table_line+1,
                             (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),(GtkAttachOptions)0,0,0);
            event_infos[2*current_argument] = (void*)current_argument;
            event_infos[2*current_argument+1] = (void*)0;
            on_file_parameter_changed(GTK_FILE_CHOOSER(file_chooser),event_infos+2*current_argument);
            g_signal_connect(file_chooser,"selection-changed",G_CALLBACK(on_file_parameter_changed),event_infos+2*current_argument);
            if (!is_silent_argument) g_signal_connect(file_chooser,"selection-changed",G_CALLBACK(_gimp_preview_invalidate),0);
            found_valid_argument = true; ++current_argument;
          }

          // Check for a color argument.
          if (!found_valid_argument && !cimg::strcasecmp(argument_type,"color")) {
            GtkWidget *const hbox = gtk_hbox_new(false,6);
            gtk_widget_show(hbox);
            gtk_table_attach(GTK_TABLE(table),hbox,0,2,current_table_line,current_table_line+1,GTK_FILL,GTK_SHRINK,0,0);
            GtkWidget *const label = gtk_label_new(argument_name);
            gtk_widget_show(label);
            gtk_box_pack_start(GTK_BOX(hbox),label,false,false,0);
            GtkWidget *const color_chooser = gtk_color_button_new();
            gtk_widget_show(color_chooser);
            gtk_color_button_set_title(GTK_COLOR_BUTTON(color_chooser),argument_name);
            gtk_box_pack_start(GTK_BOX(hbox),color_chooser,false,false,0);
            unsigned int red = 0, green = 0, blue = 0, alpha = 255;
            const int err = std::sscanf(argument_arg,"%u%*c%u%*c%u%*c%u",&red,&green,&blue,&alpha);
            if (!reset_params && std::sscanf(argument_value,"%u%*c%u%*c%u%*c%u",&red,&green,&blue,&alpha)==err) {}
            GdkColor col;
            col.pixel = 0; col.red = red<<8; col.green = green<<8; col.blue = blue<<8;
            gtk_color_button_set_color(GTK_COLOR_BUTTON(color_chooser),&col);
            if (err==4) {
              gtk_color_button_set_use_alpha(GTK_COLOR_BUTTON(color_chooser),true);
              gtk_color_button_set_alpha(GTK_COLOR_BUTTON(color_chooser),alpha<<8);
            } else gtk_color_button_set_use_alpha(GTK_COLOR_BUTTON(color_chooser),false);
            event_infos[2*current_argument] = (void*)current_argument;
            event_infos[2*current_argument+1] = (void*)0;
            on_color_parameter_changed(GTK_COLOR_BUTTON(color_chooser),event_infos+2*current_argument);
            g_signal_connect(color_chooser,"color-set",G_CALLBACK(on_color_parameter_changed),event_infos+2*current_argument);
            if (!is_silent_argument) g_signal_connect(color_chooser,"color-set",G_CALLBACK(_gimp_preview_invalidate),0);
            found_valid_argument = true; ++current_argument;
          }

          // Check for a note.
          if (!found_valid_argument && !cimg::strcasecmp(argument_type,"note")) {
            cimg::strpare(argument_arg,' ',false,true); cimg::strpare(argument_arg,'\"',true); cimg::strescape(argument_arg);
            GtkWidget *const label = gtk_label_new(NULL);
            gtk_label_set_markup(GTK_LABEL(label),argument_arg);
            gtk_label_set_line_wrap(GTK_LABEL(label),true);
            gtk_widget_show(label);
            gtk_table_attach(GTK_TABLE(table),label,0,3,current_table_line,current_table_line+1,
                             (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),GTK_SHRINK,0,0);
            gtk_misc_set_alignment(GTK_MISC(label),0,0.5);
            found_valid_argument = true;
          }

          // Check for a link.
          if (!found_valid_argument && !cimg::strcasecmp(argument_type,"link")) {
            char label[1024] = { 0 }, url[1024] = { 0 };
            if (std::sscanf(argument_arg,"%1023[^,],%1023s",label,url)==1) std::strcpy(url,label);
            cimg::strpare(label,' ',false,true); cimg::strpare(label,'\"',true); cimg::strescape(label);
            cimg::strpare(url,' ',false,true); cimg::strpare(url,'\"',true);
            GtkWidget *const link = gtk_link_button_new_with_label(url,label);
            gtk_widget_show(link);
            gtk_table_attach(GTK_TABLE(table),link,0,3,current_table_line,current_table_line+1,
                             GTK_SHRINK,GTK_SHRINK,0,0);
            found_valid_argument = true;
          }

          // Check for a value.
          if (!found_valid_argument && !cimg::strcasecmp(argument_type,"value")) {
            cimg::strpare(argument_arg,' ',false,true); cimg::strpare(argument_arg,'\"',true);
            set_filter_parameter(filter,current_argument,argument_arg);
            found_valid_argument = true; ++current_argument;
          }

          if (!found_valid_argument) {
            if (get_verbosity_mode()>0)
              std::fprintf(stderr,"\n*** Plug-in 'gmic_gimp' : Found invalid parameter type '%s' for argument '%s'.\n",
                           argument_type,argument_name);
          } else ++current_table_line;
        } else break;
      }
      set_filter_nbparams(filter,current_argument);
    }
  }
  gtk_container_add(GTK_CONTAINER(right_frame),table);

  // Take care that the size of the parameter table is right.
  GtkRequisition requisition; gtk_widget_size_request(table,&requisition);
  gtk_widget_set_size_request(table,cimg::max(400,requisition.width),-1);
  gtk_widget_show(dialog_window);
  set_preview_factor();
}

// Create main plug-in dialog window and wait for events.
//-------------------------------------------------------
bool create_dialog_gui() {

  // Init GUI_specific variables
  _create_dialog_gui = true;
  gimp_ui_init("gmic",true);
  event_infos = 0;

  // Create main dialog window with buttons.
  char dialog_title[1024] = { 0 };
  std::sprintf(dialog_title,"%s - %d.%d.%d.%d",
               t("G'MIC for GIMP"),gmic_version/1000,(gmic_version/100)%10,(gmic_version/10)%10,gmic_version%10);

  dialog_window = gimp_dialog_new(dialog_title,"gmic",0,(GtkDialogFlags)0,0,0,NULL);
  gimp_window_set_transient(GTK_WINDOW(dialog_window));

  g_signal_connect(dialog_window,"close",G_CALLBACK(on_dialog_cancel_clicked),0);
  g_signal_connect(dialog_window,"delete-event",G_CALLBACK(on_dialog_cancel_clicked),0);

  GtkWidget *const cancel_button = gtk_dialog_add_button(GTK_DIALOG(dialog_window),GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL);
  g_signal_connect(cancel_button,"clicked",G_CALLBACK(on_dialog_cancel_clicked),0);

  GtkWidget *const reset_button = gtk_dialog_add_button(GTK_DIALOG(dialog_window),GIMP_STOCK_RESET,1);
  g_signal_connect(reset_button,"clicked",G_CALLBACK(on_dialog_reset_clicked),0);

  GtkWidget *const apply_button = gtk_dialog_add_button(GTK_DIALOG(dialog_window),GTK_STOCK_APPLY,GTK_RESPONSE_APPLY);
  g_signal_connect(apply_button,"clicked",G_CALLBACK(on_dialog_apply_clicked),0);

  GtkWidget *const ok_button = gtk_dialog_add_button(GTK_DIALOG(dialog_window),GTK_STOCK_OK,GTK_RESPONSE_OK);
  g_signal_connect(ok_button,"clicked",G_CALLBACK(gtk_main_quit),0);

  GtkWidget *const dialog_hbox = gtk_hbox_new(false,0);
  gtk_widget_show(dialog_hbox);
  gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog_window)->vbox),dialog_hbox);

  // Create the left pane.
  left_pane = gtk_vbox_new(false,4);
  gtk_widget_show(left_pane);
  gtk_box_pack_start(GTK_BOX(dialog_hbox),left_pane,true,true,0);

  GtkWidget *const image_align = gtk_alignment_new(0.1,0,0,0);
  gtk_widget_show(image_align);
  gtk_box_pack_end(GTK_BOX(left_pane),image_align,false,false,0);
  const unsigned int logo_width = 102, logo_height = 22;
  GdkPixbuf *const pixbuf = gdk_pixbuf_new_from_data(data_gmic_logo,GDK_COLORSPACE_RGB,false,8,logo_width,logo_height,3*logo_width,0,0);
  GtkWidget *const image = gtk_image_new_from_pixbuf(pixbuf);
  gtk_widget_show(image);
  gtk_container_add(GTK_CONTAINER(image_align),image);

  GtkWidget *const left_align = gtk_alignment_new(0,0,0,0);
  gtk_widget_show(left_align);
  gtk_box_pack_end(GTK_BOX(left_pane),left_align,false,false,0);

  GtkWidget *const left_frame = gtk_frame_new(NULL);
  gtk_widget_show(left_frame);
  gtk_container_set_border_width(GTK_CONTAINER(left_frame),4);
  gtk_container_add(GTK_CONTAINER(left_align),left_frame);

  GtkWidget *const frame_title = gtk_label_new(NULL);
  gtk_widget_show(frame_title);
  gtk_label_set_markup(GTK_LABEL(frame_title),t("<b> Input / Output : </b>"));
  gtk_frame_set_label_widget(GTK_FRAME(left_frame),frame_title);

  GtkWidget *const left_table = gtk_table_new(6,1,false);
  gtk_widget_show(left_table);
  gtk_table_set_row_spacings(GTK_TABLE(left_table),6);
  gtk_table_set_col_spacings(GTK_TABLE(left_table),6);
  gtk_container_set_border_width(GTK_CONTAINER(left_table),8);
  gtk_container_add(GTK_CONTAINER(left_frame),left_table);

  GtkWidget *const input_combobox = gtk_combo_box_new_text();
  gtk_widget_show(input_combobox);
  gtk_combo_box_append_text(GTK_COMBO_BOX(input_combobox),t("Input layers..."));
  gtk_combo_box_append_text(GTK_COMBO_BOX(input_combobox),"-");
  gtk_combo_box_append_text(GTK_COMBO_BOX(input_combobox),t("None"));
  gtk_combo_box_append_text(GTK_COMBO_BOX(input_combobox),t("Active (default)"));
  gtk_combo_box_append_text(GTK_COMBO_BOX(input_combobox),t("All"));
  gtk_combo_box_append_text(GTK_COMBO_BOX(input_combobox),t("Active & below"));
  gtk_combo_box_append_text(GTK_COMBO_BOX(input_combobox),t("Active & above"));
  gtk_combo_box_append_text(GTK_COMBO_BOX(input_combobox),t("All visibles"));
  gtk_combo_box_append_text(GTK_COMBO_BOX(input_combobox),t("All invisibles"));
  gtk_combo_box_append_text(GTK_COMBO_BOX(input_combobox),t("All visibles (decr.)"));
  gtk_combo_box_append_text(GTK_COMBO_BOX(input_combobox),t("All invisibles (decr.)"));
  gtk_combo_box_append_text(GTK_COMBO_BOX(input_combobox),t("All (decr.)"));
  gtk_combo_box_set_active(GTK_COMBO_BOX(input_combobox),get_input_mode(false));

  gtk_table_attach_defaults(GTK_TABLE(left_table),input_combobox,0,1,0,1);
  g_signal_connect(input_combobox,"changed",G_CALLBACK(on_dialog_input_mode_changed),0);

  GtkWidget *const output_combobox = gtk_combo_box_new_text();
  gtk_widget_show(output_combobox);
  gtk_combo_box_append_text(GTK_COMBO_BOX(output_combobox),t("Output mode..."));
  gtk_combo_box_append_text(GTK_COMBO_BOX(output_combobox),"-");
  gtk_combo_box_append_text(GTK_COMBO_BOX(output_combobox),t("In place (default)"));
  gtk_combo_box_append_text(GTK_COMBO_BOX(output_combobox),t("New layer(s)"));
  gtk_combo_box_append_text(GTK_COMBO_BOX(output_combobox),t("New active layer(s)"));
  gtk_combo_box_append_text(GTK_COMBO_BOX(output_combobox),t("New image"));
  gtk_combo_box_set_active(GTK_COMBO_BOX(output_combobox),get_output_mode(false));
  gtk_table_attach_defaults(GTK_TABLE(left_table),output_combobox,0,1,1,2);
  g_signal_connect(output_combobox,"changed",G_CALLBACK(on_dialog_output_mode_changed),0);

  GtkWidget *const verbosity_combobox = gtk_combo_box_new_text();
  gtk_widget_show(verbosity_combobox);
  gtk_combo_box_append_text(GTK_COMBO_BOX(verbosity_combobox),t("Output messages..."));
  gtk_combo_box_append_text(GTK_COMBO_BOX(verbosity_combobox),"-");
  gtk_combo_box_append_text(GTK_COMBO_BOX(verbosity_combobox),t("Quiet (default)"));
  gtk_combo_box_append_text(GTK_COMBO_BOX(verbosity_combobox),t("Verbose"));
  gtk_combo_box_append_text(GTK_COMBO_BOX(verbosity_combobox),t("Very verbose"));
  gtk_combo_box_append_text(GTK_COMBO_BOX(verbosity_combobox),t("Debug mode"));
  gtk_combo_box_set_active(GTK_COMBO_BOX(verbosity_combobox),get_verbosity_mode(false));
  gtk_table_attach_defaults(GTK_TABLE(left_table),verbosity_combobox,0,1,2,3);
  g_signal_connect(verbosity_combobox,"changed",G_CALLBACK(on_dialog_verbosity_mode_changed),0);

  GtkWidget *const preview_combobox = gtk_combo_box_new_text();
  gtk_widget_show(preview_combobox);
  gtk_combo_box_append_text(GTK_COMBO_BOX(preview_combobox),t("Output preview..."));
  gtk_combo_box_append_text(GTK_COMBO_BOX(preview_combobox),"-");
  gtk_combo_box_append_text(GTK_COMBO_BOX(preview_combobox),t("1st output (default)"));
  gtk_combo_box_append_text(GTK_COMBO_BOX(preview_combobox),t("2nd output"));
  gtk_combo_box_append_text(GTK_COMBO_BOX(preview_combobox),t("3rd output"));
  gtk_combo_box_append_text(GTK_COMBO_BOX(preview_combobox),t("4th output"));
  gtk_combo_box_append_text(GTK_COMBO_BOX(preview_combobox),t("1st -> 2nd"));
  gtk_combo_box_append_text(GTK_COMBO_BOX(preview_combobox),t("1st -> 3rd"));
  gtk_combo_box_append_text(GTK_COMBO_BOX(preview_combobox),t("1st -> 4th"));
  gtk_combo_box_append_text(GTK_COMBO_BOX(preview_combobox),t("All outputs"));
  gtk_combo_box_set_active(GTK_COMBO_BOX(preview_combobox),get_preview_mode(false));
  gtk_table_attach_defaults(GTK_TABLE(left_table),preview_combobox,0,1,3,4);
  g_signal_connect(preview_combobox,"changed",G_CALLBACK(on_dialog_preview_mode_changed),0);

  GtkWidget *const maximize_button = gtk_button_new_with_mnemonic(t("Maximize prev_iew"));
  gtk_widget_show(maximize_button);
  gtk_table_attach_defaults(GTK_TABLE(left_table),maximize_button,0,1,4,5);
  g_signal_connect(maximize_button,"clicked",G_CALLBACK(on_dialog_maximize_button_clicked),0);

  GtkWidget *const preview_button = gtk_button_new_with_mnemonic(t("_Manual preview"));
  gtk_widget_show(preview_button);
  gtk_table_attach_defaults(GTK_TABLE(left_table),preview_button,0,1,5,6);
  g_signal_connect(preview_button,"clicked",G_CALLBACK(on_dialog_preview_button_clicked),0);

  drawable_preview = gimp_drawable_get(gimp_image_get_active_drawable(image_id));
  gui_preview = gimp_zoom_preview_new(drawable_preview);
  gtk_widget_show(gui_preview);
  gtk_box_pack_end(GTK_BOX(left_pane),gui_preview,true,true,0);
  g_signal_connect(gui_preview,"invalidated",G_CALLBACK(process_preview),0);
  g_signal_connect(dialog_window,"size-request",G_CALLBACK(on_dialog_resized),0);

  // Create the middle pane.
  GtkWidget *const middle_frame = gtk_frame_new(NULL);
  gtk_widget_show(middle_frame);
  gtk_container_set_border_width(GTK_CONTAINER(middle_frame),4);
  gtk_box_pack_start(GTK_BOX(dialog_hbox),middle_frame,false,false,0);

  GtkWidget *const middle_pane = gtk_vbox_new(false,4);
  gtk_widget_show(middle_pane);
  gtk_container_add(GTK_CONTAINER(middle_frame),middle_pane);

  GtkWidget *const scrolled_window = gtk_scrolled_window_new(NULL,NULL);
  gtk_widget_show(scrolled_window);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
  gtk_box_pack_start(GTK_BOX(middle_pane),scrolled_window,true,true,0);

  GtkWidget *const tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(tree_view_store));
  gtk_widget_show(tree_view);
  gtk_container_add(GTK_CONTAINER(scrolled_window),tree_view);

  GtkWidget *const tree_hbox = gtk_hbox_new(false,6);
  gtk_widget_show(tree_hbox);
  gtk_box_pack_start(GTK_BOX(middle_pane),tree_hbox,false,false,0);

  GtkWidget *const update_button = gtk_button_new_from_stock(GTK_STOCK_REFRESH);
  gtk_widget_show(update_button);
  gtk_box_pack_start(GTK_BOX(tree_hbox),update_button,false,false,0);
  g_signal_connect_swapped(update_button,"clicked",G_CALLBACK(on_dialog_update_clicked),tree_view);

  GtkWidget *const internet_checkbutton = gtk_check_button_new_with_mnemonic(t("Internet updates"));
  gtk_widget_show(internet_checkbutton);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(internet_checkbutton),get_net_update());
  gtk_box_pack_start(GTK_BOX(tree_hbox),internet_checkbutton,false,false,0);
  g_signal_connect(internet_checkbutton,"toggled",G_CALLBACK(on_dialog_net_update_toggled),0);

  tree_mode_button = gtk_button_new();
  gtk_box_pack_start(GTK_BOX(tree_hbox),tree_mode_button,false,false,0);
  g_signal_connect_swapped(tree_mode_button,"clicked",G_CALLBACK(on_dialog_tree_mode_clicked),tree_view);

  GtkTreeViewColumn *const column = gtk_tree_view_column_new();
  gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view),column);
  flush_tree_view(tree_view);

  GtkTreeSelection *const selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_view));
  gtk_tree_selection_set_mode(selection,GTK_SELECTION_SINGLE);
  g_signal_connect(G_OBJECT(selection),"changed",G_CALLBACK(on_filter_changed),0);

  // Create the right pane.
  GtkWidget *const right_pane = gtk_vbox_new(false,0);
  gtk_widget_show(right_pane);
  gtk_box_pack_start(GTK_BOX(dialog_hbox),right_pane,false,false,0);

  right_frame = gtk_frame_new(NULL);
  gtk_widget_show(right_frame);
  gtk_container_set_border_width(GTK_CONTAINER(right_frame),4);
  gtk_box_pack_start(GTK_BOX(right_pane),right_frame,true,true,0);

  // Show dialog window and wait for user response.
  create_parameters_gui(false);
  gtk_main();

  // Destroy dialog box widget and free resources.
  gtk_widget_destroy(dialog_window);
  if (tree_mode_stockbutton) gtk_widget_destroy(tree_mode_stockbutton);
  if (event_infos) delete[] event_infos;
  return _create_dialog_gui;
}

// 'Run' function, required by the GIMP plug-in API.
//--------------------------------------------------
void gmic_run(const gchar *name, gint nparams, const GimpParam *param, gint *nreturn_vals, GimpParam **return_vals) {

  // Init plug-in variables.
  try {
    set_locale();
    static GimpParam output_values[1];
    output_values[0].type = GIMP_PDB_STATUS;
    output_values[0].data.d_status = GIMP_PDB_SUCCESS;
    *return_vals  = output_values;
    *nreturn_vals = 1;
    name = 0;
    nparams = 0;
    run_mode = (GimpRunMode)param[0].data.d_int32;

    // Init filters and images.
    update_filters_definition(false);
    image_id = param[1].data.d_drawable;
    gimp_tile_cache_ntiles(2*(gimp_image_width(image_id)/gimp_tile_width()+1));

    // Check for run mode.
    switch (run_mode) {

    case GIMP_RUN_INTERACTIVE : {
      if (create_dialog_gui()) {
        process_image(0);
        const char *const command_line = get_command_line(false);
        if (command_line) { // Remember command line for the next use of the filter.
          char s_tmp[256] = { 0 };
          std::sprintf(s_tmp,"gmic_command_line%u",get_current_filter());
          gimp_set_data(s_tmp,command_line,std::strlen(command_line));
        }
      }
    } break;

    case GIMP_RUN_WITH_LAST_VALS : {
      const unsigned int filter = get_current_filter();
      if (filter) {
        char s_tmp[256] = { 0 };
        std::sprintf(s_tmp,"gmic_command_line%u",filter);
        char command_line[4096] = { 0 };
        gimp_get_data(s_tmp,&command_line);
        process_image(command_line);
      }
    } break;

    case GIMP_RUN_NONINTERACTIVE : {
      const unsigned int _input_mode = get_input_mode();
      set_input_mode(param[3].data.d_int32 + 2);
      process_image(param[4].data.d_string);
      set_input_mode(_input_mode + 2);
    } break;
    }

    // Free plug-in resources.
    delete[] gmic_custom_commands;
  } catch (CImgException &e) {
    std::fprintf(stderr,"\n*** Plug-in 'gmic_gimp' : Execution error in plug-in code :\n*** %s\n",e.what());
  }
}

// 'Query' function, required by the GIMP plug-in API.
//----------------------------------------------------
void gmic_query() {
  static const GimpParamDef args[] = {
    {GIMP_PDB_INT32,    (gchar*)"run_mode", (gchar*)"Interactive, non-interactive"},
    {GIMP_PDB_IMAGE,    (gchar*)"image",    (gchar*)"Input image"},
    {GIMP_PDB_DRAWABLE, (gchar*)"drawable", (gchar*)"Input drawable (unused)"},
    {GIMP_PDB_INT32,    (gchar*)"input",    (gchar*)"Input layers mode, when non-interactive"
     "(0=none, 1=active, 2=all, 3=active & below, 4=active & above, 5=all visibles, 6=all invisibles, "
     "7=all visibles (decr.), 8=all invisibles (decr.), 9=all (decr.))"},
    {GIMP_PDB_STRING,   (gchar*)"command",  (gchar*)"G'MIC command string, when non-interactive"},
  };

  set_locale();
  gimp_install_procedure("plug-in-gmic",             // name
                         "G'MIC",                    // blurb
                         "G'MIC",                    // help
                         "David Tschumperl",        // author
                         "David Tschumperl",        // copyright
                         "2008",                     // date
                         "_G'MIC...",                // menu_path
                         "RGB*, GRAY*",              // image_types
                         GIMP_PLUGIN,                // type
                         G_N_ELEMENTS(args),         // nparams
                         0,                          // nreturn_vals
                         args,                       // params
                         0);                         // return_vals

  gimp_plugin_menu_register("plug-in-gmic", "<Image>/Filters");
}

GimpPlugInInfo PLUG_IN_INFO = { 0, 0, gmic_query, gmic_run };
MAIN()
