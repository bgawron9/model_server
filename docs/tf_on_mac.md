## How to get Tensorflow on Mac with arm64 chip (Apple Silicon) using Conda
***
requires MacOS 12 or newer
***
### Step 0:
delete currently installed Tensorflow
>python -m pip uninstall tensorflow-macos

>conda install -c apple tensorflow-deps --force-reinstall
***
### Step 1:
download Conda:
>wget https://github.com/conda-forge/miniforge/releases/latest/download/Miniforge3-MacOSX-arm64.sh

and install it
>chmod +x ~/Downloads/Miniforge3-MacOSX-arm64.sh

>sh ~/Downloads/Miniforge3-MacOSX-arm64.sh

>source ~/miniforge3/bin/activate

install the Tensorflow dependencies:
>conda install -c apple tensorflow-deps

is is possible to force conda to download specific version of tensorflow-deps (they are following base TensorFlow versions) by adding
>==2.5.0

or
>==2.6.0
***
### Step 2:
install mac version of Tensorflow:
>python -m pip install tensorflow-macos

if you get an error "not a supported wheel on this platform" use this code before installing Tensorflow-macos:

>SYSTEM_VERSION_COMPAT=0 python -m pip install tensorflow-macos
***
use Conda (miniforge) as python interpreter  
in PyCharm click:
>PyCharm -> Preferences... -> Project -> Python Interpreter

choose Python version which uses Conda (miniforge) environment  
or create new project:
>File -> New Project... -> Python Interpreter -> New environment using

and choose Conda

