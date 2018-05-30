/*
 * Copyright (C) ..
 *
 * Author: .
 *
 *
 * Your desired license
 *  
 */

#include <iostream>
#include <aws/core/utils/memory/stl/AWSString.h>
#include <aws/core/utils/logging/AWSLogging.h>
#include <aws/core/utils/logging/DefaultLogSystem.h>

#include "SQLite3DB.h"
#include "Log.h"
#include "CrashHandler.h"
#include "CloudManager.h"
#include "CLIManager.h"
#include "HardwareManager.h"
#include "AwsS3.h"
#include "SerialComm.h"
#include "SysSensor.h"
#include "I2c.h"
#include "Leds.h"
#include "AnyValue.h"
#include "Version.h"

#include <unistd.h>
#include <syscall.h>

// >>>>>>>>>>>>>>>>>>>>>>>>>
#if 0

#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>
#include <aws/core/utils/HashingUtils.h>
#include <aws/s3/model/PutObjectRequest.h>
#include <aws/core/auth/AWSCredentialsProvider.h>

using namespace Aws::S3;
using namespace Aws::S3::Model;

static const char* KEY = "hr";
static const char* BUCKET = "ngm-firmware";//"ngm-hr-raw-data";
const Aws::String ACCESSKEY = "AKIAILEW7MYDWPZ4HR3A";
const Aws::String SECRETKEY = "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";

typedef Aws::Client::ClientConfiguration    ClientConfig;
typedef Aws::Auth::AWSCredentials           Credentials;

#endif
// <<<<<<<<<<<<<<<<<<<<<<<
LoggerPtr g_main_log;
LoggerPtr g_cout_log;
LoggerPtr g_gps_log;
LoggerPtr g_rtl_log;
LoggerPtr g_msp430_log;
LoggerPtr g_object_counter_log;
LoggerPtr g_sqlite3_log;


int main(int argc, char **argv) {
    CrashHandler crashHandler;

#if 1
    //system("/usr/sbin/ntpdate pool.ntp.org");
    system("mkdir -p logs");

    //Aws::Utils::Logging::InitializeAWSLogging(Aws::MakeShared<Aws::Utils::Logging::DefaultLogSystem>
    //                                          ("Example", Aws::Utils::Logging::LogLevel::Debug, "aws_sdk_"));
    //SysSensor sysSensor("/sys/devices/virtual/hwmon/hwmon0/temp1_input");


    g_main_log = std::make_shared<Logger>();
    g_cout_log = std::make_shared<Logger>();
    g_gps_log = std::make_shared<Logger>();
    g_rtl_log = std::make_shared<Logger>();
    g_msp430_log = std::make_shared<Logger>();
    g_object_counter_log = std::make_shared<Logger>();
    g_sqlite3_log = std::make_shared<Logger>();
                                                                                // 7Mb
                                                                                //  v
    LogWriterPtr writer = std::make_shared<FileLogWriter>("logs/system.log", 4, 0x700000, true);
    g_main_log->addWriter(writer);
    g_cout_log->addWriter(writer);

    writer = std::make_shared<OStreamLogWriter>(&std::cout);
    g_cout_log->addWriter(writer);

    writer = std::make_shared<FileLogWriter>("logs/gps.log", 4, 0x700000, true);
    g_gps_log->addWriter(writer);

    writer = std::make_shared<FileLogWriter>("logs/rtl.log", 4, 0x700000, true);
    g_rtl_log->addWriter(writer);

    writer = std::make_shared<FileLogWriter>("logs/msp430.log", 4, 0x700000, true);
    g_msp430_log->addWriter(writer);

    writer = std::make_shared<FileLogWriter>("logs/object_counter.log", 2, 0x100000, true);
    g_object_counter_log->addWriter(writer);

    writer = std::make_shared<FileLogWriter>("logs/sqlite3.log", 4, 0x700000, true);
    g_sqlite3_log->addWriter(writer);

    g_main_log->start();
    g_cout_log->start();
    g_gps_log->start();
    g_rtl_log->start();
    g_msp430_log->start();
    g_object_counter_log->start();
    g_sqlite3_log->start();

    crashHandler.initialize();

#if 0 //Testing SQLite3DB

    glog::info() << "Testing sqlite3...";
    SQLite3DB sqldb("c12ngm-data-state-common.sqlite3", "c12ngm-data-state-common-create.sql");
    sqldb.exec("");
    glog::info() << "End of sqlite3 tests...";

#endif

    glog::info() << "*** Starting NGM v. " << FW_VERSION << " ***";
    CloudManager cloud;
    crashHandler.setCloudObj(&cloud);
    bool cfgDone = false;

    if (argc > 1) {
        glog::info() << "*** Firmware Upgrade Done ***";
        cloud.setUpgradeCommandId(argv[1]);
    }

    try {
        cloud.loadConfiguration("configuration/Configuration.xml.new");
        system("cp -f Configuration.xml Configuration.xml.bak");
        system("mv -f configuration/Configuration.xml.new Configuration.xml");
        cfgDone = true;
        cloud.setCfgUpdateState(CfgUpdateState::Success);
        glog::info() << "*** Configuration.xml.new was found!";
    }
    catch(CantLoadParametersXmlEx& e) {
        // There isn't cfg file for update
    }
    catch(...) {
        cloud.setCfgUpdateState(CfgUpdateState::Failed);
    }

    if (!cfgDone) {
        try {
            cloud.loadConfiguration("Configuration.xml");
        }
        catch(std::exception &e) {
            glog::error() << "Exception: " << e.what();
            glog::error() << "Configuration.xml corrupted... trying to open Configuration.xml.bak";
            cloud.loadConfiguration("Configuration.xml.bak");
            system("cp -f Configuration.xml.bak Configuration.xml");
        }
    }
      //cloud.saveConfiguration("Configuration.xml");
    std::thread cloudThread = cloud.runThread();

    CLIManager *cli = CLIManager::Instance(HardwareManager::instance());
    cli->Init(cloud.getLeds(), &cloud);
    int returnValue = cli->Run();       // 0 - reboot
                                        // 1 - lunch commBridge

    cloud.stop();
    cloudThread.join();

    return returnValue;

#else
    g_main_log = std::make_shared<Logger>();
    g_cout_log = std::make_shared<Logger>();

    LogWriterPtr writer = std::make_shared<FileLogWriter>("logs/comm_forward.log.0", true);
    g_main_log->addWriter(writer);
    g_cout_log->addWriter(writer);

    writer = std::make_shared<OStreamLogWriter>(&std::cout);
    g_cout_log->addWriter(writer);

    g_main_log->start();
    g_cout_log->start();

    std::cout << "Starting UART Bridge between /dev/ttyUSB0 and /dev/ttymxc3 (TI)...\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    SerialComm sc("/dev/ttyUSB0", "/dev/ttymxc3");
    sc.init();
    sc.join();
    return 0;

#endif
}

