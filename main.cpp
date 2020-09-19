#include "algorithms.h"
#include "rsa.h"
#include "utils.h"

#include <boost/program_options.hpp>

#include <iostream>
#include <functional>
#include <cstring>


void print_help()
{
    std::cout << "Command format to generate keys:\n"
                 "RSA -k <blocksize (must be 32, 64, 128, 256, 512, 1024, 2048 or 4096)>\n"
                 "Example: RSA -k 256\n\n"
                 "Command format to encrypt file:\n"
                 "RSA -e <blocksize> <inputFile> <outputFile> <key> <n>\n"
                 "Example: RSA -e file.txt encrypted_file.txt 3AF434353324124 12312424325435\n\n"
                 "Command format to decrypt file:\n"
                 "RSA -d <blocksize> <inputFile> <outputFile> <key> <n>\n"
                 "Example: RSA -d file.txt decrypted_file.txt FAFAFAF2342342333 12312424325435\n\n"
                  << std::endl;
}

bool is_help(const char* command)
{
    constexpr const char* variants[] = { "-h", "h", "-help", "help", "?", "/?" };
    return std::any_of(std::begin(variants), std::end(variants),
                       [command](const char* v){ return std::strcmp(v, command) == 0; });
}


int main(int argc, char* argv[])
{
    namespace po = boost::program_options;


    po::variables_map vm;
    po::options_description desc("Options");
    desc.add_options()
            ("help,h", "Show help")
            ("block-size,b", po::value<std::size_t>()->default_value(256), "Block size (in bytes)")
            ("mode,m", po::value<std::string>(), R"(Select mode: "keys", "encrypt" or "decrypt")")
            ("input-file,i", po::value<std::string>(), "Input file")
            ("output-file,o", po::value<std::string>(), "Output file")
            ("public-key,pub", po::value<std::string>(), "Public key (in HEX format)")
            ("private-key,prv", po::value<std::string>(), "Private key (in HEX format)")
            ("modulus,n", po::value<std::string>(), "Modulus (N)")
            ;

    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if(vm.count("help"))
    {
        std::cout << desc << std::endl;
        return 0;
    }

    if(vm.count("mode") == 0)
    {
        std::cout << "Option 'mode' missed" << std::endl;
        return -1;
    }

    const std::size_t blockSize = vm["block-size"].as<std::size_t>();
    if(!is_allowed_rsa_block_size(blockSize))
    {
        std::cout << "Error: invalid block size" << std::endl;
        return -2;
    }

    const std::string mode = vm["mode"].as<std::string>();
    if(mode == "keys")
    {
        show_rsa_keys(blockSize);
        return 0;
    }

    static const std::array<std::string, 3> requiredOptions = {
            "input-file", "output-file", "modulus"
    };

    for(const std::string& option : requiredOptions)
    {
        if(vm.count(option) == 0)
        {
            std::cout << "Option '" << option << "' missed" << std::endl;
            return -4;
        }
    }

    const std::string inputFile = vm["input-file"].as<std::string>();
    const std::string outputFile = vm["output-file"].as<std::string>();
    const big_int n = from_hex(vm["modulus"].as<std::string>());

    if(mode == "encrypt")
    {
        if(vm.count("public-key") == 0)
        {
            std::cout << "Option 'public-key' missed" << std::endl;
            return -5;
        }

        rsa(inputFile, outputFile, blockSize, from_hex(vm["public-key"].as<std::string>()), n);
    }
    else if(mode == "decrypt")
    {
        if(vm.count("private-key") == 0)
        {
            std::cout << "Option 'private-key' missed" << std::endl;
            return -6;
        }

        rsa(inputFile, outputFile, blockSize, from_hex(vm["private-key"].as<std::string>()), n);
    }
    else
    {
        std::cout << "Unknown 'mode' option value" << std::endl;
        return -7;
    }

    return 0;
}
