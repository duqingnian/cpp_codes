c++代码：
  #include <iostream>
#include <fstream>
#include <stdexcept>
#include <openssl/evp.h>
#include <chrono>
#include <iomanip>
#include <ctime>

std::string calculateMD5(const std::string& filePath) {
    unsigned char buffer[8192];
    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int digestLength = 0;
    EVP_MD_CTX* mdContext = EVP_MD_CTX_new();
    std::ifstream file(filePath, std::ifstream::binary);

    if (!file)
    {
        throw std::runtime_error("Cannot open file: " + filePath);
    }

    if (!mdContext)
    {
        throw std::runtime_error("Failed to create MD context");
    }

    if (EVP_DigestInit_ex(mdContext, EVP_md5(), NULL) != 1)
    {
        EVP_MD_CTX_free(mdContext);
        throw std::runtime_error("Failed to initialize MD context");
    }

    while (file.good())
    {
        file.read(reinterpret_cast<char*>(buffer), sizeof(buffer));
        if (EVP_DigestUpdate(mdContext, buffer, file.gcount()) != 1)
        {
            EVP_MD_CTX_free(mdContext);
            throw std::runtime_error("Failed to update MD context");
        }
    }

    if (EVP_DigestFinal_ex(mdContext, digest, &digestLength) != 1)
    {
        EVP_MD_CTX_free(mdContext);
        throw std::runtime_error("Failed to finalize MD context");
    }

    EVP_MD_CTX_free(mdContext);

    char md5String[33];
    for (unsigned int i = 0; i < digestLength; ++i)
    {
        snprintf(&md5String[i * 2], 3, "%02x", digest[i]);
    }

    return std::string(md5String);
}

int main(int argc, char* argv[]) {
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <file_path>" << std::endl;
        return 1;
    }

    try
    {
        auto start = std::chrono::system_clock::now();
        std::string md5Hash = calculateMD5(argv[1]);
        auto end = std::chrono::system_clock::now();

        std::chrono::duration<double> elapsed = end - start;

        std::time_t startTime = std::chrono::system_clock::to_time_t(start);
        std::time_t endTime = std::chrono::system_clock::to_time_t(end);

        struct tm startTm;
        struct tm endTm;

        localtime_s(&startTm, &startTime);
        localtime_s(&endTm, &endTime);

        std::cout << "Start Time: " << std::put_time(&startTm, "%Y-%m-%d %H:%M:%S") << std::endl;
        std::cout << "Complete Time: " << std::put_time(&endTm, "%Y-%m-%d %H:%M:%S") << std::endl;
        std::cout << "Elapsed Time: " << elapsed.count() << " seconds" << std::endl;
        std::cout << "MD5 Hash: " << md5Hash << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

生成的exe：Md5BigFile.exe
然后在exe同级目录下，创建measure_certutil_time.ps1，内容：

# PowerShell script to measure the elapsed time for CertUtil command

param (
    [string]$filePath
)

# Get the start time
$startTime = Get-Date

# Execute the CertUtil command
& certutil -hashfile $filePath MD5

# Get the end time
$endTime = Get-Date

# Calculate the elapsed time
$elapsedTime = $endTime - $startTime

# Display the start time, end time, and elapsed time
Write-Host "Start Time: $($startTime.ToString('yyyy-MM-dd HH:mm:ss'))"
Write-Host "Complete Time: $($endTime.ToString('yyyy-MM-dd HH:mm:ss'))"
Write-Host "Elapsed Time: $($elapsedTime.TotalSeconds) seconds"


最后用管理员权限打开power shell:
PS D:\project\cpp\TEST\x64\Release> Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope Process
PS D:\project\cpp\TEST\x64\Release> .\measure_certutil_time.ps1 -filePath "D:\ideaIU-2024.3.3.exe"
MD5 hash of D:\ideaIU-2024.3.3.exe:
c643b02d185c9ebf68ff5474093af84b
CertUtil: -hashfile command completed successfully.
Start Time: 2025-03-05 09:59:42
Complete Time: 2025-03-05 09:59:45
Elapsed Time: 2.4775263 seconds
PS D:\project\cpp\TEST\x64\Release> .\Md5BigFile.exe "D:\ideaIU-2024.3.3.exe"
Start Time: 2025-03-05 09:59:49
Complete Time: 2025-03-05 09:59:51
Elapsed Time: 2.09901 seconds
MD5 Hash: c643b02d185c9ebf68ff5474093af84b
PS D:\project\cpp\TEST\x64\Release>
