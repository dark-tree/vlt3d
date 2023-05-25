## GLPH
GLPH is a Vulkan based rendering engine

#### Setup
**1**. First make sure you have all development pacakges for vulkan installed, it can be done with those commands:
 - On Ubuntu: `sudo apt install vulkan-tools libvulkan-dev vulkan-validationlayers-dev spirv-tools`
 - On Fedora: `sudo dnf install vulkan-tools vulkan-loader-devel mesa-vulkan-devel vulkan-validation-layers-devel`
 - On Arch Linux: `sudo pacman -S vulkan-devel`

**2**. You will need the LUNARG Vulkan SDK, try running `sudo apt install vulkan-sdk` if that failes you will need to add the LUNARG PPA,
for more info see https://vulkan.lunarg.com/sdk/home#linux (see the "Ubuntu Packages" section under Linux)

**3**. Now run `./compile --sync` to download the required dependencies, this process can take some time.

#### Running
* Use `./compile --run main` to run the "main" example
* Use `./compile --list` to list avaible example programs
* Use `./compile --help` for aditional info
