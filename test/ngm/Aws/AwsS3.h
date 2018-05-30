#ifndef AWSS3_H
#define AWSS3_H

#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>

#include <iostream>
#include <fstream>
#include <string>

#include "Log.h"
#include "HRData.h"

typedef enum {
    asFile = 0,
    asString
} UploadType;

class AwsS3 {
    LOG_MODULE(AwsS3, g_main_log);
public:
    AwsS3();

public:
    void Init();
    void Done();
    bool Send(std::string eventId, ByteBufferPtr data, size_t dataSize, const std::string metadata);
    bool Upload(const std::string data, const char *key, const std::string bucket, size_t dataSize, UploadType upType);

private:
    Aws::SDKOptions m_options;

    bool UploadAsFile   (Aws::S3::S3Client &client, Aws::S3::Model::PutObjectRequest &putObjectRequest, size_t dataSize, const std::string fileName);
    bool UploadAsString (Aws::S3::S3Client &client, Aws::S3::Model::PutObjectRequest &putObjectRequest, const std::string data);
};

#endif // AWSS3_H
