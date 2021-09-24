# LayoutWatcher

Library for watching current keyboard language layout. She can notify about changes via [callbacks](https://github.com/wqking/eventpp).

## C++ standard requirements

- To Use the library: **C++11**
- To develop the library: **C++20**

## Supported platforms

- KDE Session (X11/Wayland)
- Any other X11 Sessions

## Usage

### System-wide installation

1. add include

```cpp
#include <LayoutWatcher/LayoutWatcher.h>
```

2. link library `-lLayoutWatcher`

### CMake subdirectory

1. add subdirectory

```cmake
add_subdirectory(LayoutWatcher)
```

2. link static library

```cmake
target_link_libraries(${PROJECT_NAME} PRIVATE LayoutWatcher)
```

3. add include

```cpp
#include "LayoutWatcher/LayoutWatcher.h"
```



### Create watcher

```cpp
LayoutWatcher watcher;

std::cout << "Current layout: " << watcher.getActiveLayout() << std::endl;
std::cout << "Available layouts:" << std::endl;
for( auto &&layout : watcher.getLayoutsList() )
    std::cout << layout.shortName << ": " << layout.longName << std::endl;
```

Also, you can subscribe on changes:

```cpp
watcher.onLayoutChanged.append([]( const std::string &layout ){
    std::cout << "Layout is changed: " << layout << std::endl;
});
watcher.onLayoutListChanged.append([]( const std::vector<LayoutWatcher::LayoutNames> &layouts ){
    std::cout << "Available layouts is changed:" << std::endl;
	for( auto &&layout : layouts )
    	std::cout << layout.shortName << ": " << layout.longName << std::endl;
});
```

