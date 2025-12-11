// 
// Scriptpack.cpp : Defines the entry point for the console application.
//
#include <sys/types.h>
#include <sys/stat.h>

#include "cbase.h"
#include "packer.h"
#include "crc/crchash.h"
#include "cxxopts.hpp"
#include "logger.h"

bool g_Verbose; //--verbose
bool g_Release; //--release
bool g_ErrFile;
bool g_FailOnErr; //--fail

int main(int argc, char** argv)
{
	//Program description
	cxxopts::Options options("ScriptPacker", "Packs the scripts used for MSR into single a Pak file.");
	
	std::string workDir = std::filesystem::current_path().string() + "\\scripts";

	options.add_options()
	("o,output", "The output directory for packed scripts", cxxopts::value<std::string>()->default_value(std::filesystem::current_path().string()))
	("r,release", "Release build, this will clean the scripts then pack them.", cxxopts::value<bool>()->default_value("false"))
	("v,verbose", "Verbose printing.", cxxopts::value<bool>()->default_value("false"))
	("f,fail", "Exit on script error.", cxxopts::value<bool>()->default_value("false"))
	("p,pack", "Turn off script packing.", cxxopts::value<bool>()->default_value("false"))
	("l,log", "Enable logging to a text file.", cxxopts::value<bool>()->default_value("true"))
	("h,help", "Print usage");
	
	//Parse command line arguements
	auto result = options.parse(argc, argv);
	if (result.count("help"))
	{
		std::cout << options.help() << std::endl;
		exit(0);
	}

	Logger::GetInstance().EnableFileLogging(result["log"].as<bool>());
	
	std::string outDir = result["output"].as<std::string>();
	g_Release = result["release"].as<bool>();
	g_Verbose = result["verbose"].as<bool>();
	g_FailOnErr = result["fail"].as<bool>();
	
	if(!std::filesystem::exists(workDir))
	{
		std::cout << "Error: work directory " << workDir << " not found!" << std::endl;
		exit(-1);
	}

	if(!std::filesystem::exists(outDir))
	{
		std::cout << "Error: output directory " << outDir << " not found!" << std::endl;
		exit(-1);
	}
	
	Packer packer(workDir, std::filesystem::current_path().string(), outDir);
	packer.readDirectory(workDir);

	if(result["pack"].as<bool>() == false)
	{
		packer.packScripts();
		Logger::GetInstance().Log("Wrote changes to the script pak. Hash {}\n\n", GetFileCheckSum("./scripts.pak"));
	}

	return 0;
}