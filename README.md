# LeetBot-discord-win
A multifunctional Discord bot made on Windows to help users improve at Leetcode from Discord and to track their Leetcode data.

Instructions and requirements may be subject to change.

## Build Requirements
- Visual Studio 2022 (C++20)
- [D++](https://github.com/brainboxdotcc/DPP) library
- MySQL server

## Setup Instructions
1. Clone the repo via bash -> (git clone https://github.com/Dinguart/LeetBot-discord-win)
2. Install required dependencies -> (D++ library as mentioned above, as well as MySQL connector (done in MySQL installation https://dev.mysql.com/downloads/installer/ (I recommend watching a tutorial for this))

### MySQL C++ Connector
Follow this [tutorial](https://www.youtube.com/watch?v=a_W4zt5sR1M)
2a. Download MySQL from community downloads
2b. Click on Connector/C++
2c. Download Windows (x86, 64-bit), ZIP Archive
2d. Extract the ZIP file into a libraries folder (or any folder you choose)
2e. Change config to release and go to the VS 2022 properties page (x64 platform)
2f. C/C++ -> General | Add include folder of MySQL into Additional Include Directories
2g. C/C++ -> Preprocessor | Add STATIC_CONCPP; macro to Preprocessor definitions
2h. C/C++ -> Code Generation | Make sure Runtime Library is set to MD
2i. Linker -> General | In Additional Library Directories, go to the mysql library folder -> lib64 -> vs14.
2j. Linker -> Input | In Additional Dependencies add the .lib file. Go to mysql library folder -> lib64 -> mysqlcppconn-static.lib


3. Open project in VS 2022 and configure credentials, copy example env files and use your credentials, then read those files accordingly with the provided readAuthFile function in the ReadAuthFile file.
4. Build the project.


Copyright 2025 Henri Brizuela Ocana

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
