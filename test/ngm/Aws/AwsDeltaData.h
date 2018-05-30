#pragma once

#include "AwsHandler.h"

extern "C" {
#include "aws_iot_shadow_interface.h"
}

typedef AwsHandler<std::string, std::string> AwsDeltaHandler;

struct AwsDeltaData
{
	AwsDeltaHandler handler;
	jsonStruct_t deltaObj;
	AwsDeltaData(AwsDeltaHandler::Handler cb):handler(cb){}
};
