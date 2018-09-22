// Mark the rest of the file as a system include, for the purpose of silencing spurious warnings.
// This is a hack around https://github.com/sakra/cotire/issues/105 (cotire fails to pass
// the -isystem flag when it should.)
#if defined(__clang__)
#pragma clang system_header
#elif defined(__GNUC__) 
#pragma GCC system_header
#endif

// C++ standard library

#include <algorithm>
#include <array>
#include <bitset>
#include <cassert>
#include <clocale>
#include <cmath>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <exception>
#include <fstream>
#include <functional>
#include <initializer_list>
#include <iomanip>
#include <iostream>
#include <istream>
#include <iterator>
#include <limits>
#include <list>
#include <locale>
#include <map>
#include <memory>
#include <numeric>
#include <ostream>
#include <queue>
#include <random>
#include <set>
#include <sstream>
#include <stack>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

// GLEW

#include <GL/glew.h>

// wxWidgets

#include <wx/accel.h>
#include <wx/app.h>
#include <wx/apptrait.h>
#include <wx/bitmap.h>
#include <wx/bmpbuttn.h>
#include <wx/bookctrl.h>
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/choicdlg.h>
#include <wx/choice.h>
#include <wx/clipbrd.h>
#include <wx/clrpicker.h>
#include <wx/cmdline.h>
#include <wx/colour.h>
#include <wx/combobox.h>
#include <wx/confbase.h>
#include <wx/config.h>
#include <wx/control.h>
#include <wx/ctrlsub.h>
#include <wx/datetime.h>
#include <wx/dc.h>
#include <wx/dcbuffer.h>
#include <wx/dcclient.h>
#include <wx/debug.h>
#include <wx/defs.h>
#include <wx/dialog.h>
#include <wx/dir.h>
#include <wx/dirdlg.h>
#include <wx/display.h>
#include <wx/dnd.h>
#include <wx/event.h>
#include <wx/file.h>
#include <wx/fileconf.h>
#include <wx/filedlg.h>
#include <wx/filefn.h>
#include <wx/filename.h>
#include <wx/font.h>
#include <wx/frame.h>
#include <wx/gbsizer.h>
#include <wx/gdicmn.h>
#include <wx/generic/helpext.h>
#include <wx/glcanvas.h>
#include <wx/grid.h>
#include <wx/icon.h>
#include <wx/layout.h>
#include <wx/listbox.h>
#include <wx/listctrl.h>
#include <wx/log.h>
#include <wx/longlong.h>
#include <wx/menu.h>
#include <wx/menuitem.h>
#include <wx/msgdlg.h>
#include <wx/notebook.h>
#include <wx/panel.h>
#include <wx/persist.h>
#include <wx/persist/toplevel.h>
#include <wx/platinfo.h>
#include <wx/popupwin.h>
#include <wx/process.h>
#include <wx/radiobut.h>
#include <wx/scrolbar.h>
#include <wx/scrolwin.h>
#include <wx/settings.h>
#include <wx/simplebook.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/spinbutt.h>
#include <wx/spinctrl.h>
#include <wx/srchctrl.h>
#include <wx/sstream.h>
#include <wx/statbmp.h>
#include <wx/statbox.h>
#include <wx/statline.h>
#include <wx/stattext.h>
#include <wx/statusbr.h>
#include <wx/stdpaths.h>
#include <wx/string.h>
#include <wx/textctrl.h>
#include <wx/textdlg.h>
#include <wx/textentry.h>
#include <wx/tglbtn.h>
#include <wx/thread.h>
#include <wx/time.h>
#include <wx/timer.h>
#include <wx/tokenzr.h>
#include <wx/toolbar.h>
#include <wx/txtstrm.h>
#include <wx/utils.h>
#include <wx/window.h>
#include <wx/wupdlock.h>

// undefine some macros from glx.h that clash with TB
#if defined(None)
#undef None
#endif
#if defined(Always)
#undef Always
#endif
#if defined(True)
#undef True
#endif
#if defined(False)
#undef False
#endif
#if defined(Opposite)
#undef Opposite
#endif
#if defined(Bool)
#undef Bool
#endif
