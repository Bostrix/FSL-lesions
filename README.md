# Lesion filling - Compilation and Usage Guide

## Introduction
Welcome to Lesion filling, a tool designed to improve volume measurements in neuroimaging by reducing intensity contrast within known lesion areas.This guide will walk you through the process of compiling the Lesion Filling tool and utilizing its functionalities for improving volume measurements in neuroimaging research, particularly in cases involving small lesions like those seen in Multiple Sclerosis patients.

## Clone the Repository
Begin by cloning the project repository from GitHub onto your local machine. You can do this by running the following command in your terminal or command prompt:
```bash
https://github.com/Bostrix/FSL-lesions.git
```
This command will create a local copy of the project in a directory named "FSL-lesions".

## Navigate to Project Directory
Change your current directory to the newly cloned project directory using the following command:
```bash
cd FSL-lesions
```
## Compilation
To compile the lesion filling tool, follow these steps:
- Install Required Libraries:
    Ensure that all necessary libraries, such as `Blas`,`Lapack`,`zlib`, are installed on your system. Use the following command to install those:
```bash
sudo apt-get install libblas-dev libblas3
sudo apt-get install liblapack-dev liblapack3
sudo apt-get install zlib1g zlib1g-dev
```

- Modfication of the Makefile:
  After installing the necessary tools, modify the makefile to include additional LDFLAGS for the required libraries. For example, if using zlib, add the following line to the makefile:
```bash
ZNZLIB_LDFLAGS = -L/path/to/your/znzlib/directory -lfsl-znz
```
Replace `Path to your znzlib directory` with actual path to your directory.Then, make sure that `$(ZNZLIB_LDFLAGS)` is added in the compile step of the makefile.

- Compile Source Code:
    Execute the appropriate compile command to build the lesion filling. For example:
```bash
make
```
This command will compile the source code and generate the executable file for lesion filling.

- Resolving Shared Library Errors:
When running an executable on Linux, you may encounter errors related to missing shared libraries.This typically manifests as messages like:
```bash
./lesion_filling: error while loading shared libraries: libexample.so: cannot open shared object file:No such file or directory
```
To resolve these errors,Pay attention to the names of the missing libraries mentioned in the error message.Locate the missing libraries on your system. If they are not present, you may need to install the corresponding packages.If the libraries are installed in custom directories, you need to specify those directories using the `LD_LIBRARY_PATH` environment variable. For example:
```bash
export LD_LIBRARY_PATH=/path/to/custom/libraries:$LD_LIBRARY_PATH
```
Replace `/path/to/custom/libraries` with the actual path to the directory containing the missing libraries.Once the `LD_LIBRARY_PATH` is set, attempt to run the executable again.If you encounter additional missing library errors, repeat steps until all dependencies are resolved.


- Resolving "The environment variable FSLOUTPUTTYPE is not defined" errors:
If you encounter an error related to the FSLOUTPUTTYPE environment variable not being set.Setting it to `NIFTI_GZ` is a correct solution, as it defines the output format for FSL tools to NIFTI compressed with gzip.Here's how you can resolve:
```bash
export FSLOUTPUTTYPE=NIFTI_GZ
```
By running this command, you've set the `FSLOUTPUTTYPE` environment variable to `NIFTI_GZ`,which should resolve the error you encountered.

## Usage
To use the Lesion Filling tool, follow the steps outlined below:

- Run Lesion Filling Command:
Execute the `lesion_filling` command with the necessary arguments and options. Here is the basic syntax:
```bash
./lesion_filling -i <intensity_image> -l <lesion_mask_image> -o <output_filled_image> [options]
```
Replace `<intensity_image>`, `<lesion_mask_image>`, and `<output_filled_image>` with the respective filenames or paths of your input intensity image, lesion mask image, and output filled image.

- Specify Compulsory Arguments:
You must set one or more of the following compulsory arguments:

  `-i, --in`: Input image filename (e.g., T1w image)

  `-o, --out`: Output filename (lesion filled image)

  `-l, --lesionmask`: Filename of lesion mask image

- Provide Optional Arguments:
Optionally, you can include additional arguments to customize the tool's behavior:

  `-w, --wmmask`: Specifies the filename of the white matter mask image.
  
  `-v, --verbose`: Enables diagnostic messages.
  
  `-c, --components`: Saves all lesion components as volumes.
  
  `-h, --help`: Displays the usage message.
  
## Conclusion
You've successfully utilized the Lesion Filling tool to enhance volume measurements in neuroimaging research. After execution, lesions are filled with intensities similar to surrounding white matter tissue, improving segmentation accuracy. For further assistance, consult the documentation or project maintainers. 
