#include "algorithms.h"
#include "rsa.h"
#include "utils.h"

#include <boost/program_options.hpp>
#include <iostream>
#include <thread>


namespace po = boost::program_options;


const std::string& require_string_option(const po::variables_map& options, const std::string& option)
{
    auto it = options.find(option);
    if(it == options.end())
        throw std::runtime_error{"Option '" + option + "' missed"};

    return it->second.as<std::string>();
}

std::size_t get_rsa_block_size_option(const po::variables_map& options, const std::string& option)
{
    auto it = options.find(option);
    if(it == options.end())
        throw std::runtime_error{"Option '" + option + "' missed"};

    const auto blockSize = it->second.as<std::size_t>();
    if(!is_allowed_rsa_block_size(blockSize))
        throw std::runtime_error{"Error: invalid block size"};

    return blockSize;
}

void execute_program(int argc, char* argv[])
{
    po::variables_map vm;
    po::options_description desc("Options");
    desc.add_options()
            ("help,h", "Show help")
            ("block-size,b", po::value<std::size_t>()->default_value(256), "Block size (in bytes)")
            ("input-file,i", po::value<std::string>(), "Input file")
            ("output-file,o", po::value<std::string>(), "Output file")
            ("key,k", po::value<std::string>(), "Key in HEX format: public (for encrypt) or private (for decrypt)")
            ("modulus,n", po::value<std::string>(), "Modulus (n)")
            ("fast,f", "Fast mode (Multi-threaded with a large memory usage)");

    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if(vm.count("help"))
    {
        std::cout << desc << std::endl;
        return;
    }

    const auto blockSize = get_rsa_block_size_option(vm, "block-size");
    if(vm.size() == 1)
    {
        show_rsa_keys(blockSize);
        return;
    }

    if(vm.count("fast"))
    {
        rsa_mt(require_string_option(vm, "input-file"),
            require_string_option(vm, "output-file"),
            blockSize,
            from_hex(require_string_option(vm, "key")),
            from_hex(require_string_option(vm, "modulus")), std::thread::hardware_concurrency());
    }
    else
    {
        rsa(require_string_option(vm, "input-file"),
            require_string_option(vm, "output-file"),
            blockSize,
            from_hex(require_string_option(vm, "key")),
            from_hex(require_string_option(vm, "modulus")));
    }
}

void test()
{
    const std::size_t blockSize = 64;

    std::random_device rd;
    std::mt19937 gen(rd());

    const std::string filename = "../troll.jpg";
    const std::string encrypted = "../enc_troll.jpg";
    const std::string decrypted = "../dec_troll.jpg";

    const std::size_t countThreads = std::thread::hardware_concurrency();

    const auto[pub, prv, n] = generate_rsa_keys(gen, blockSize);
    rsa_mt(filename, encrypted, blockSize, pub, n, countThreads);
    rsa_mt(encrypted, decrypted, blockSize, prv, n, countThreads);
}


int main(int argc, char* argv[])
{
    try {
        execute_program(argc, argv);
    }
    catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }
    catch (...) {
        std::cerr << "Unknown error" << std::endl;
        return -2;
    }

    return 0;
}
