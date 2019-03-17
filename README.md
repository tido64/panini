# panini 🥪

panini 🥪 is a dumb-as-bread [INI file](https://en.wikipedia.org/wiki/INI_file)
reader with a SAX-like interface.

## Example

`config.ini`:

```ini
[core]
ResolutionWidth = 1280
ResolutionHeight = 720
MSAA = 4
AllowHiDPI = true
; This is a comment.
```

`main.cpp`:

```c++
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <string>
#include <string_view>

#include <panini/panini.hpp>

using namespace std::literals::string_view_literals;

auto main(int argc, char* argv[]) -> int
{
  auto f = fopen("config.ini", "rb");
  fseek(f, 0, SEEK_END);

  const auto size = ftell(f);
  auto config = std::make_unique<char[]>(size + 1);

  fseek(f, 0, SEEK_SET);
  fread(config.get(), sizeof(char), size, f);
  config[size] = '\0';
  fclose(f);

  // The callback is called for every key/value pair read.
  panini::parse(
    config.get(),
    [](panini::State state,
       std::string_view section,
       std::string_view key,
       std::string_view value) {

      if (state == panini::State::Error) {
        printf("Error parsing config.ini:%s: %s", section.data(), key.data());
        return;
      }

      if (section != "core"sv) {
        return;
      }

      if (key == "ResolutionWidth"sv) {
        printf("Width: %s px", value.data());
      } else if (key == "ResolutionHeight"sv) {
        printf("Height: %s px", value.data());
      } else if (key == "MSAA"sv) {
        printf("MSAA: %i", std::clamp(atoi(value.data()), 0, 16));
      } else if (key == "AllowHiDPI"sv) {
        printf("Allow HiDPI: %s", value.data());
      }
    });

  return 0;
}
```

## Features

- Leading and trailing whitespaces, and blank lines are ignored
- Sections
- Case sensitive
- Comment lines start with semicolons (`;`)

### Not Supported

- Nested sections
- Multi-line support
- Writing

## Requirements

- A C++17 compiler

## License

Copyright &copy; 2019 Tommy Nguyen

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
