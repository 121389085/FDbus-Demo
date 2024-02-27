#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <fdbus/fdbus.h>
//#pragma comment(lib,"ws2_32.lib")
#define XCLT_TEST_SINGLE_DIRECTION 0
#define XCLT_TEST_BI_DIRECTION     1
using namespace ipc::fdbus;

class FDBusClient : public CBaseClient
{
public:
    FDBusClient(const char* name, CBaseWorker* worker = 0)
        : CBaseClient(name, worker)//worker：如果指定则所有回调函数在该worker上运行
                                   //否则在fdbus工作线程FDB_CONTEXT上运行
    {
    }
protected:
    void onOnline(const CFdbOnlineInfo& info) override
    {
        printf("........ [%s] onOnline mSid=%d.............\n", name().c_str(), info.mSid);

        std::vector<int32_t> subIDs;
        subIDs.push_back(1);

        CFdbMsgSubscribeList subscribe_list;
        for (auto id : subIDs)
        {
            addNotifyItem(subscribe_list, id);
        }
        subscribe(subscribe_list);
        char buffer[64] = {};
        std::cout << "请输入要发送的信息：";
        std::cin >> buffer;
        this->invoke(XCLT_TEST_BI_DIRECTION, buffer, sizeof(buffer));
    }

    void onOffline(const CFdbOnlineInfo& info) override
    {
        printf("\nOffline...\n");
    }

    void onReply(CBaseJob::Ptr& msg_ref) override
    {
        auto msg = castToMessage<CBaseMessage*>(msg_ref);
        printf("%s\n", msg->getPayloadBuffer());
    }

    void onStatus(CBaseJob::Ptr& msg_ref, int32_t error_code, const char* description) override
    {
        std::string sError = "[error] ";
        sError += "code:" + std::to_string(error_code) + ";";
        sError += "detail:";
        sError += description;
    }

    void onBroadcast(CBaseJob::Ptr& msg_ref) override
    {
        printf("[%s]\n", __FUNCTION__);
        auto msg = castToMessage<CBaseMessage*>(msg_ref);
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
    FDB_CONTEXT->start(); //启动fdbus收发线程；其本质就是一个worker线程

    auto spWorker = std::make_shared<CBaseWorker>();
    spWorker->start(); //启动fdbus收发线程；其本质就是一个worker线程
    std::string url(FDB_URL_SVC);
    url += FDB_XTEST_NAME;
    auto client = new FDBusClient("TestClient", spWorker.get());    //指定链接TestClient(名字寻址)，.get()指定原始地址
    client->connect(url.c_str());

    /* convert main thread into worker */
    CBaseWorker background_worker;
    background_worker.start(FDB_WORKER_EXE_IN_PLACE);
}