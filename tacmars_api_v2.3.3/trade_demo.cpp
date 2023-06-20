#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/eventfd.h>
#include <unistd.h>

#include "TacFtdcTraderApi.h"
#include "trade_demo.h"

#define TacmarsFrontAddress  "tcp://126.89.64.131:9998"
#define ClientAppID          "abc_123_!@#"
#define ClientAuthCode       "123456"
#define ClientUserID         "1001"
#define ClientPassword       "123456"
#define ClientLogFilePath    "MyClientLog_" ClientUserID

int requestSeq = 0;
int event = eventfd(0, EFD_SEMAPHORE);

SimpleHandler::SimpleHandler(CTacFtdcTraderApi* pApi, const char* app, const char* auth, const char* user, const char* pass)
    : m_pApi(pApi)
{
    strncpy(m_app, app, sizeof(m_app));
    strncpy(m_auth, auth, sizeof(m_auth));
    strncpy(m_user, user, sizeof(m_user));
    strncpy(m_pass, pass, sizeof(m_pass));
}

void SimpleHandler::OnFrontConnected()
{
    printf("Front %s connected\n", TacmarsFrontAddress);

    CTacFtdcAuthenticateField req = {};

    // Set APP ID
    strncpy(req.AppID, m_app, sizeof(req.AppID));

    // Set Authenticate Code
    strncpy(req.AuthCode, m_auth, sizeof(req.AuthCode));

    // Set User ID
    strncpy(req.UserID, m_user, sizeof(req.UserID));

    // Submit Login request
    m_pApi->ReqAuthenticate(&req, requestSeq++);
}

void SimpleHandler::OnFrontDisconnected(int nReason)
{
    printf("Front %s disconnected, Reason %d\n", TacmarsFrontAddress, nReason);
}

///错误应答
void SimpleHandler::OnRspError(CTacFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    printf("OnRspError, ErrorID=[%d], ErrorMsg=[%s]\n", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
}

void SimpleHandler::OnRspAuthenticate(CTacFtdcRspAuthenticateField *pRspAuthenticate, CTacFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (pRspInfo->ErrorID != 0)
    {
        printf("Authentication Failed\n");
        exit(-1);
    }

    CTacFtdcUserLoginField req = {};

    // Set Client ID
    strncpy(req.UserID, m_user, sizeof(req.UserID));

    // Set Client password
    strncpy(req.Password, m_pass, sizeof(req.Password));

    // Submit Login request
    m_pApi->ReqUserLogin(&req, requestSeq++);
}

///用户登录应答
void SimpleHandler::OnRspUserLogin(CTacFtdcRspUserLoginField *pRspUserLogin, CTacFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    printf("OnRspUserLogin:\n");
    printf("ErrorID=[%d], ErrorMessage=[%s], User=[%s]\n",
           pRspInfo->ErrorID, pRspInfo->ErrorMsg, pRspUserLogin->UserID);

    if (pRspInfo->ErrorID != 0)
    {
        printf("Login Failed\n");
        exit(-1);
    }

    CTacFtdcInputOrderField req = {};

    // Set instrument ID
    strcpy(req.InstrumentID, "ag1806");

    // Set price type
    req.OrderPriceType = TAC_FTDC_OPT_LimitPrice;

    // Set order direction
    req.Direction = TAC_FTDC_D_Buy;

    // Set order offset
    req.OffsetFlag = TAC_FTDC_OF_Open;

    // Set price
    req.LimitPrice = 10000.0f;

    // Set order volume
    req.VolumeTotalOriginal = 1;

    // Set time condition
    req.TimeCondition = TAC_FTDC_TC_GFD;

    // Set the client local order reference ID
    req.OrderRef = 1;

    // Set client ID which insert the order.
    // It should be the same with the login client ID
    strcpy(req.ClientID, m_user);

    // Set hedge flag
    req.HedgeFlag = TAC_FTDC_HF_Speculation;

    // Set exchange ID
    req.ExchangeID = TAC_FTDC_EXID_SHFE;

    // Submit the order to Tacmars
    int ret = m_pApi->ReqOrderInsert(&req, requestSeq++);

    if (ret < 0)
    {
        printf("ReqOrderInsert Error, return value[%d]\n", ret);
    }
}

///用户登出应答
void SimpleHandler::OnRspUserLogout(CTacFtdcUserLogoutField *pUserLogout, CTacFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    printf("OnRspUserLogout:\n");
    printf("ErrorID=[%d], ErrorMessage=[%s]\n", pRspInfo->ErrorID, pRspInfo->ErrorMsg);

    eventfd_write(event, 1);
}

///报单录入应答
void SimpleHandler::OnRspOrderInsert(CTacFtdcRspOrderInsertField *pRspOrderInsert, CTacFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    printf("OnRspOrderInsert:\n");
    printf("ErrorID=[%d], ErrorMessage=[%s]\n", pRspInfo->ErrorID, pRspInfo->ErrorMsg);

    if (pRspInfo->ErrorID != 0)
    {
        printf("OrderInsert Failed\n");
        exit(-1);
    }
}

///报单回报
void SimpleHandler::OnRtnOrder(CTacFtdcRtnOrderField *pOrder)
{
    printf("OnRtnOrder:\n");
    printf("OrderRefID=[%ld], OrderLocalID=[%ld], OrderSysID=[%ld], OrderStatus=[%c]\n",
           pOrder->OrderRef, pOrder->OrderLocalID, pOrder->OrderSysID, pOrder->OrderStatus);
}

///成交回报
void SimpleHandler::OnRtnTrade(CTacFtdcRtnTradeField *pTrade)
{
    printf("OnRtnTrade:\n");
    printf("OrderRefID=[%ld], OrderLocalID=[%ld], OrderSysID=[%ld], TradeID=[%ld], Price=[%f], Volume=[%d]\n",
           pTrade->OrderRef, pTrade->OrderLocalID, pTrade->OrderSysID, pTrade->TradeID, pTrade->Price, pTrade->Volume);
}

int main()
{
    CTacFtdcTraderApi* pUserApi = CTacFtdcTraderApi::CreateFtdcTraderApi("flow");

    SimpleHandler spiHandler(pUserApi, ClientAppID, ClientAuthCode, ClientUserID, ClientPassword);

    if (pUserApi == nullptr)
    {
        printf("Create CTacFtdcTraderApi failed\n");
        exit(-1);
    }

    printf("------%s------\n", pUserApi->GetApiVersion());

    pUserApi->RegisterSpi(&spiHandler);

    pUserApi->RegisterFront(TacmarsFrontAddress);

    pUserApi->SubscribePrivateTopic(TAC_TERT_RESUME);

    pUserApi->SetLogFilePath(ClientLogFilePath);

    pUserApi->SetLogLevel(true);

    pUserApi->SetTraderApiCpuAffinity(1, 2);

    // Start worker thread and init api after setup configuration
    pUserApi->Init();

    // Logger thread starts in 2 seconds, so sleep sometime to wait log output
    sleep(3);

    CTacFtdcUserLogoutField req = {};

    // Set Client ID
    strcpy(req.UserID, ClientUserID);

    // Submit Logout request
    pUserApi->ReqUserLogout(&req, requestSeq++);

    eventfd_read(event, nullptr);

    pUserApi->Release();
}
