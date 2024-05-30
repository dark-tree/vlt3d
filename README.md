## VLT3D

#### Setup
 - First make sure you have all development packages for vulkan installed, it can be done with those commands:
   - On Ubuntu: `sudo apt install vulkan-tools libvulkan-dev vulkan-validationlayers-dev spirv-tools`
   - On Fedora: `sudo dnf install vulkan-tools vulkan-loader-devel mesa-vulkan-devel vulkan-validation-layers-devel`
   - On Arch Linux: `sudo pacman -S vulkan-devel`

 - You will also need the LUNARG Vulkan SDK, try running `sudo apt install vulkan-sdk` if that failes you will need to add the LUNARG PPA,
for more info see https://vulkan.lunarg.com/sdk/home#linux (see the "Ubuntu Packages" section under Linux)

 - Now run `./compile --sync` to download the required dependencies, this process can take some time.

#### Running
To run VLT3D execute the `main` cmake target,
or use the compile helper script:

* Use `./compile --run` to run the main target
* Use `./compile --help` for additional info
