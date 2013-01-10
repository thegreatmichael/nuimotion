#include <node.h>
#include <v8.h>
#include <stdio.h>
#include <stdlib.h>

#include <OpenNI.h>

#include "../Common/OniSampleUtilities.h"

using namespace v8;
using namespace openni;

Device device;
VideoStream depth;

Handle<Value> initDevice(const Arguments& args) {
    HandleScope scope;
	Status rc = OpenNI::initialize();
	if (rc != STATUS_OK)
	{
		printf("Initialize failed\n%s\n", OpenNI::getExtendedError());
	}

	rc = device.open(ANY_DEVICE);
	if (rc != STATUS_OK)
	{
		printf("Couldn't open device\n%s\n", OpenNI::getExtendedError());
	}

    if (device.getSensorInfo(SENSOR_DEPTH) != NULL)
    {
        rc = depth.create(device, SENSOR_DEPTH);
        if (rc != STATUS_OK)
        {
            printf("Couldn't create depth stream\n%s\n", OpenNI::getExtendedError());
        }
    }

    rc = depth.start();
    if (rc != STATUS_OK)
    {
        printf("Couldn't start the depth stream\n%s\n", OpenNI::getExtendedError());
    }
	return scope.Close( Boolean::New(true) );
}

Handle<Value> closeDevice(const Arguments& args) {
    HandleScope scope;
	depth.stop();
	depth.destroy();
	device.close();
	OpenNI::shutdown();
	return scope.Close( Boolean::New(true) );
}

Handle<Value> getDepth(const Arguments& args) {
    HandleScope scope;

    VideoFrameRef frame;

    Status rc;
    rc = depth.readFrame(&frame);
    if (rc != STATUS_OK)
    {
        printf("Wait failed\n");
    }

    if (frame.getVideoMode().getPixelFormat() != PIXEL_FORMAT_DEPTH_1_MM && frame.getVideoMode().getPixelFormat() != PIXEL_FORMAT_DEPTH_100_UM)
    {
        printf("Unexpected frame format\n");
    }

    DepthPixel* pDepth = (DepthPixel*)frame.getData();

    int middleIndex = (frame.getHeight()+1)*frame.getWidth()/2;
    return scope.Close(Number::New(pDepth[middleIndex]));
}

Handle<Value> getRandomCoords2D(const Arguments& args) {
    HandleScope scope;

    Local<Object> obj = Object::New();
    obj->Set(String::NewSymbol("x"), Number::New( 1 + (rand() % 100 )));
    obj->Set(String::NewSymbol("y"), Number::New( 1 + (rand() % 100 )));

    return scope.Close(obj);
}

Handle<Value> getRandomCoords3D(const Arguments& args) {
    HandleScope scope;

    if (args.Length() < 3) {
        ThrowException(Exception::TypeError(String::New("Wrong number of arguments")));
        return scope.Close(Undefined());
    }

    if (!args[0]->IsNumber() || !args[1]->IsNumber() || !args[2]->IsNumber()) {
        ThrowException(Exception::TypeError(String::New("Wrong arguments")));
        return scope.Close(Undefined());
    }

    Local<Number> xBound = args[0]->ToNumber();
    Local<Number> yBound = args[1]->ToNumber();
    Local<Number> zBound = args[2]->ToNumber();

    Local<Object> obj = Object::New();
    obj->Set(String::NewSymbol("x"), Number::New( 1 + (rand() % xBound->IntegerValue() )));
    obj->Set(String::NewSymbol("y"), Number::New( 1 + (rand() % yBound->IntegerValue() )));
    obj->Set(String::NewSymbol("z"), Number::New( 1 + (rand() % zBound->IntegerValue() )));
    return scope.Close(obj);
}

void init(Handle<Object> target) {
    target->Set(String::NewSymbol("getRandomCoords3D"),
        FunctionTemplate::New(getRandomCoords3D)->GetFunction());

    target->Set(String::NewSymbol("getRandomCoords2D"),
        FunctionTemplate::New(getRandomCoords2D)->GetFunction());

    target->Set(String::NewSymbol("initDevice"),
        FunctionTemplate::New(initDevice)->GetFunction());

    target->Set(String::NewSymbol("getDepth"),
        FunctionTemplate::New(getDepth)->GetFunction());
}
NODE_MODULE(sweatin, init)
