#include <CL/cl.hpp>

#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
using namespace std;

//#include "utils.h"
//#include "bmp-utils.h"

float* readImgtxt(char *filename){
	float *img;
	int channels = 3;
	int size = 32;
	img = new float [size*size*channels];

	FILE *fp = fopen(filename, "r");
	
	if (fp == NULL){
		std::cout<<"test image open failed!";
		exit(-1);
	}
	for(int channels=0; channels<3; channels++){
		for(int i=0; i<size; i++){
			for(int j=0; j<size; j++)
				fscanf(fp, "%f\n", img + i*size + j + channels*(size*size));
		}
	}
	return img;
}


class Conv2D
{
	public:
		int numChannels;
		int kernelWidth;
		int kernelHeight;
		int kernelDepth;
		string *layerName;
		string *weightFilePath;
		FILE *filePtr;
		double ****weights;
		double *weights2;
		double *biases;

		Conv2D(string weightFilePath)
		{
			this->weightFilePath = new string(weightFilePath);
			this->layerName = NULL;

			bool status = readFile();
			if(status == 0)
				throw std::invalid_argument("read failed, please make sure you are provding correct file path...");
			else
				cout<<"Pointer to file "<<*(this->weightFilePath)<<" opened successfully..."<<endl;
			
			parseLayerName();   // Get layer name
			parseKernelDimensions();    // Get kernel dimensions
			allocateSpace();    // Allocate space to hold weights
			parseWeights();     // Parse weights value into array
			parseBiases();      // Parse biases value into array
		}
		~Conv2D()
		{
			delete this->layerName;
			delete this->weightFilePath;
			deallocateSpace();
		}

		void layerSummary()
		{
			cout<<"Layer Name : "<<*(this->layerName)<<endl;
			cout<<"Kernel Width : "<<this->kernelWidth<<endl;
			cout<<"Kernel Height : "<<this->kernelHeight<<endl;
			cout<<"Kernel Depth : "<<this->kernelDepth<<endl;
			cout<<"Channels : "<<this->numChannels<<endl;
		}

	protected:
		void allocateSpace()
		{	
			/* Allocate Space for weights */
			// number of channels x width x height x depth
			this->weights = new double***[this->numChannels];
			for(int channel=0; channel<(this->numChannels); channel++)
			{
				this->weights[channel] = new double**[this->kernelWidth];
				for(int width=0; width<(this->kernelWidth); width++)
				{
					this->weights[channel][width] = new double*[this->kernelHeight];
					for(int height=0; height<(this->kernelHeight); height++)
					{
						this->weights[channel][width][height] = new double[this->kernelDepth];
					}
				}
			}
			/* Allocate space for biases */
			this->weights2 = new double[(this->numChannels)*(this->kernelWidth)*(this->kernelHeight)*(this->kernelDepth)];
			this->biases = new double[this->numChannels];
		}

		void deallocateSpace()
		{
			/* Deallocate space of weights */
			for(int channel=0; channel<(this->numChannels); channel++)
			{
				for(int width=0; width<(this->kernelWidth); width++)
				{
					for(int height=0; height<(this->kernelHeight); height++)
					{
						delete[] this->weights[channel][width][height];
					}
					delete[] this->weights[channel][width];
				}
				delete[] this->weights[channel];
			}
			delete[] this->weights;
			/* Deallocates space of biases */
			delete[] this->weights2;
			delete[] this->biases;
		}

		bool readFile()
		{	
			this->filePtr = fopen(this->weightFilePath->c_str(), "r");
			if(this->filePtr == 0)
				return false;
			else
				return true;
		}

		void parseLayerName()
		{
			char tmp[100];
			fscanf(this->filePtr, "%s\n", tmp);
			this->layerName = new string(tmp);
		}

		void parseKernelDimensions()
		{
			fscanf(this->filePtr, "%d %d %d %d\n", &this->kernelWidth, &this->kernelHeight, &this->kernelDepth, &this->numChannels);
		}

