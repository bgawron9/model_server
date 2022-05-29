## How to get Tensorflow plugin on Mac with arm64 (Apple Silicon)
***
### Step 0:
It is recommended to create a virtual environment first.
***
### Step 1:
To continue it is requierd to download Conda:
>https://github.com/conda-forge/miniforge/releases/latest/download/Miniforge3-MacOSX-arm64.sh

and install it
>chmod +x ~/Downloads/Miniforge3-MacOSX-arm64.sh
sh ~/Downloads/Miniforge3-MacOSX-arm64.sh
source ~/miniforge3/bin/activate

After then let's get the Tensorflow dependencies:
>conda install -c apple tensorflow-deps

You can force conda to download specific version of tensorflow-deps (they are following base TensorFlow versions) by adding
>==2.5.0

or
>==2.6.0

It is also recommended to uninstall existing tensorflow-macos

>python -m pip uninstall tensorflow-macos
conda install -c apple tensorflow-deps --force-reinstall
***
### Step 2:
Now you can install mac version of Tensorflow plugin
>python -m pip install tensorflow-macos

If you get an error "not a supported wheel on this platform" try to use this code before installing Tensorflow-macos

>SYSTEM_VERSION_COMPAT=0 python -m pip install tensorflow-macos
***
Remember to use "conda" environment in your python app.