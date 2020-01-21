/*
    Copyright (c) 2020 Harrison Wade, aka "RigoLigo RLC"

    This file is part of LC2KiCad.

    LC2KiCad is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as 
    published by the Free Software Foundation, either version 3 of
    the License, or (at your option) any later version.

    LC2KiCad is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with LC2KiCad. If not, see <https://www.gnu.org/licenses/>.
*/

#include <includes.hpp>
#include <lc2kicad.hpp>

using namespace std;
using namespace lc2kicad;

int main(int argc, const char** argv) {
  switch(argc)
  {
    case 1:
      displayUsage();
      break;
    case 2:
      if((strcmp(argv[1], "-h") * strcmp(argv[1], "--help")) == 0)
      {
        displayUsage();
        break;
      }
      else if ((strcmp(argv[1], "-v") * strcmp(argv[1], "--version")) == 0)
      {
        displayAbout();
        break;
      }
#ifdef ENABLE_EXCEPTION_TESTER
    case 3:
      if(strcmp(argv[1], "--testerr") == 0)
      {
        runtime_error *e = new runtime_error(argv[2]);
        errorAndQuit(e);
      }
#endif
    default:
      //try to parse all the documents specified

      cout << argc - 1 << " document(s) specified in this session.\n"
           << "Try to locate all the specified files now.\n\n";

      try
      {
        parseDocumentList(argc - 1, (char **)argv);
      }
      catch(runtime_error& e)
      {
        errorAndQuit(&e);
      }
      //if all the files can be found, then start parsing each file.
      try
      {
        parseDocuments(argc - 1, (char **)argv);
      }
      catch (runtime_error& e)
      {
        errorAndQuit(&e);
      }
      break;
  }
  return 0;
}