		void parseWeights()
		{
			int nc = (this->numChannels);
			int ww = (this->kernelWidth);
			int hh = (this->kernelHeight);
			int dd = (this->kernelDepth);
			//cout<<"Weights Start \n";
			for(int channel=0; channel<nc; channel++)
			{
				for(int width=0; width<ww; width++)
				{
					for(int height=0; height<hh; height++)
					{
						for(int depth=0; depth<dd; depth++){
							fscanf(this->filePtr, "%lf ", &weights[channel][width][height][depth]);
							//weights2[channel*(ww*hh*dd) + width*(hh*dd) + height*(hh) + depth] = weights[channel][width][height][depth];
						}
					}
				}
			}
			//cout<<"Weights End \n";
			fscanf(this->filePtr, "\n");
		}

		void parseBiases()
		{
			for(int channel=0; channel<(this->numChannels); channel++)
				fscanf(this->filePtr,"%ld ", &biases[channel]);
		}
};

class Dense
{
	public:
		int inChannels;
		int outChannels;
		string layerName;
		string *weightFilePath;
		string *weightFilePathB;
		FILE *filePtr;
		FILE *filePtrB;
		double *weights;
		double *biases;

		Dense(string weightFilePath, int inc, int outc)
		{
			std::cout<<weightFilePath+"Dense.txt\n";
			std::cout<<weightFilePath+"Dense_biases.txt\n";
			
			this->weightFilePath = new string(weightFilePath+"Dense.txt");
			this->weightFilePathB = new string(weightFilePath+"Dense_biases.txt");

			this->layerName = "Dense";

			bool status = readFile();
			if(status == 0)
				throw std::invalid_argument("read failed, please make sure you are provding correct file path...");
			else
				cout<<"\nPointer to file "<<*(this->weightFilePath)<<" opened successfully..."<<endl;
			
			//parseLayerName();   // Get layer name
			this->inChannels = inc;
			this->outChannels = outc;
			this->layerName = "Dense";
			//parseKernelDimensions();    // Get kernel dimensions
			

			allocateSpace();    // Allocate space to hold weights
			
			parseWeights();     // Parse weights value into array
			
			parseBiases();      // Parse biases value into array
			
		}
		~Dense()
		{
			//delete this->layerName;
			delete this->weightFilePath;
			deallocateSpace();
		}

		void layerSummary()
		{
			cout<<"Layer Name : "<<(this->layerName)<<endl;
			cout<<"in channel : "<<this->inChannels<<endl;
			cout<<"out channel : "<<this->outChannels<<endl;
		}

	protected:
		void allocateSpace()
		{	
			/* Allocate Space for weights */
			// number of channels x width x height x depth
			//std::cout<<(this->inChannels)<<" "<<(this->outChannels)<<"\n";
			this->weights = new double[(this->inChannels)*(this->outChannels)];
			
			/* Allocate space for biases */
			this->biases = new double[this->outChannels];
		}

		void deallocateSpace()
		{
			/* Deallocate space of weights */
			delete[] this->weights;
			/* Deallocates space of biases */
			delete[] this->biases;
		}

		bool readFile()
		{	
			this->filePtr = fopen(this->weightFilePath->c_str(), "r");
			this->filePtrB = fopen(this->weightFilePathB->c_str(), "r");
			if(this->filePtr == 0 && this->filePtrB == 0)
				return false;
			else
				return true;
		}

		void parseLayerName()
		{
			char tmp[100];
			fscanf(this->filePtr, "%s\n", tmp);
			//this->layerName = new string(tmp);
		}

		void parseKernelDimensions()
		{
			fscanf(this->filePtr, "%d %d \n", &this->inChannels, &this->outChannels);
		}

		void parseWeights()
		{
			int inp = (this->inChannels);
			int out = (this->outChannels);
			std::cout<<inp<<" "<<out<<"\n";
			float s = 0;
				try{
					for(int i=0; i<inp; i++)
					{
						for(int o=0; o<out; o++)
						{	
							fscanf(this->filePtr, "%1d ", &weights[(i*inp)+o]);
						}
					}
					
				}
				catch(...){
					std::cout<<"dafad";
				}
			//	fscanf(this->filePtr, "\n");
		}

