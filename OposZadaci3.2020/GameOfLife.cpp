#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <iostream>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <CL/cl.h>
#include <string>


#define MAX_DIM 30000

bool** currentStateOfWorld;
int iteration = 0;
int heightOfWorld;
int widthOfWorld;

char* readKernelSource(const char* filename)
{
	char* kernelSource = nullptr;
	long length;
	FILE* f = fopen(filename, "r");
	if (f)
	{
		fseek(f, 0, SEEK_END);
		length = ftell(f);
		fseek(f, 0, SEEK_SET);
		kernelSource = (char*)calloc(length, sizeof(char));
		if (kernelSource)
			fread(kernelSource, 1, length, f);
		fclose(f);
	}
	return kernelSource;
}

//x and y are coordinates
static void readImage(const char* filename, int x, int y) {
	int width = 0;
	int height = 0;
	FILE* fp = fopen(filename, "rb"); //b-binary mode
	if (!fscanf(fp, "P5\n%d %d\n255\n", &width, &height)) {
		throw "error";
	}
	unsigned char* image = new unsigned char[(size_t)width * height];
	fread(image, sizeof(unsigned char), (size_t)width * (size_t)height, fp);
	fclose(fp);

	int xMin = (x + width < widthOfWorld) ? x + width : widthOfWorld;
	int yMin = (y + height < heightOfWorld) ? y + height: heightOfWorld;
	for (int i = y; i < yMin; i++) {
		for (int j = x; j < xMin; j++) {
			if (image[(i - y) * width + (j - x)] > 127)
				currentStateOfWorld[i][j] = true;
			else 
				currentStateOfWorld[i][j] = false;
		}
	}
}




static void writeImage(const char* filename, int xMin, int xMax, int yMin, int yMax)
{
	FILE* fp = fopen(filename, "wb"); // b-binary mode
	int width = (xMax > widthOfWorld) ? widthOfWorld - xMin : xMax - xMin;
	int height = (yMax > heightOfWorld) ? heightOfWorld - yMin : yMax - yMin;
	fprintf(fp, "P5\n%d %d\n255\n", width, height);
	int index = 0;
	unsigned char* array = (unsigned char*)malloc(width * height * sizeof(unsigned char));
	for (int i = yMin; i < yMin + height; i++) {
		for (int j = xMin; j < xMin + width; j++) {
			if (currentStateOfWorld[i][j] == true)
				array[index++] = 255;
			else
				array[index++] = 0;
		}
	}

	fwrite(array, sizeof(unsigned char), (size_t)width * (size_t)height, fp);
	fclose(fp);
}


int main() {

	cl_platform_id cpPlatform;        // OpenCL platform
	cl_device_id device_id;           // device ID
	cl_context context;               // context
	cl_command_queue queue;           // command queue
	cl_program program;               // program
	//dodaj druge kernele
	cl_kernel kernel;                 // kernel

	size_t globalSize[2], localSize[2];
	cl_int err;

	// Number of work items in each local work group
	localSize[0] = localSize[1] = 8;

	// Number of total work items - localSize must be devisor
/*	globalSize[0] = width;
	globalSize[1] = width;*/

	// Bind to platform
	err = clGetPlatformIDs(1, &cpPlatform, NULL);

	// Get ID for the device
	err = clGetDeviceIDs(cpPlatform, CL_DEVICE_TYPE_GPU, 1, &device_id, NULL);

	// Create a context  
	context = clCreateContext(0, 1, &device_id, NULL, NULL, &err);

	// Create a command queue 
	queue = clCreateCommandQueue(context, device_id, 0, &err);

//	char* kernelSource = readKernelSource("GameOfLife.cl");

	// Create the compute program from the source buffer
//	program = clCreateProgramWithSource(context, 1, (const char**)&kernelSource, NULL, &err);

	// Build the program executable 
	err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);

	if (err)
	{
		// Determine the size of the log
		size_t log_size;
		clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);

		// Allocate memory for the log
		char* log = (char*)malloc(log_size);

		// Get the log
		clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);

		// Print the log
		printf("%s\n", log);

		free(log);
	}

	//!!!!! dodaj kad napises kernele
	// Create the compute kernel in the program we wish to run
//	kernel = clCreateKernel(program, "vecAdd", &err);

	//size_t kernelWorkGroupSize = 0;
	//clGetKernelWorkGroupInfo(kernel, device_id, CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &kernelWorkGroupSize, nullptr);

	//calculate maximum size of two conway states able to fit in device memory 

	cl_ulong ulong;
	clGetDeviceInfo(device_id, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(ulong), &ulong, NULL);
	//initialize height and width of world
	widthOfWorld = heightOfWorld = (int)sqrt(ulong / 2);

	//initialize world on host, all cells are dead
	currentStateOfWorld=new bool* [heightOfWorld];

	for (int i = 0; i < widthOfWorld; i++) {
		currentStateOfWorld[i] = new bool[widthOfWorld];

		for (int j = 0; j < heightOfWorld; j++)
			currentStateOfWorld[i][j] = false;
	}
	//now set the initial state
	// user loads the image
	char* filename;
	int x, y;
	std::cout << "Path to the image which contains initial state:" << std::endl;
	std::cin.getline(filename, 256);
	std::cout << "Coordinates in the world:" << std::endl;
	std::cin >> x >> y;
	readImage(filename, x,y);

	writeImage("image2.pgm", 0, 0, 30, 30);


	
}