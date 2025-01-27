/*
 * Copyright (C) 2023 Xiaomi Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <binder/ICpcServiceManager.h>
#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>

#include "BnTestPromise.h"
#include "ITestPromise.h"
#include "ITestPromiseCallback.h"

#include <utils/Log.h>

using namespace android;

class TestServer : public BnTestPromise {
public:
    binder::Status add(int32_t a, int32_t b, const sp<ITestPromiseCallback>& callback)
    {
        usleep(500000);
        callback->result(a + b);
        return binder::Status::ok();
    }
    binder::Status requestExit()
    {
        ALOGI("requestExit: from thread %d", gettid());
#ifdef __NuttX__
        ProcessState::self()->requestExit();
#else
        exit(0);
#endif
        return binder::Status::ok();
    }
};

extern "C" int main(int argc, char** argv)
{
    ALOGD("promise service start argc: %d, argv[0]: %s", argc, argv[0]);

    sp<TestServer> testserver = sp<TestServer>::make();

    sp<IServiceManager> sm(defaultCpcServiceManager());

    sp<IBinder> binder = sm->getService(String16("promise.srv"));
    if (binder != NULL) {
        ALOGE("Service promise.srv has already been added!");
        return 0;
    }

    sm->addService(String16("promise.srv"), testserver);

    IPCThreadState::self()->joinThreadPool();

    return 0;
}
