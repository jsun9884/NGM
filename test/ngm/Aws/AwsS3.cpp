#include "AwsS3.h"
#include "Event.h"

#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>
#include <aws/core/utils/HashingUtils.h>
#include <aws/s3/model/PutObjectRequest.h>
#include <aws/core/auth/AWSCredentialsProvider.h>

#include "CloudManager.h"
#include <fstream>
#include <stdlib.h>
#include <stdio.h>

using namespace Aws::S3;
using namespace Aws::S3::Model;

static const char* KEY = "hr";
static const char* BUCKET = "ngm-hr-raw-data";
const Aws::String ACCESSKEY = "AKIAILEW7MYDWPZ4HR3A";
const Aws::String SECRETKEY = "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";

typedef Aws::Client::ClientConfiguration    ClientConfig;
typedef Aws::Auth::AWSCredentials           Credentials;

AwsS3::AwsS3()
{
}

void AwsS3::Init() {
    m_options.loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Off;
    Aws::InitAPI(m_options);
}

bool AwsS3::Send(std::string eventId, ByteBufferPtr data, size_t dataSize, const std::string metadata)
{
    std::ofstream dataFile;
    std::string sFileName = "./high_resolution/" + eventId;
    const char* dataFileName = eventId.c_str();
    const char* metaDataFileName;
    std::string tmp_metaDataFileName = eventId;

    tmp_metaDataFileName += ".metadata";
    metaDataFileName = tmp_metaDataFileName.c_str();

    log::debug(dataFileName);
    log::debug(metaDataFileName);

    dataFile.open(sFileName);
    dataFile.write((const char *)data.get(), dataSize);
    dataFile.flush();
    dataFile.close();

    if (Upload(metadata, metaDataFileName, BUCKET, dataSize, asString)) {
        if(Upload(sFileName, dataFileName, BUCKET, dataSize, asFile)){
            std::remove(sFileName.c_str());
        }
    }
    else {
        return false;
    }
}

bool AwsS3::Upload(const std::string data, const char* key, const std::string bucket, size_t dataSize, UploadType upType)
{
    Credentials s3AccessCreds(ACCESSKEY, SECRETKEY);
    ClientConfig s3ClientConfig;
    s3ClientConfig.region = Aws::Region::US_WEST_2;
    s3ClientConfig.scheme = Aws::Http::Scheme::HTTPS;
    s3ClientConfig.requestTimeoutMs = 30000;
    S3Client client(s3AccessCreds, s3ClientConfig);

    PutObjectRequest putObjectRequest;
    putObjectRequest.WithKey(key).WithBucket(bucket.c_str());

    if (upType == asString) {
        return UploadAsString(client, putObjectRequest, data);
    }
    else if (upType == asFile) {
        return UploadAsFile(client, putObjectRequest, dataSize, data);
    }
}

bool AwsS3::UploadAsFile(S3Client &client, PutObjectRequest &putObjectRequest, size_t dataSize, const std::string fileName) {
    log::info(">>>>>> uploading as file... <%s>", fileName.c_str());

    //std::cout << "open file: " << fileName << "\n";
    auto input_data = Aws::MakeShared<Aws::FStream>("data", fileName, std::ios_base::in);
    //std::cout << "2\n";

    putObjectRequest.SetBody(input_data);
    //std::cout << "3\n";
    auto putObjectOutcome = client.PutObject(putObjectRequest);

    //std::cout << "4\n";
    bool ok = putObjectOutcome.IsSuccess();
    if (ok) {
        log::info(">>>>>> upload as file succedded!");
    }
    else {
        log::error() << ">>>>>> upload as file failed! *** ";// << putObjectOutcome.GetError().GetExceptionName() << " " <<
                     //putObjectOutcome.GetError().GetMessage();
    }

    return ok; // putObjectOutcome.IsSuccess();
}

bool AwsS3::UploadAsString(S3Client &client, PutObjectRequest &putObjectRequest, const std::string data) {
    log::info(">>>>>> uploading as string...");

    auto requestStream = Aws::MakeShared<Aws::StringStream>("data");

    log::info() << "data: " << data;

    *requestStream << Aws::String(data.c_str());
    putObjectRequest.SetBody(requestStream);
    auto putObjectOutcome = client.PutObject(putObjectRequest);

    bool ok = putObjectOutcome.IsSuccess();
    if (ok) {
        log::info(">>>>>> upload as string succedded!");
    }
    else {
        log::error() << ">>>>>> upload as string failed! *** ";// << putObjectOutcome.GetError().GetExceptionName() << " " <<
                     //putObjectOutcome.GetError().GetMessage();
    }

    return ok; //putObjectOutcome.IsSuccess();
}

void AwsS3::Done() {
    Aws::ShutdownAPI(m_options);
}
