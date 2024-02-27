#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <fdbus/fdbus.h>
#pragma comment(lib,"ws2_32.lib")
#define XCLT_TEST_SINGLE_DIRECTION 0
#define XCLT_TEST_BI_DIRECTION     1

using namespace ipc::fdbus;

class CXServer : public CBaseServer
{
public:
    CXServer(const char* name, CBaseWorker* worker = 0)
        : CBaseServer(name, worker)//worker：如果指定则所有回调函数在该worker上运行；
                                   //否则在fdbus工作线程FDB_CONTEXT上运行
    {

    }

protected:
    void onOnline(const CFdbOnlineInfo& info) override
    {
        printf("%s\n", __FUNCTION__);
    }
    void onOffline(const CFdbOnlineInfo& info) override
    {
        printf("%s\n", __FUNCTION__);
    }
    void onInvoke(CBaseJob::Ptr& msg_ref) override
    {
        //printf("%s\n", __FUNCTION__);
        auto msg = castToMessage<CBaseMessage*>(msg_ref);
        auto sid=msg->getSession()->sid();
        char* message = (char*)(msg->getPayloadBuffer());
        switch (msg->code())
        {
        case XCLT_TEST_BI_DIRECTION:
        {
            incrementReceived(msg->getPayloadSize()); 
            std::cout << message;
            char buffer[] = "已收到";
            msg->reply(msg_ref, buffer, sizeof(buffer));
        }
        break;
        case XCLT_TEST_SINGLE_DIRECTION:
            incrementReceived(msg->getPayloadSize());
            std::cout << message;
            char buffer[] = "已收到";
            msg->reply(msg_ref, buffer, sizeof(buffer));
            break;
        }
    }
private:
    uint64_t mTotalBytesReceived;
    uint64_t mTotalRequest;
    uint64_t mIntervalBytesReceived;
    uint64_t mIntervalRequest;
    void incrementReceived(uint32_t size)
    {
        mTotalBytesReceived += size;
        mIntervalBytesReceived += size;
        mTotalRequest++;
        mIntervalRequest++;
    }
};


int main(int argc, char** argv)
{
#ifdef __WIN32__
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;

    /* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
    wVersionRequested = MAKEWORD(2, 2);

    err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
        /* Tell the user that we could not find a usable */
        /* Winsock DLL.                                  */
        printf("WSAStartup failed with error: %d\n", err);
        return 1;
    }
#endif
    FDB_CONTEXT->enableLogger(false);
    /* start fdbus context thread */
    FDB_CONTEXT->start();//启动fdbus收发线程；其本质就是一个worker线程

    auto server = new CXServer(FDB_XTEST_NAME);
    server->TCPEnabled();
    server->bind();
    /* convert main thread into worker */
    CBaseWorker background_worker;
    background_worker.start(FDB_WORKER_EXE_IN_PLACE);//FDB_WORKER_EXE_IN_PLACE：置位表示不启动线程，在调用的地方运行worker的主循环
                                                    //否则启动新线程执行worker的主循环

}