		void parseBiases()
		{
			for(int channel=0; channel<(this->outChannels); channel++)
			{
				//cout<<channel<<"\n";
				fscanf(this->filePtrB,"%lf ", &biases[channel]);
			}
		}
};
int main() 
{
   	std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);
    std::vector<cl::Device> devices;
    platforms[0].getDevices(CL_DEVICE_TYPE_GPU, &devices);
    cl::Context context(devices);
    cl::CommandQueue queue = cl::CommandQueue(context, devices[0]);
	
	float *output_buffer = new float [5000000];
	float *input_buffer = new float [5000000];
    for (int i =0;i<5000000;i++){
    	output_buffer[i] = 0;
    }
	float *hInputImage;
	float *hOutputImage;
	int imageRows = 32;
	int imageCols = 32;
	
	char* inputImagePath = "snail.txt";
	/*
			0 -- Conv
			1 -- MaxPool
			2 -- Dense
	*/
	int arr[] = 	  {0,0 ,1 ,0 ,0  ,1  ,0  ,0  ,0  ,0  ,1  ,0  ,0  ,0  ,0  ,1  ,0  ,0  ,0  ,0  ,1  ,  2,   2, 2};
	int chanelarr[] = {3,64,64,64,128,128,128,256,256,256,256,256,512,512,512,512,512,512,512,512,512, 512,4096,4096,10};
	//string fnarr[] = {"1Conv2d","2Conv2d","4Conv2d","5Conv2d","7Conv2d","8Conv2d","9Conv2d","","23Dense"};
	hInputImage = readImgtxt(inputImagePath);
	for (int p = 0;p<(3*imageRows*imageCols);p++){ 
					input_buffer[p] = hInputImage[p]; 
				}
	//input_buffer = hInputImage;
	int LayerNum = 25;
	int Curr_channel = 3;
	for(int i=0;i<LayerNum; i++){
		std::cout<<std::to_string(i)<<"\n";
			
		if(arr[i]==0){
			///// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-   Convolution Layer -=--=-=-=-=-=-=-=-=-=-=-=-=--=--=-==-=- /////
			//string fn = "Conv2D"+ std::to_string(i) + ".txt"; 
			string fn = "Weights/"+std::to_string(i+1)+"Conv2d.txt";
			string weightFilePath = fn;
			Conv2D layer1(weightFilePath);
			layer1.layerSummary();
	
				int in_channels, out_channels, kernel_size, imgRows, imgCols;
				in_channels = layer1.kernelDepth;
				out_channels = layer1.numChannels;
				kernel_size = 3;
				imgRows = imageRows;
				imgCols = imageCols;	
				hOutputImage = new float [imageRows*imageCols*out_channels];
			
			try{	
				cl::Buffer inputBuffer = cl::Buffer(context, CL_MEM_READ_ONLY, in_channels*imgRows*imgCols*sizeof(float));
				cl::Buffer filterBuffer = cl::Buffer(context, CL_MEM_READ_ONLY, in_channels*out_channels*kernel_size*kernel_size*sizeof(float));
				cl::Buffer biasBuffer = cl::Buffer(context, CL_MEM_READ_ONLY, out_channels*sizeof(float));
				cl::Buffer outputBuffer = cl::Buffer(context, CL_MEM_WRITE_ONLY, out_channels*imgRows*imgCols*sizeof(float));
				cl::Buffer in_channelsBuffer = cl::Buffer(context, CL_MEM_READ_ONLY, sizeof(int));
				cl::Buffer out_channelsBuffer = cl::Buffer(context, CL_MEM_READ_ONLY, sizeof(int));
				cl::Buffer kernelSizeBuffer = cl::Buffer(context, CL_MEM_READ_ONLY, sizeof(int));
				cl::Buffer imgRowsBuffer = cl::Buffer(context, CL_MEM_READ_ONLY, sizeof(int));
				cl::Buffer imgColsBuffer = cl::Buffer(context, CL_MEM_READ_ONLY, sizeof(int));
				
				queue.enqueueWriteBuffer(inputBuffer, CL_TRUE, 0, in_channels*imgRows*imgCols*sizeof(float), input_buffer);
				queue.enqueueWriteBuffer(filterBuffer, CL_TRUE, 0, in_channels*out_channels*kernel_size*kernel_size*sizeof(float), layer1.weights);
				queue.enqueueWriteBuffer(biasBuffer, CL_TRUE, 0, out_channels*sizeof(float), layer1.biases);
				queue.enqueueWriteBuffer(outputBuffer, CL_TRUE, 0, out_channels*imgRows*imgCols*sizeof(float), output_buffer);
				queue.enqueueWriteBuffer(in_channelsBuffer, CL_TRUE, 0, sizeof(int), &in_channels);
				queue.enqueueWriteBuffer(out_channelsBuffer, CL_TRUE, 0, sizeof(int), &out_channels);
				queue.enqueueWriteBuffer(kernelSizeBuffer, CL_TRUE, 0, sizeof(int), &kernel_size);
				queue.enqueueWriteBuffer(imgRowsBuffer, CL_TRUE, 0, sizeof(int), &imgRows);
				queue.enqueueWriteBuffer(imgColsBuffer, CL_TRUE, 0, sizeof(int), &imgCols);

				std::ifstream sourceFile("Kernels/Conv.cl");
				std::string sourceCode(
				std::istreambuf_iterator<char>(sourceFile),(std::istreambuf_iterator<char>()));
				cl::Program::Sources source(1, std::make_pair(sourceCode.c_str(),sourceCode.length() + 1));

				cl::Program program = cl::Program(context, source);

				program.build(devices);
				
				cl::Kernel kernel(program, "convolution");

				kernel.setArg(0, out_channelsBuffer);
				kernel.setArg(1, in_channelsBuffer);
				kernel.setArg(2, kernelSizeBuffer);
				kernel.setArg(3, inputBuffer);
				kernel.setArg(4, filterBuffer);
				kernel.setArg(5, biasBuffer);
				kernel.setArg(6, outputBuffer);
				kernel.setArg(7, imgRowsBuffer);
				kernel.setArg(8, imgColsBuffer);

				cl::NDRange global(imgCols, imgRows);
				cl::NDRange local(2, 2);
				cl::Event event;
					
				queue.enqueueNDRangeKernel(kernel, cl::NullRange, global, local,NULL,&event);
				queue.finish();
				// Read data back
				queue.enqueueReadBuffer(outputBuffer, CL_TRUE, 0, out_channels*imgRows*imgCols*sizeof(float), output_buffer);
				cl_ulong time_start;
				cl_ulong time_end;
				Curr_channel = out_channels;
				event.wait();
				double total_time;
				event.getProfilingInfo(CL_PROFILING_COMMAND_END, &time_end); 
				event.getProfilingInfo(CL_PROFILING_COMMAND_START, &time_start);
				total_time = time_end - time_start;

				/* Results */
				std::cout << "Execution time in milliseconds for convolution layer " << total_time*1.0e-6f << std::endl;   
				
			}
			/*catch(cl::Error error)
			{
				std::cout << error.what() << "(" << error.err() << ")" <<std::endl;
			}*/
			catch(...){
				cout<<"Error";
			}

			// --------------------------------------------------- Layer 1 End
			cout<<"Complete \n";
			for (int p = 0;p<(out_channels*imgRows*imgCols);p++){ 
					input_buffer[p] = output_buffer[p]; 
				}
			
			
		}
		if(arr[i]==1){
			/* ------------------------------------ MaxPool 2D Starts ------------------------------------ */

			int channels, pool_size, outImgRows, outImgCols;
			channels = Curr_channel;
			//imgRows = layer[j][3];
			//imgCols = layer[j][3];
			pool_size = 2;
			int imgRows = imageRows;
			int imgCols = imageCols;
			outImgRows = (int)(imageRows/pool_size);
			outImgCols = (int)(imageCols/pool_size);
			for (int i =0;i<channels*outImgCols*outImgCols;i++)
				output_buffer[i] = 0;
			try
			{
				cl::Buffer inputBuffer = cl::Buffer(context, CL_MEM_READ_ONLY, channels*imgRows*imgCols*sizeof(float));
				cl::Buffer outputBuffer = cl::Buffer(context, CL_MEM_WRITE_ONLY, channels*outImgRows*outImgCols*sizeof(float));
				cl::Buffer channelsBuffer = cl::Buffer(context, CL_MEM_READ_ONLY, sizeof(int));
				cl::Buffer poolSizeBuffer = cl::Buffer(context, CL_MEM_READ_ONLY, sizeof(int));
				cl::Buffer inDimBuffer = cl::Buffer(context, CL_MEM_READ_ONLY, sizeof(int));
				cl::Buffer outDimBuffer = cl::Buffer(context, CL_MEM_READ_ONLY, sizeof(int));

				queue.enqueueWriteBuffer(inputBuffer, CL_TRUE, 0, channels*imgRows*imgCols*sizeof(float), input_buffer);
				queue.enqueueWriteBuffer(outputBuffer, CL_TRUE, 0, channels*outImgRows*outImgCols*sizeof(float), output_buffer);
				queue.enqueueWriteBuffer(channelsBuffer, CL_TRUE, 0, sizeof(int), &channels);
				queue.enqueueWriteBuffer(poolSizeBuffer, CL_TRUE, 0, sizeof(int), &pool_size);
				queue.enqueueWriteBuffer(inDimBuffer, CL_TRUE, 0, sizeof(int), &imgRows);
				queue.enqueueWriteBuffer(outDimBuffer, CL_TRUE, 0, sizeof(int), &outImgRows);

				std::ifstream sourceFile("Kernels/Max_Pool2D.cl");
				std::string sourceCode(
				std::istreambuf_iterator<char>(sourceFile),
				(std::istreambuf_iterator<char>()));
				cl::Program::Sources source(1,
				std::make_pair(sourceCode.c_str(),
				sourceCode.length() + 1));

				cl::Program program = cl::Program(context, source);

				program.build(devices);

				cl::Kernel kernel(program, "max_pool2d");

				kernel.setArg(0, channelsBuffer);
				kernel.setArg(1, inDimBuffer);
				kernel.setArg(2, poolSizeBuffer);
				kernel.setArg(3, outDimBuffer);
				kernel.setArg(4, inputBuffer);
				kernel.setArg(5, outputBuffer);

				cl::NDRange global(outImgRows, outImgCols);
				cl::NDRange local(1, 1);
				cl::Event event;
				queue.enqueueNDRangeKernel(kernel, cl::NullRange, global, local,NULL,&event);
				queue.finish();

				queue.enqueueReadBuffer(outputBuffer, CL_TRUE, 0, channels*outImgRows*outImgCols*sizeof(float), output_buffer);
				cl_ulong time_start;
				cl_ulong time_end;

				event.wait();
				double total_time;
				event.getProfilingInfo(CL_PROFILING_COMMAND_END, &time_end); 
				event.getProfilingInfo(CL_PROFILING_COMMAND_START, &time_start);
				total_time = time_end - time_start;

				/* Results */
				std::cout << "Execution time in milliseconds for maxpool layer " << total_time*1.0e-6f << std::endl;   
				imageRows = outImgRows;
				imageCols = outImgCols;
			}
			catch(...)
			//catch(cl::Error error)
			{
				std::cout << "Error"; 
				//std::cout << error.what() << "(" << error.err() << ")" <<std::endl;
			}
			for (int p = 0;p<(channels*imgRows*imgCols);p++){ 
					input_buffer[p] = output_buffer[p]; 
				}

		}
		else if(arr[i]==2){
			//// =-=-=-=-=-=-=-=-=------=--=-=-=-= Dense -=-=-=-=-=-=-=-=-==--=-=
			string fn = "Weights/"+std::to_string(i+2);
			string weightFilePath = fn ;
			std::cout<<weightFilePath<<" "<<chanelarr[i]<<" "<< chanelarr[i+1];
			Dense layer1(weightFilePath,chanelarr[i], chanelarr[i+1]);
			layer1.layerSummary();
			
			int in_features, out_features;
			in_features = layer1.inChannels;
			out_features = layer1.outChannels;
			try
			{
				cl::Buffer inputBuffer = cl::Buffer(context, CL_MEM_READ_ONLY, in_features*sizeof(float));
				cl::Buffer outputBuffer = cl::Buffer(context, CL_MEM_WRITE_ONLY, out_features*sizeof(float));
				cl::Buffer weightsBuffer = cl::Buffer(context, CL_MEM_READ_ONLY, in_features*out_features*sizeof(float));
				cl::Buffer biasesBuffer = cl::Buffer(context, CL_MEM_READ_ONLY, out_features*sizeof(float));
				cl::Buffer inFeaturesBuffer = cl::Buffer(context, CL_MEM_READ_ONLY, sizeof(int));
				cl::Buffer outFeaturesBuffer = cl::Buffer(context, CL_MEM_READ_ONLY, sizeof(int));
				
				queue.enqueueWriteBuffer(inputBuffer, CL_TRUE, 0, in_features*sizeof(float), input_buffer);
				queue.enqueueWriteBuffer(outputBuffer, CL_TRUE, 0, out_features*sizeof(float), output_buffer);
				queue.enqueueWriteBuffer(weightsBuffer, CL_TRUE, 0, in_features*out_features*sizeof(float),layer1.weights);
				queue.enqueueWriteBuffer(biasesBuffer, CL_TRUE, 0, out_features*sizeof(float), layer1.biases);
				queue.enqueueWriteBuffer(inFeaturesBuffer, CL_TRUE, 0, sizeof(int), &in_features);
				queue.enqueueWriteBuffer(outFeaturesBuffer, CL_TRUE, 0, sizeof(int), &out_features);
				
				std::ifstream sourceFile("Kernels/Dense.cl");
				std::string sourceCode(std::istreambuf_iterator<char>(sourceFile),	(std::istreambuf_iterator<char>()));
				cl::Program::Sources source(1, std::make_pair(sourceCode.c_str(), sourceCode.length() + 1));
				
				cl::Program program = cl::Program(context, source);

				program.build(devices);

				cl::Kernel kernel(program, "dense");

				kernel.setArg(0, inFeaturesBuffer);
				kernel.setArg(1, outFeaturesBuffer);
				kernel.setArg(2, inputBuffer);
				kernel.setArg(3, weightsBuffer);
				kernel.setArg(4, biasesBuffer);
				kernel.setArg(5, outputBuffer);
				//std::cout<<"d22\n";
				cl::NDRange global(out_features, 1);
				cl::NDRange local(1, 1);
				cl::Event event;
				queue.enqueueNDRangeKernel(kernel, cl::NullRange, global, local,NULL,&event);
				queue.finish();
				//std::cout<<"dsfebv\n";
				queue.enqueueReadBuffer(outputBuffer, CL_TRUE, 0, out_features*sizeof(float), output_buffer);
				cl_ulong time_start;
				cl_ulong time_end;
					
				event.wait();
				double total_time;
				event.getProfilingInfo(CL_PROFILING_COMMAND_END, &time_end); 
				event.getProfilingInfo(CL_PROFILING_COMMAND_START, &time_start);
				total_time = time_end - time_start;
				/* Results */
				std::cout << "Execution time in milliseconds for Fully Connected/Dense layer " << total_time*1.0e-6f << std::endl;   
			}
			catch(...)
			{
				std::cout<<"Error";
				//std::cout << error.what() << "(" << error.err() << ")" <<std::endl;
			}
		for (int p = 0;p<(out_features);p++){ 
					input_buffer[p] = output_buffer[p]; 
				}	
		}
		for(int i=0;i<10;i++)
				std::cout << input_buffer[i]<<" ";
		std::cout<<"\n";
		
	}
	

	
return 0;
}

  

    
    
        
      
   

