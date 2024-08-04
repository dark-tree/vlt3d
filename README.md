## VLT3D

### Setup
 - First make sure you have all development packages for vulkan installed, it can be done with those commands:
   - On Ubuntu: `sudo apt install vulkan-tools libvulkan-dev vulkan-validationlayers-dev spirv-tools`
   - On Fedora: `sudo dnf install vulkan-tools vulkan-loader-devel mesa-vulkan-devel vulkan-validation-layers-devel`
   - On Arch Linux: `sudo pacman -S vulkan-devel`

 - You will also need the LUNARG Vulkan SDK, try running `sudo apt install vulkan-sdk` if that failes you will need to add the LUNARG PPA,
for more info see https://vulkan.lunarg.com/sdk/home#linux (see the "Ubuntu Packages" section under Linux)

Due to extremely long link times with GCC's `ld` it is recommend 
to use the [mold linker](https://github.com/rui314/mold):

```bash
sudo apt install mold
```

### Running
To run VLT3D execute the `main` cmake target, 
like so:

```bash
# Build
mkdir build
cmake . -B build -G Ninja
cmake --build build/ --target main -j $(($(nproc --all) + 1))

# Execute
cd build && ./main
```

You can also build and run the `test` target,  
that runs some simple unit tests of the internal utilities and systems

### Code Style
VLT3D uses a quite unique code style that focuses on clarity and simplicity. As such, prefer short
and unique names in place of long and overly descriptive ones, and try to limit the number of words:
 - Favor `data()` in place of `getData()`, `dataPtr()`, or `ptr()`.
 - Favor `mustReplace()` in place of `isImageOutdated()`, `mustReplaceImage()`, or `attachmentReplacementNeeded()`.

Avoid unnecessary or not well-established abbreviations:
 - Favor `output` in place of `dataOut`.
 - Favor `window` in place of `hwnd` or `windowHandle`.

Prefix external library state with the library namespace:
 - Favor `vk_instance` in place of `instance` or `vulkanInstance`.
 - Favor `al_source` in place of `audioSource` or `source`.

Avoid using namespaces; prefer static methods or possibly global functions for stateless procedures.
Classes should use **PascalCase**, methods **camelCase** and fields **snake_case**, following
the previously mentioned guidelines and exceptions. Classes should indent the `private/public` keywords like so:

```C++
class SimulationState {

	private:

		// Field with library prefix 'glfw',
		// never use 'const' before fields; you can use
		// the macro hint 'READONLY' in its place or c++ keyword 'mutable'
		READONLY int glfw_window;

	public:

		void apply() {
			// Some operation
		}

		void reject() {
			// Note the short method names
		}

};
```

Try to...
 - Write code in an extensible and debuggable manner (prefer writing dev tools in place of features; in the long run, that will be more efficient).
 - Use functional design where applicable (builders, chains, etc.).
 - Limit the use of clever C++ structures where resources are managed.
 - Avoid using destructors, move constructors, and copy constructors; prefer explicit methods like `.close()`, especially when resources are managed.
 - Provide parameter-less constructors when applicable for compatibility with standard collections.

And above all, prefer readability over dogmas.  
When using CLion refer to the `clion-config.xml` style file.
